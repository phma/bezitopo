/******************************************************/
/*                                                    */
/* readtin.cpp - read a TIN into a pointlist          */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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

#include "readtin.h"
#include "dxf.h"

using namespace std;

void readTinFile(pointlist &pl,string fileName,double unit)
{
  int i,j;
  vector<array<xyz,3> > bareTriangles;
  ifstream file;
  bareTriangles=extractTriangles(readDxfGroups(fileName));
  if (unit!=1)
    for (i=0;i<bareTriangles.size();i++)
      for (j=0;j<3;j++)
	bareTriangles[i][j]*=unit;
  pl.makeBareTriangles(bareTriangles);
  bareTriangles.clear();
  cout<<"Read "<<pl.triangles.size()<<" triangles\n";
  pl.fillInBareTin();
  cout<<pl.triangles.size()<<" triangles after filling in\n";
  pl.addperimeter();
}
