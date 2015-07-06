/******************************************************/
/*                                                    */
/* contour.cpp - generates contours                   */
/*                                                    */
/******************************************************/

/* After finding extrema, generate contours. The method for generating contours
  as polylines is as follows:
  1. Between two corners (i.e. points in the TIN, corners of triangles) in the TIN
     that includes the extrema, find a point on the edge that has the given elevation.
     Join these points with line segments.
  2. Add points to each line segment, staying within the triangle, until they are
     closer together than the tolerance. Keep the circular list of points.
  3. Make a curve defined by three of these points (which is necessarily a circle).
  4. Add points from the list to make the curve fit the points better.
  */
#include <iostream>
#include <cassert>
#include "contour.h"
#include "pointlist.h"
using namespace std;

vector<uintptr_t> contstarts(pointlist &pts,double elev)
{
  vector<uintptr_t> ret;
  uintptr_t ep;
  int sd;
  triangle *tri;
  int i,j;
  cout<<"Exterior edges:";
  for (i=0;i<pts.edges.size();i++)
    if (!pts.edges[i].isinterior())
    {
      tri=pts.edges[i].tria;
      if (!tri)
	tri=pts.edges[i].trib;
      assert(tri);
      cout<<' '<<i;
      for (j=0;j<3;j++)
      {
	ep=j+(uintptr_t)&pts.edges[i];
	sd=tri->subdir(ep);
	if (tri->crosses(sd,elev) && tri->upleft(sd))
	{
	  cout<<(char)(j+'a');
	  ret.push_back(ep);
	}
      }
    }
  cout<<endl;
  return ret;
}
