/******************************************************/
/*                                                    */
/* brent.cpp - Brent's root-finding method            */
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
#include <utility>
#include <iostream>
#include "ldecimal.h"
#include "brent.h"

using namespace std;

double invquad(double x0,double y0,double x1,double y1,double x2,double y2)
{
  double z0,z1,z2,offx;
  z0=x0;
  z1=x1;
  z2=x2;
  if (z0>z1)
    swap(z0,z1);
  if (z1>z2)
    swap(z1,z2);
  if (z0>z1)
    swap(z0,z1);
  //cout<<"Brent: "<<ldecimal(x0)<<' '<<ldecimal(x1)<<' '<<ldecimal(x2)<<endl;
  //cout<<"Before: "<<ldecimal(x0-x1)<<' '<<ldecimal(x1-x2)<<' '<<ldecimal(x2-x0)<<endl;
  if (z0<0 && z2>0)
    offx=0;
  if (z0>=0)
    offx=z0;
  if (z2<=0)
    offx=z2;
  x0-=offx;
  x1-=offx;
  x2-=offx;
  //cout<<"After:  "<<ldecimal(x0-x1)<<' '<<ldecimal(x1-x2)<<' '<<ldecimal(x2-x0)<<endl;
  z0=x0*y1*y2/(y0-y1)/(y0-y2);
  z1=x1*y2*y0/(y1-y2)/(y1-y0);
  z2=x2*y0*y1/(y2-y0)/(y2-y1);
  return (z0+z1+z2)+offx;
}
