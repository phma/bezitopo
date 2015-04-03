/******************************************************/
/*                                                    */
/* polyline.cpp - polylines                           */
/*                                                    */
/******************************************************/

/* A polyline can be open or closed. Smoothing a polyline does this:
   If it is open, it tries various starting angles until it finds the shortest total length.
   If it is closed and has an odd number of points, it computes the two starting angles
   that make the ending angle the same, and chooses the one with the shortest total length.
   If it is closed and has an even number of points, it distributes the angular misclosure
   evenly among the points, and makes the shortest total length.
   */

#include "polyline.h"
#include "manysum.h"
using namespace std;

bool polyline::isopen()
{
  return endpoints.size()>deltas.size();
}

arc polyline::getarc(int i)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  return arc(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation),deltas[i]);
}

void polyline::insert(xy newpoint,int pos)
/* Inserts newpoint in position pos. E.g. insert(xy(8,5),2) does
 * {(0,0),(1,1),(2,2),(3,3)} -> {(0,0),(1,1),(8,5),(2,2),(3,3)}.
 * If the polyline is open, inserting a point in position 0, -1, or after the last
 * (-1 means after the last) results in adding a line segment.
 * If the polyline is empty (and therefore closed), inserting a point results in
 * adding a line segment from that point to itself.
 * In all other cases, newpoint is inserted into an arc, whose delta is split
 * proportionally to the distances to the adjacent points.
 */
{
  xy *prevpt,*nextpt;
  int *prevarc,*nextarc;
  bool wasopen;
  vector<xy>::iterator ptit;
  vector<int>::iterator arcit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  arcit=deltas.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
}

void polyline::setlengths()
{
  int i;
  lengths.resize(deltas.size());
  for (i=0;i<deltas.size();i++)
    lengths[i]=getarc(i).length();
}

void polyline::setdelta(int i,int delta)
{
  deltas[i%deltas.size()]=delta;
}

double polyline::length()
{
  int i;
  double len;
  for (len=i=0;i<lengths.size();i++)
    len+=lengths[i];
  return len;
}

double polyline::area()
{
  int i;
  xy startpnt;
  manysum a;
  if (endpoints.size())
    startpnt=endpoints[0];
  if (isopen())
    a+=NAN;
  else
    for (i=0;i<lengths.size();i++)
    {
      a+=area3(startpnt,endpoints[i],endpoints[(i+1)%endpoints.size()]);
      a+=getarc(i).diffarea();
    }
  return a.total();
}

void polyline::open()
{
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
}

void polyline::close()
{
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
}
