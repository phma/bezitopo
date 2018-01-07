/******************************************************/
/*                                                    */
/* transmer.cpp - series for transverse Mercator      */
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
#include <string>
#include <iostream>
#include "polyline.h"
#include "projection.h"
#include "ellipsoid.h"

using namespace std;

vector<string> args;

int main(int argc, char *argv[])
{
  int i;
  for (i=1;i<argc;i++)
    args.push_back(argv[i]);
  for (i=0;i<countEllipsoids();i++)
    cout<<getEllipsoid(i).getName()<<endl;
  return 0;
}
