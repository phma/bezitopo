/******************************************************/
/*                                                    */
/* ellipsoid.h - ellipsoids                           */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#ifndef ELLIPSOID_H
#define ELLIPSOID_H
#include "point.h"
#include "angle.h"

class ellipsoid
{
private:
  double eqr,por;
public:
  ellipsoid *sphere;
  ellipsoid(double equradius,double polradius,double flattening);
  ~ellipsoid();
  xyz geoc(double lat,double lon,double elev);
  xyz geoc(latlong ll,double elev);
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
  double eccentricity();
  double radiusAtLatitude(latlong ll,int bearing); // bearing is 0 for east; use DEG45 for average radius
};

extern ellipsoid Sphere,Clarke,GRS80,WGS84,ITRS;
#endif
