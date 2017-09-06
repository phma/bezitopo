/******************************************************/
/*                                                    */
/* zoom.cpp - zoom ratios for viewing                 */
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
#include <map>
#include <cmath>
#include "zoom.h"
#define ZOOMSTEP 1.259423
/* Used in GUI windows for zooming. 3 zoom steps are just under a ratio of 2;
 * 10 zoom steps are just over a ratio of 10.
 */

using namespace std;

map<int,double> zooms;

double zoomratio(int n)
{
  double ret=zooms[n];
  if (ret==0)
    zooms[n]=ret=pow(ZOOMSTEP,n);
  return ret;
}
