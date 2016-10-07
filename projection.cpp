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

/*xy LambertConicSphere::geocentricToGrid(xyz geoc)
{
  return xy(0,0);
}*/

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