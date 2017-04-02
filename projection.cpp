/******************************************************/
/*                                                    */
/* projection.cpp - map projections                   */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmath>
#include <iostream>
#include "projection.h"
#include "brent.h"
#include "ldecimal.h"

using namespace std;

Projection::Projection()
{
  ellip=&Sphere;
  offset=xy(0,0);
  scale=1;
}

void LambertConicSphere::setParallel(double Parallel)
{
  centralParallel=Parallel;
  exponent=sin(Parallel);
  if (exponent==0)
    coneScale=1;
  else if (fabs(exponent)==1)
    coneScale=2;
  else
    coneScale=cos(Parallel)/pow(tan((M_PIl/2-Parallel)/2),exponent);
  //cout<<"Parallel "<<radtodeg(Parallel)<<" coneScale "<<coneScale<<endl;
}

LambertConicSphere::LambertConicSphere():Projection()
{
  centralMeridian=0;
  setParallel(0);
  poleY=INFINITY;
}

LambertConicSphere::LambertConicSphere(double Meridian,double Parallel):Projection()
{
  latlong maporigin;
  centralMeridian=Meridian;
  setParallel(Parallel);
  poleY=0;
  maporigin=latlong(Meridian,Parallel);
  poleY=-latlongToGrid(maporigin).gety();
}

LambertConicSphere::LambertConicSphere(double Meridian,double Parallel0,double Parallel1):Projection()
{
  latlong maporigin;
  brent br;
  latlong ll;
  double ratiolog0,ratiolog1,Parallel;
  centralMeridian=Meridian;
  setParallel(Parallel0);
  ratiolog0=scaleRatioLog(Parallel0,Parallel1);
  setParallel(Parallel1);
  ratiolog1=scaleRatioLog(Parallel0,Parallel1);
  /* Setting one parallel to ±90° and the other to something else, or either
   * or both to ±(>90°), is an error. I'm not sure whether to throw an exception
   * or set the state to invalid. I'm now setting it to invalid.
   * If the parallels are 45° and 90°, different computers give wildly different
   * results: 84.714395 on Linux/Intel, but 67.5° on Linux/ARM and DFBSD.
   * This is because M_PIl!=M_PI, resulting in M_PIl/2-ll.lat being tiny but
   * positive when ll.lat is M_PIl rounded to double.
   * If the parallels are 45° and DEG90-1 (89.999999832°), the three computers
   * agree that Parallel is 82.686083 (but no more precisely), so that is not
   * an error.
   */
  if ((Parallel0!=Parallel1 && (radtobin(fabs(Parallel0))==DEG90 || radtobin(fabs(Parallel1))==DEG90))
      || fabs(Parallel0)>M_PIl/2
      || fabs(Parallel1)>M_PIl/2)
  {
    centralParallel=poleY=exponent=coneScale=NAN;
    cerr<<"Invalid parallels in LambertConicSphere"<<endl;
  }
  else
  {
    Parallel=br.init(Parallel0,ratiolog0,Parallel1,ratiolog1,false);
    while (!br.finished())
    {
      cout<<"Parallel "<<ldecimal(radtodeg(Parallel))<<endl;
      setParallel(Parallel);
      Parallel=br.step(scaleRatioLog(Parallel0,Parallel1));
    }
    setParallel(Parallel);
    ll.lon=centralMeridian;
    ll.lat=Parallel0;
    scale=1/scaleFactor(ll);
    poleY=0;
    maporigin=latlong(Meridian,Parallel);
    poleY=-latlongToGrid(maporigin).gety();
  }
}

double LambertConicSphere::scaleRatioLog(double Parallel0,double Parallel1)
{
  latlong ll;
  double ret;
  ll.lon=centralMeridian;
  ll.lat=Parallel0;
  ret=log(scaleFactor(ll));
  ll.lat=Parallel1;
  ret-=log(scaleFactor(ll));
  return ret;
}

latlong LambertConicSphere::gridToLatlong(xy grid)
{
  double angle,radius;
  latlong ret;
  grid=(grid-offset)/scale;
  if (exponent==0)
  {
    angle=grid.east()/ellip->geteqr();
    radius=exp(-grid.north()/ellip->getpor());
  }
  ret.lat=M_PIl/2-2*atan(radius);
  ret.lon=angle+centralMeridian;
  return ret;
}

