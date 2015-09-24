/******************************************************/
/*                                                    */
/* ellipsoid.h - ellipsoids                           */
/*                                                    */
/******************************************************/
#ifndef ELLIPSOID_H
#define ELLIPSOID_H
#include "point.h"

class ellipsoid
{
private:
  double eqr,por;
public:
  ellipsoid *sphere;
  ellipsoid(double equradius,double polradius,double flattening);
  ~ellipsoid();
  xyz geoc(double lat,double lon,double elev);
  xyz geoc(int lat,int lon,int elev); // elev is in 1/65536 meter; for lat and long see angle.h
  double avgradius();
  double geteqr()
  {
    return eqr;
  };
  double getpor()
  {
    return por;
  };
};

extern ellipsoid Clarke,GRS80,WGS84,ITRS;
#endif
