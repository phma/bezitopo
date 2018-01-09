/******************************************************/
/*                                                    */
/* ellipsoid.h - ellipsoids                           */
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
#ifndef ELLIPSOID_H
#define ELLIPSOID_H
#include <string>
#include <QString>
#include <QCoreApplication>
#include "point.h"
#include "angle.h"
#include "latlong.h"

class ellipsoid
{
  Q_DECLARE_TR_FUNCTIONS(ellipsoid)
private:
  double eqr,por;
  xyz cen;
  std::string name;
public:
  ellipsoid *sphere;
  ellipsoid(double equradius,double polradius,double flattening,xyz center,std::string ename);
  ~ellipsoid();
  xyz geoc(double lat,double lon,double elev);
  xyz geoc(latlong ll,double elev);
  xyz geoc(latlongelev lle);
  xyz geoc(int lat,int lon,int elev); // elev is in 1/65536 meter; for lat and long see angle.h
  latlongelev geod(xyz geocen);
  double avgradius();
  double geteqr()
  {
    return eqr;
  };
  double getpor()
  {
    return por;
  };
  xyz getCenter()
  {
    return cen;
  }
  std::string getName()
  {
    return name;
  }
  QString translateName()
  {
    return tr(name.c_str());
  }
  double eccentricity();
  double radiusAtLatitude(latlong ll,int bearing); // bearing is 0 for east; use DEG45 for average radius
  double conformalLatitude(double lat);
  double apxConLatDeriv(double lat);
  double inverseConformalLatitude(double lat);
};

extern ellipsoid Sphere,Clarke,GRS80,WGS84,ITRS;
int countEllipsoids();
ellipsoid& getEllipsoid(int n);
#endif
