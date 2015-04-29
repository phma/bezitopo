/******************************************************/
/*                                                    */
/* ellipsoid.cpp - ellipsoids                         */
/*                                                    */
/******************************************************/
#include "angle.h"
#include "ellipsoid.h"

ellipsoid::ellipsoid(double equradius,double polradius,double flattening)
{
  if (polradius==0)
    polradius=equradius*(1-flattening);
  else if (equradius==0)
    equradius=equradius/(1-flattening);
  eqr=equradius;
  por=polradius;
}

xyz ellipsoid::geoc(int lat,int lon,int elev)
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
  ret=xyz(ret.east()*eqr,ret.north()*eqr,ret.elev()*por);
  normal=normal/normal.length();
  ret=ret+normal*(elev/65536.);
  return ret;
}

ellipsoid Clarke(6378206.4,6356583.8,0);
ellipsoid GRS80(6378137,0,1/298.257222101);
ellipsoid WGS84(6378137,0,1/298.257223563);
ellipsoid ITRS(6378136.49,0,1/298.25645);
