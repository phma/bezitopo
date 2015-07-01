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
#include "contour.h"
#include "pointlist.h"
using namespace std;

vector<uintptr_t> contstarts(pointlist &pts,double elev)
{
  vector<uintptr_t> ret;
  int i;
  cout<<"Exterior edges:";
  for (i=0;i<pts.edges.size();i++)
    if (!pts.edges[i].isinterior())
    {
      cout<<' '<<i;
    }
  cout<<endl;
  return ret;
}
