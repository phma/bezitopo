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

#include <cassert>
#include "polyline.h"
#include "manysum.h"
using namespace std;

polyline::polyline()
{
  elevation=0;
}

polyline::polyline(double e)
{
  elevation=e;
}

bool polyline::isopen()
{
  return endpoints.size()>lengths.size();
}

int polyline::size()
{
  return lengths.size();
}

segment polyline::getsegment(int i)
{
  i%=lengths.size();
  if (i<0)
    i+=lengths.size();
  return segment(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation));
}

arc polyarc::getarc(int i)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  return arc(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation),deltas[i]);
}

bezier3d polyline::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getsegment(i).approx3d(precision);
  return ret;
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
  bool wasopen;
  int i;
  vector<xy>::iterator ptit;
  vector<double>::iterator lenit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  endpoints.insert(ptit,newpoint);
  lengths.insert(lenit,0);
  pos--;
  if (pos<0)
    if (wasopen)
      pos=0;
    else
      pos+=endpoints.size();
  for (i=0;i<2;i++)
  {
    if (pos+1<endpoints.size())
      lengths[pos]=dist(endpoints[pos],endpoints[pos+1]);
    if (pos+1==endpoints.size() && !wasopen)
      lengths[pos]=dist(endpoints[pos],endpoints[0]);
    pos++;
    if (pos>=lengths.size())
      pos=0;
  }
}

void polyarc::insert(xy newpoint,int pos)
{
  bool wasopen;
  vector<xy>::iterator ptit;
  vector<int>::iterator arcit;
  vector<double>::iterator lenit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  arcit=deltas.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
  lengths.insert(lenit,0);
}

void polyline::setlengths()
{
  int i;
  for (i=0;i<lengths.size();i++)
    lengths[i]=getsegment(i).length();
}

void polyarc::setlengths()
{
  int i;
  assert(lengths.size()==deltas.size());
  for (i=0;i<deltas.size();i++)
    lengths[i]=getarc(i).length();
}

void polyarc::setdelta(int i,int delta)
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
    }
  return a.total();
}

double polyarc::area()
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
  lengths.resize(endpoints.size()-1);
}

void polyarc::open()
{
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
}

void polyline::close()
{
  lengths.resize(endpoints.size());
}

void polyarc::close()
{
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
}
