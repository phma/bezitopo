/******************************************************/
/*                                                    */
/* quaternion.h - quaternions                         */
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

#ifndef QUATERNION_H
#define QUATERNION_H

class Quaternion;

#include "xyz.h"

class Quaternion
{
public:
  Quaternion();
  Quaternion(double r);
  Quaternion(double r,double i,double j,double k);
  Quaternion(double r,xyz i);
  double getcomp(int n);
  double getreal();
  xyz getimag();
  double normsq();
  double norm();
  void normalize();
  Quaternion conj();
  Quaternion inv();
  xyz rotate(xyz vec);
  friend bool operator!=(const Quaternion &l,const Quaternion &r);
  friend bool operator==(const Quaternion &l,const Quaternion &r);
  friend Quaternion operator+(const Quaternion &l,const Quaternion &r);
  friend Quaternion operator-(const Quaternion &l,const Quaternion &r);
  friend Quaternion operator*(const Quaternion &l,const Quaternion &r);
  friend Quaternion operator*(const Quaternion &l,double r);
  friend Quaternion operator/(const Quaternion &l,double r);
  // operator/ for two quaternions is not defined.
  // Multiply by the reciprocal on the appropriate side.
  Quaternion& operator+=(double r);
  Quaternion& operator-=(double r);
  Quaternion& operator*=(double r);
  Quaternion& operator/=(double r);
  friend Quaternion versor(xyz vec);
  friend Quaternion versor(xyz vec,int angle);
protected:
  double w,x,y,z;
};

#endif
