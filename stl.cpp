/******************************************************/
/*                                                    */
/* stl.cpp - stereolithography (3D printing) export   */
/*                                                    */
/******************************************************/
/* Copyright 2013,2015,2019 Pierre Abbat.
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

#include "stl.h"
#include "smooth5.h"
using namespace std;

/* The STL polyhedron consists of three kinds of face: bottom, side, and top.
 * The bottom is a convex polygon which is triangulated. The sides are
 * trapezoids, each of which is drawn as two triangles. The top is the
 * TIN surface. Each edge is split into some number of pieces which is a
 * power of 2, enough to make it smooth at the printing scale, then some
 * are split into more pieces because every triangle must have one side that
 * is split into a number of pieces that is a multiple of the number of
 * pieces that the other two sides are split into, which must be equal.
 */

vector<int> stltable;
vector<short> stlfac;

void initStlTable()
{
  int i,fac,j,n;
  if (stltable.size()!=216)
  {
    stltable.clear();
    for (i=1;i<=7776000;i++)
      if (7776000%i==0 && smooth5(i))
      {
	stltable.push_back(i);
	fac=0;
	n=i;
	for (j=0;j<9;j++)
	  if (n%2==0)
	  {
	    n/=2;
	    fac++;
	  }
	for (j=0;j<6;j++)
	  if (n%3==0)
	  {
	    n/=3;
	    fac+=16;
	  }
	for (j=0;j<4;j++)
	  if (n%5==0)
	  {
	    n/=5;
	    fac+=256;
	  }
	stlfac.push_back(fac);
      }
    stltable.shrink_to_fit();
    stlfac.shrink_to_fit();
  }
}

stltriangle::stltriangle()
{
  normal=a=b=c=xyz(0,0,0);
}

stltriangle::stltriangle(xyz A,xyz B,xyz C)
{
  a=A;
  b=B;
  c=C;
}
