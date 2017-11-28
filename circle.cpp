/******************************************************/
/*                                                    */
/* circle.cpp - circles, including lines              */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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

#include "circle.h"
#include "angle.h"

Circle::Circle()
// Constructs the x-axis.
{
  mid=xy(0,0);
  bear=0;
  cur=0;
}

Circle::Circle(xy c,double r)
{
  bear=atan2i(c);
  mid=c-r*cossin(bear);
  bear-=DEG90;
  cur=1/r;
}

Circle::Circle(xy m,int b,double c)
{
  mid=m;
  bear=b;
  cur=c;
}
