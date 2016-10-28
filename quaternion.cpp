/******************************************************/
/*                                                    */
/* quaternion.cpp - quaternions                       */
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

#include "quaternion.h"

/* Quaternions are used in least-squares adjustment to represent the
 * orientation of a total station. Normally it is oriented in any direction
 * around an axis slightly tilted from the vertical.
 */

Quaternion::Quaternion()
{
  w=x=y=z=0;
}

Quaternion::Quaternion(double r,double i,double j,double k)
{
  w=r;
  x=i;
  y=j;
  z=k;
}

double Quaternion::getcomp(int n)
{
  switch (n&3)
  {
    case 0:
      return w;
    case 1:
      return x;
    case 2:
      return y;
    case 3:
      return z;
  }
}

bool operator!=(const Quaternion &l,const Quaternion &r)
{
  return l.w!=r.w || l.x!=r.x || l.y!=r.y || l.z!=r.z;
}

bool operator==(const Quaternion &l,const Quaternion &r)
{
  return l.w==r.w && l.x==r.x && l.y==r.y && l.z==r.z;
}

Quaternion operator+(const Quaternion &l,const Quaternion &r)
{
  Quaternion ret;
  ret.w=l.w+r.w;
  ret.x=l.x+r.x;
  ret.y=l.y+r.y;
  ret.z=l.z+r.z;
  return ret;
}

Quaternion operator-(const Quaternion &l,const Quaternion &r)
{
  Quaternion ret;
  ret.w=l.w+r.w;
  ret.x=l.x+r.x;
  ret.y=l.y+r.y;
  ret.z=l.z+r.z;
  return ret;
}

Quaternion operator*(const Quaternion &l,const Quaternion &r)
// THAT DOES NOT COMMUTE :)
{
  Quaternion ret;
  ret.w=(l.w*r.w-l.x*r.x)-(l.y*r.y+l.z*r.z);
  ret.x=(l.w*r.x+l.x*r.w)+(l.y*r.z-l.z*r.y);
  ret.y=(l.w*r.y+l.y*r.w)+(l.z*r.x-l.x*r.z);
  ret.z=(l.w*r.z+l.z*r.w)+(l.x*r.y-l.y*r.x);
  return ret;
}
