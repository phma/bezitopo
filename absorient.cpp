/******************************************************/
/*                                                    */
/* absorient.cpp - 2D absolute orientation            */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Given two pointlists and a list of matching points,
 * find the rotation and translation to match them with the
 * least sum of square distances. This is called the
 * absolute orientation problem.
 */
#include <assert.h>
#include "absorient.h"
#include "manysum.h"
using namespace std;

double sumsqdist(vector<xy> a,vector<xy> b)
{
  int i;
  manysum acc;
  assert(a.size()==b.size());
  for (i=0;i<a.size();i++)
    acc+=sqr(dist(a[i],b[i]));
  return acc.total();
}
