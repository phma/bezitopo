/******************************************************/
/*                                                    */
/* ellipsoid.h - ellipsoids                           */
/*                                                    */
/******************************************************/
#include "point.h"

class ellipsoid
{
private:
  double eqr,por;
public:
  ellipsoid(double equradius,double polradius,double flattening);
  xyz geoc(int lat,int lon,int elev); // elev is in 1/65536 meter; for lat and long see angle.h
};

extern ellipsoid Clarke,GRS80,WGS84,ITRS;