xyz LambertConicSphere::gridToGeocentric(xy grid)
{
  return ellip->geoc(gridToLatlong(grid),0);
}

xy LambertConicSphere::geocentricToGrid(xyz geoc)
{
  latlongelev lle=ellip->geod(geoc);
  latlong ll(lle);
  return latlongToGrid(ll);
}

xy LambertConicSphere::latlongToGrid(latlong ll)
{
  double radius,angle,northing,easting;
  radius=tan((M_PIl/2-ll.lat)/2);
  angle=ll.lon-centralMeridian;
  while(angle>M_PIl*2)
    angle-=M_PIl*2;
  while(angle<-M_PIl*2)
    angle+=M_PIl*2;
  /* TODO: if exponent is small, say less than 1/64, then use a complex
   * function similar to expm1 on the Mercator coordinates.
   */
  if (exponent==0)
  {
    easting=angle*ellip->geteqr();
    northing=-log(radius)*ellip->getpor();
  }
  else
  {
    radius=pow(radius,exponent)*ellip->getpor()/exponent*coneScale;
    angle*=exponent;
    easting=radius*sin(angle);
    northing=poleY-radius*cos(angle);
  }
  return xy(easting,northing)*scale+offset;
}

double LambertConicSphere::scaleFactor(xy grid)
{
  return scaleFactor(gridToLatlong(grid));
}

double LambertConicSphere::scaleFactor(latlong ll)
{
  double coneradius,cenconeradius,parradius,cenparradius;
  coneradius=tan((M_PIl/2-ll.lat)/2);
  cenconeradius=tan((M_PIl/2-centralParallel)/2);
  parradius=(ellip->geoc(ll.lat,0.,0.)).getx()/ellip->geteqr();
  cenparradius=(ellip->geoc(centralParallel,0.,0.)).getx()/ellip->geteqr();
  return pow(coneradius/cenconeradius,exponent)*cenparradius/parradius*scale;
}

/* North Carolina state plane, original:
 * ellipsoid Clarke
 * central meridian -79°
 * parallels 34°20' and 36°10'
 * 33°45'N 79°W = (609601.219202438405,0)
 * current:
 * ellipsoid GRS80
 * everything else the same
 */

const Quaternion rotateStereographic(1/14.,5/14.,7/14.,11/14.);
const Quaternion unrotateStereographic(-1/14.,5/14.,7/14.,11/14.);
/* This rotates (-96/196,-164/196,-48/196) to the South Pole, which is then
 * projected to infinity. This point is:
 * (-3120489.796,-5330836.735,-1560244.898) in ECEF coordinates,
 * (5,-0.292682926829,0.585365853659) in volleyball coordinates,
 * 14.1758035159S 120.343248884W in lat-long degrees,
 * 14°10'32.9"S 120.20°35.7'W in lat-long DMS,
 * 84561961.799S 717875442.017W in lat-long integer coordinates.
 * This point is in the Pacific Ocean over a megameter from land. It is highly
 * unlikely to be near any geoid file boundary, and neither a boldatni boundary
 * nor a cylinterval boundary can exactly hit it.
 */

StereographicSphere::StereographicSphere():Projection()
{
  rotation=1;
}

StereographicSphere::StereographicSphere(Quaternion Rotation):Projection()
{
  rotation=Rotation;
}

latlong StereographicSphere::gridToLatlong(xy grid)
{
  return gridToGeocentric(grid).latlon();
}

xyz StereographicSphere::gridToGeocentric(xy grid)
{
  double sf=scaleFactor(grid);
  return rotation.conj().rotate(xyz(grid/sf,ellip->getpor()*(2/sf-1)));
}

xy StereographicSphere::geocentricToGrid(xyz geoc)
{
  geoc=rotation.rotate(geoc);
  geoc.normalize();
  return xy(geoc)*ellip->getpor()*2/(geoc.getz()+1);
}

xy StereographicSphere::latlongToGrid(latlong ll)
{
  return geocentricToGrid(ellip->geoc(ll,0));
}

double StereographicSphere::scaleFactor(xy grid)
{
  return 1+sqr(grid.length()/ellip->getpor());
}

double StereographicSphere::scaleFactor(latlong ll)
{
  return scaleFactor(latlongToGrid(ll));
}
