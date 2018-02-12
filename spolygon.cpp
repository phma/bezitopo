/******************************************************/
/*                                                    */
/* spolygon.cpp - spherical polygons                  */
/*                                                    */
/******************************************************/
/* Copyright 2017-2018 Pierre Abbat.
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
#include <cassert>
#include "angle.h"
#include "spolygon.h"
#include "geoid.h"
#include "manysum.h"
using namespace std;

double surfaceLength(double throughLength)
{
  assert(throughLength<2.001*EARTHRAD);
  if (throughLength>=2*EARTHRAD)
    return M_PI*EARTHRAD;
  else
    return 2*EARTHRAD*asin(throughLength/2/EARTHRAD);
}

double surfacePerimeter(vector<xyz> polygon)
/* All vertices of polygon are assumed to be at the surface of the earth,
 * as provided by decodedir.
 */
{
  vector<double> sideLength(polygon.size(),0);
  int i;
  for (i=0;i<polygon.size();i++)
    sideLength[i]=surfaceLength(dist(polygon[i],polygon[(i+1)%polygon.size()]));
  return pairwisesum(sideLength);
}

/* To compute the area of a boundary (which is needed only for testing; area
 * can be computed easier by asking the geoquads), add up the spherical
 * deflection angles and subtract from 2π. To compute the deflection angle ABC:
 * • AXB=A×B; BXC=B×C;
 * • Normalize AXB, BXC, and B;
 * • (AXB×BXC)·B is the sine of the angle; AXB·BXC is the cosine.
 * 
 * There is no difference between a polygon traversed clockwise, thus having
 * a negative area, and a polygon traversed counterclockwise and containing
 * almost all the earth. Areas are thus expressed as 32-bit integers like
 * angles. One ulp is about 119 dunams.
 */

double deflectionAngle(xyz a,xyz b,xyz c)
{
  xyz axb,bxc;
  axb=cross(a,b);
  bxc=cross(b,c);
  axb.normalize();
  b.normalize();
  bxc.normalize();
  return atan2(dot(cross(axb,bxc),b),dot(axb,bxc));
}

int iSurfaceArea(std::vector<xyz> polygon)
{
  vector<double> dangle;
  int i;
  dangle.resize(polygon.size());
  for (i=0;i<polygon.size();i++)
    dangle[i]=deflectionAngle(polygon[(i+polygon.size()-1)%polygon.size()],
			      polygon[i],
			      polygon[(i+1)%polygon.size()]);
  return (polygon.size()?DEG360:0)-radtobin(pairwisesum(dangle));
  // The area of an empty polygon is 0. Without the ?;, it would be 2π.
}
