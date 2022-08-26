/******************************************************/
/*                                                    */
/* curvefit.cpp - fit polyarc/alignment to points     */
/*                                                    */
/******************************************************/
/* Copyright 2022 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cassert>
#include "curvefit.h"
#include "ps.h"
#include "manysum.h"
#include "ldecimal.h"
#include "leastsquares.h"

using namespace std;

vector<double> curvefitResiduals(polyarc q,vector<xy> points)
/* The points must not be off the ends of q.
 */
{
  vector<double> ret;
  int i,bear;
  double along;
  for (i=0;i<points.size();i++)
  {
    along=q.closest(points[i]);
    bear=q.bearing(along);
    ret.push_back(distanceInDirection(q.station(along),points[i],bear));
  }
  return ret;
}
