/******************************************************/
/*                                                    */
/* ellipsoid.cpp - ellipsoids                         */
/*                                                    */
/******************************************************/
/* Copyright 2015-2018 Pierre Abbat.
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
#include "ellipsoid.h"
#include "rootfind.h"
using namespace std;

/* Unlike most of the program, which represents angles as integers,
 * ellipsoid and projection require double precision for angles.
 * With integers for angles, 1 ulp is 18.6 mm along the equator
 * or a meridian. The latitude transformation of the conformal map,
 * if done with integers, would result in 18.6 mm jumps, which
 * aren't good. Representing the zero point of a project in integers
 * is sufficiently accurate, but the calculations for doing so
 * need double.
 */

ellipsoid::ellipsoid(double equradius,double polradius,double flattening,xyz center,string ename)
{
  cen=center;
  name=ename;
  if (polradius==0)
    polradius=equradius*(1-flattening);
  else if (equradius==0)
    equradius=equradius/(1-flattening);
  eqr=equradius;
  por=polradius;
  if (eqr==por || std::isnan(eqr))
    sphere=this;
  else
    sphere=new ellipsoid(avgradius(),0,0,center,"");
}

ellipsoid::~ellipsoid()
{
  if (sphere!=this)
    delete sphere;
}

xyz ellipsoid::geoc(double lat,double lon,double elev)
/* Geocentric coordinates. (0,0,0) is the center of the earth.
 * (6378k,0,0) is in the Bight of Benin; (-6378k,0,0) is near Howland and Baker.
 * (0,6378k,0) is in the Indian Ocean; (0,-6378k,0) is in the Galápagos.
 * (0,0,6357k) is the North Pole; (0,0,-6357k) is the South Pole.
 * lat is positive north, long is positive east,elev is positive up.
 */
{
  xyz normal,ret;
  double z,cylr;
  z=sin(lat)*por;
  cylr=cos(lat)*eqr;
  ret=xyz(cylr*cos(lon),cylr*sin(lon),z);
  ret=ret/ret.length();
  normal=xyz(ret.east()*por,ret.north()*por,ret.elev()*eqr);
  ret=xyz(ret.east()*eqr,ret.north()*eqr,ret.elev()*por)+cen;
  normal=normal/normal.length();
  ret=ret+normal*elev;
  return ret;
}

xyz ellipsoid::geoc(int lat,int lon,int elev)
{
  return geoc(bintorad(lat),bintorad(lon),elev/65536.);
}

xyz ellipsoid::geoc(latlong ll,double elev)
{
  return geoc(ll.lat,ll.lon,elev);
}

xyz ellipsoid::geoc(latlongelev lle)
{
  return geoc(lle.lat,lle.lon,lle.elev);
}

latlongelev ellipsoid::geod(xyz geocen)
// Geodetic coordinates. Inverse of geoc.
{
  latlongelev ret;
  int i;
  xyz chk,normal,at0;
  double z,cylr,toler=avgradius()/1e15;
  geocen-=cen;
  z=geocen.getz();
  cylr=hypot(geocen.gety(),geocen.getx());
  ret.lon=atan2(geocen.gety(),geocen.getx());
  ret.lat=atan2(z*eqr/por,cylr*por/eqr);
  ret.elev=0;
  for (i=0;i<100;i++)
  {
    chk=geoc(ret);
    if (dist(chk,geocen)<toler)
      break;
    normal=sphere->geoc(ret);
    normal.normalize();
    ret.elev+=dot(geocen-chk,normal);
    at0=geocen-ret.elev*normal;
    z=at0.getz();
    cylr=hypot(at0.gety(),at0.getx());
    ret.lat=atan2(z*eqr/por,cylr*por/eqr);
  }
  if (i==100) // this can happen if the point is in the earth's core
    ret.lon=ret.lat=ret.elev=NAN;
  return ret;
}

double ellipsoid::avgradius()
{
  return cbrt(eqr*eqr*por);
}

double ellipsoid::eccentricity()
{
  return sqrt(1-por*por/eqr/eqr);
}

double ellipsoid::radiusAtLatitude(latlong ll,int bearing)
{
  double rprime; // radius in the prime (at east azimuth)
  double rmerid; // radius in the meridian (at north azimuth)
  double latfactor,bearfactor,ecc2;
  ecc2=1-por*por/eqr/eqr;
  latfactor=1-ecc2*sqr(sin(ll.lat));
  bearfactor=sqr(sin(bearing));
  rprime=eqr/sqrt(latfactor);
  rmerid=rprime*(1-ecc2)/latfactor;
  return 1/(bearfactor/rmerid+(1-bearfactor)/rprime);
}

