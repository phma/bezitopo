/******************************************************/
/*                                                    */
/* carlsontin.cpp - input TIN in Carlson DTM format   */
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

#include <fstream>
#include "binio.h"
#include "carlsontin.h"
using namespace std;

#define CA_POINT 0x1c01
#define CA_TRIANGLE 0xd04

bool readCarlsonTin(std::string inputFile,pointlist &pl,double unit)
{
  ifstream tinFile(inputFile,ios::binary);
  int i,n,tag,ptnum,corners[3];
  double x,y,z;
  bool good=false,cont=true;
  triangle *tri;
  for (i=0;i<0x43;i++) // Skip 0x10c bytes of header. I have no idea what any
    readleint(tinFile); // of it means, except the string at the start.
  pl.clear();
  while (cont)
  {
    tag=readleshort(tinFile);
    switch (tag)
    {
      case CA_POINT:
	ptnum=readleint(tinFile);
	x=readledouble(tinFile)*unit;
	y=readledouble(tinFile)*unit;
	z=readledouble(tinFile)*unit;
	if (pl.points.count(ptnum))
	  good=cont=false; // point number repeated
	else if (!(fabs(x)<4e7 && fabs(y)<4e7 && fabs(z)<12000))
	  good=cont=false; // point is bigger than Earth, or is NaN
	else
	{
	  pl.points[ptnum]=point(x,y,z,"");
	  pl.revpoints[&pl.points[ptnum]]=ptnum;
	}
	if (pl.points.size()==3)
	  good=true;
	break;
      case CA_TRIANGLE:
	for (i=0;i<3;i++)
	{
	  corners[i]=readleint(tinFile);
	  if (pl.points.count(corners[i])==0)
	    good=cont=false;
	}
	tinFile.get(); // this is a 0 written for an unknown reason
	n=pl.addtriangle();
	tri=&pl.triangles[n];
	tri->a=&pl.points[corners[0]];
	tri->b=&pl.points[corners[1]];
	tri->c=&pl.points[corners[2]];
	tri->flatten();
	if (tri->sarea<=0)
	  good=cont=false;
	break;
      default:
	if (tinFile.good())
	  good=false;
	cont=false;
    }
  }
  return good;
}
