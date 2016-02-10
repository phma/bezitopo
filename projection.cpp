/******************************************************/
/*                                                    */
/* projection.cpp - map projections                   */
/*                                                    */
/******************************************************/
#include <cmath>
#include "projection.h"

Projection::Projection()
{
  ellip=&Sphere;
  offset=xy(0,0);
  scale=1;
}

LambertConicSphere::LambertConicSphere():Projection()
{
  centralMeridian=0;
  centralParallel=0;
  poleY=INFINITY;
  exponent=0;
}

latlong LambertConicSphere::gridToLatlong(xy grid)
{
  latlong ret;
  ret.lat=0;
  ret.lon=0;
  return ret;
}

xyz LambertConicSphere::gridToGeocentric(xy grid)
{
  return xyz(6371e3,0,0);
}

xy LambertConicSphere::geocentricToGrid(xyz geoc)
{
  return xy(0,0);
}

xy LambertConicSphere::latlongToGrid(latlong ll)
{
  double radius,angle,northing,easting;
  radius=tan((M_PIl-ll.lat)/4);
  angle=ll.lon-centralMeridian;
  while(angle>M_PIl*2)
    angle-=M_PIl*2;
  while(angle<-M_PIl*2)
    angle+=M_PIl*2;
  if (exponent==0)
  {
    easting=angle*ellip->geteqr();
    northing=-2*log(radius)*ellip->getpor();
  }
  return xy(easting,northing)*scale+offset;
}

double LambertConicSphere::scaleFactor(xy grid)
{
  return 1;
}

double LambertConicSphere::scaleFactor(latlong ll)
{
  return 1;
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