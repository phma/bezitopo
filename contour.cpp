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
  int sd,io;
  triangle *tri;
  int i,j;
  //cout<<"Exterior edges:";
  for (io=0;io<2;io++)
    for (i=0;i<pts.edges.size();i++)
      if (io==pts.edges[i].isinterior())
      {
	tri=pts.edges[i].tria;
	if (!tri)
	  tri=pts.edges[i].trib;
	assert(tri);
	//cout<<' '<<i;
	for (j=0;j<3;j++)
	{
	  ep=j+(uintptr_t)&pts.edges[i];
	  sd=tri->subdir(ep);
	  if (tri->crosses(sd,elev) && (io || tri->upleft(sd)))
	  {
	    //cout<<(char)(j+'a');
	    ret.push_back(ep);
	  }
	}
      }
  //cout<<endl;
  return ret;
}

void mark(uintptr_t ep)
{
  ((edge *)(ep&-4))->mark(ep&3);
}

bool ismarked(uintptr_t ep)
{
  return ((edge *)(ep&-4))->ismarked(ep&3);
}

int midarcdir(xy a,xy b,xy c)
/* Returns the bearing of the arc abc at point b. May be off by 360°;
 * make sure consecutive bearings do not differ by more than 180°.
 */
{
  return dir(a,b)+dir(b,c)-dir(a,c);
}

polyline intrace(triangle *tri,double elev)
/* Returns the contour that is inside the triangle, if any. The contour is an elliptic curve.
 * If a contour is wholly inside a triangle, there is at most one contour partly in it.
 * If there is no contour wholly inside the triangle, there can be three partly inside it.
 * Start at a subsegment and trace the contour. One of three things will happen:
 * • You get a segment number greater than the number of subsegments (i.e. 65535). You've exited the triangle.
 * • You get a segment number less than the one you started with. You're retracing a contour you traced already.
 * • You get the segment number you started with. You've found a contour inside the triangle.
 */
{
  polyline ret(elev);
  int i,j,start;
  vector<int> sube;
  for (i=0;i<tri->subdiv.size();i++)
    if (tri->crosses(i,elev))
    {
      start=i;
      if (!tri->upleft(start))
	start+=65536;
      sube.clear();
      for (j=start;sube.size()==0 || (j&65535)>(start&65535) && (j&65535)<tri->subdiv.size();j=tri->proceed(j,elev))
	sube.push_back(j);
      if (j==start)
	break;
    }
  if (j==start)
    for (i=0;i<sube.size();i++)
      ret.insert(tri->contourcept(sube[i],elev));
  return ret;
}

polyline trace(uintptr_t edgep,double elev)
{
  polyline ret(elev);
  int subedge,subnext;
  uintptr_t prevedgep;
  bool wasmarked;
  triangle *tri,*ntri;
  tri=((edge *)(edgep&-4))->tria;
  ntri=((edge *)(edgep&-4))->trib;
  if (tri==nullptr || !tri->upleft(tri->subdir(edgep)))
    tri=ntri;
  mark(edgep);
  //cout<<"Start edgep "<<edgep<<endl;
  ret.insert(tri->contourcept(tri->subdir(edgep),elev));
  do
  {
    prevedgep=edgep;
    subedge=tri->subdir(edgep);
    //cout<<"before loop "<<subedge<<' '<<subnext<<endl;
    do
    {
      subnext=tri->proceed(subedge,elev);
      if (subnext>=0)
      {
	if (subnext==subedge)
	  cout<<"proceed failed!"<<endl;
	subedge=subnext;
	ret.insert(tri->contourcept(subedge,elev));
      }
    } while (subnext>=0);
    //cout<<"after loop "<<subedge<<' '<<subnext<<endl;
    edgep=tri->edgepart(subedge);
    //cout<<"Next edgep "<<edgep<<endl;
    if (edgep==prevedgep)
    {
      cout<<"Edge didn't change"<<endl;
      subedge=tri->subdir(edgep);
      subnext=tri->proceed(subedge,elev);
      if (subnext>=0)
      {
	if (subnext==subedge)
	  cout<<"proceed failed!"<<endl;
	subedge=subnext;
	//ret.insert(tri->contourcept(subedge,elev));
      }
    }
    if (edgep==0)
    {
      ntri=nullptr;
      cout<<"Tracing stopped in middle of a triangle"<<endl;
      subedge=tri->subdir(prevedgep);
      subnext=tri->proceed(subedge,elev);
    }
    else
    {
      wasmarked=ismarked(edgep);
      if (!wasmarked)
	ret.insert(tri->contourcept(tri->subdir(edgep),elev));
      else
	cout<<"Was already marked"<<endl;
      mark(edgep);
      ntri=((edge *)(edgep&-4))->othertri(tri);
    }
    if (ntri)
      tri=ntri;
  } while (ntri && !wasmarked);
  if (!ntri)
    ret.open();
  return ret;
}
