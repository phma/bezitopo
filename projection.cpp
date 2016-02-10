/******************************************************/
/*                                                    */
/* projection.cpp - map projections                   */
/*                                                    */
/******************************************************/
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
  return xy(0,0);
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