double guder(double x)
{
  return atan(sinh(x));
}

double invGuder(double x)
{
  return asinh(tan(x));
}

double ellipsoid::conformalLatitude(double lat)
/* Returns the latitude on a sphere that a latitude on this ellipsoid
 * would conformally project to.
 * 
 * The formula using asin(tanh()) for the Gudermannian loses precision
 * when the latitude is near 90°.
 */
{
  double ecc=eccentricity();
  return guder(invGuder(lat)-ecc*atanh(ecc*sin(lat)));
}

latlong ellipsoid::conformalLatitude(latlong ll)
{
  return latlong(conformalLatitude(ll.lat),ll.lon);
}

double ellipsoid::apxConLatDeriv(double lat)
/* This is actually the geocentric latitude's derivative,
 * which is close enough for root finding purposes.
 * FIXME: this isn't really the geoc lat's deriv.
 */
{
  double x,z,x1,z1,x2,z2,rtsumsq,rtsumsq1,rtsumsq2;
  x=cos(lat);
  z=sin(lat);
  rtsumsq=sqrt(sqr(x*eqr)+sqr(z*por));
  x1=x*eqr/rtsumsq;
  z1=z*por/rtsumsq;
  rtsumsq1=sqrt(sqr(x1*por)+sqr(z1*eqr));
  return sqr(rtsumsq1/rtsumsq);
}

double ellipsoid::inverseConformalLatitude(double lat)
{
  double ret;
  Newton ne;
  double lo=lat*por/eqr,hi=(lat-M_PI/2)*por/eqr+M_PI/2;
  ret=ne.init(lo,conformalLatitude(lo)-lat,apxConLatDeriv(lo),
              hi,conformalLatitude(hi)-lat,apxConLatDeriv(hi));
  while (!ne.finished())
  {
    ret=ne.step(conformalLatitude(ret)-lat,apxConLatDeriv(ret));
  }
  return ret;
}

latlong ellipsoid::inverseConformalLatitude(latlong ll)
{
  return latlong(inverseConformalLatitude(ll.lat),ll.lon);
}

double ellipsoid::scaleFactor(double ellipsoidLatitude,double sphereLatitude)
// Distance between points on the ellipsoid, divided by distance on the sphere.
{
  double ellipsoidRadius,sphereRadius; // radius of circle of latitude
  ellipsoidRadius=geoc(ellipsoidLatitude,0.,0.).getx();
  sphereRadius=sphere->geoc(sphereLatitude,0.,0.).getx();
  if (ellipsoidLatitude>bintorad(DEG90-256) || sphereLatitude>bintorad(DEG90-256))
    return pow(eqr/por,4/3.)/exp(eccentricity()*atanh(eccentricity()));
  else
    return ellipsoidRadius/sphereRadius;
}

ellipsoid Sphere(6371000,0,0,xyz(0,0,0),"Sphere");
ellipsoid Clarke(6378206.4,6356583.8,0,xyz(0,0,0),"Clarke");
ellipsoid GRS80(6378137,0,1/298.257222101,xyz(0,0,0),"GRS80");
ellipsoid WGS84(6378137,0,1/298.257223563,xyz(0,0,0),"WGS84");
ellipsoid ITRS(6378136.49,0,1/298.25645,xyz(0,0,0),"ITRS");
/* The center of Clarke is NOT (0,0,0), and the ellipsoid used for NAD 83
 * is about 2.24 m off from that used in the 2022 datum, but I haven't found
 * exact values.
 */
ellipsoid *ellipsoids[]={&Sphere,&Clarke,&GRS80,&WGS84,&ITRS};

int countEllipsoids()
{
  return sizeof(ellipsoids)/sizeof(ellipsoids[0]);
}

ellipsoid& getEllipsoid(int n)
{
  return *ellipsoids[n];
}

ellipsoid *getEllipsoid(string name)
{
  int i;
  ellipsoid *ret=nullptr;
  for (i=0;i<countEllipsoids();i++)
    if (name==ellipsoids[i]->getName())
      ret=ellipsoids[i];
  return ret;
}
