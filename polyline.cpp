/******************************************************/
/*                                                    */
/* polyline.cpp - polylines                           */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014-2019 Pierre Abbat.
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

/* A polyline can be open or closed. Smoothing a polyspiral does this:
 * Each point, except the ends of an open polyspiral, is assigned the bearing
 * at that point of the arc that passes through its predecessor, it, and
 * its successor. Each segment of the polyspiral is redrawn to be tangent
 * to the bearings at its ends, unless it would exceed bendlimit, in which case
 * it is redrawn as a straight line.
 */

#include <cassert>
#include <iostream>
#include "polyline.h"
#include "manysum.h"
#include "relprime.h"
#include "ldecimal.h"
using namespace std;
int bendlimit=DEG180;

int midarcdir(xy a,xy b,xy c)
/* Returns the bearing of the arc abc at point b. May be off by 360°;
 * make sure consecutive bearings do not differ by more than 180°.
 */
{
  return dir(a,b)+dir(b,c)-dir(a,c);
}

polyline::polyline()
{
  elevation=0;
}

polyarc::polyarc(): polyline::polyline()
{
}

polyspiral::polyspiral(): polyarc::polyarc()
{
  curvy=true;
}

polyline::polyline(double e)
{
  elevation=e;
}

polyarc::polyarc(double e): polyline::polyline(e)
{
}

polyspiral::polyspiral(double e): polyarc::polyarc(e)
{
  curvy=true;
}

polyarc::polyarc(polyline &p)
{
  elevation=p.elevation;
  endpoints=p.endpoints;
  lengths=p.lengths;
  boundCircles=p.boundCircles;
  cumLengths=p.cumLengths;
  deltas.resize(lengths.size());
}

polyspiral::polyspiral(polyline &p)
{
  int i,j;
  elevation=p.elevation;
  endpoints=p.endpoints;
  lengths=p.lengths;
  boundCircles=p.boundCircles;
  cumLengths=p.cumLengths;
  deltas.resize(lengths.size());
  delta2s.resize(lengths.size());
  bearings.resize(endpoints.size());
  midbearings.resize(lengths.size());
  midpoints.resize(lengths.size());
  clothances.resize(lengths.size());
  curvatures.resize(lengths.size());
  for (i=0;i<lengths.size();i++)
  {
    j=(i+1==endpoints.size())?0:i+1;
    midpoints[i]=(endpoints[i]+endpoints[j])/2;
    midbearings[i]=dir(endpoints[i],endpoints[j]);
    if (i)
      midbearings[i]=midbearings[i-1]+foldangle(midbearings[i]-midbearings[i-1]);
    bearings[i]=midbearings[i];
  }
  curvy=false;
}

bool polyline::isopen()
{
  return endpoints.size()>lengths.size();
}

int polyline::size()
{
  return lengths.size();
}

unsigned polyline::hash()
{
  return memHash(&lengths[0],lengths.size()*sizeof(double),
         memHash(&cumLengths[0],cumLengths.size()*sizeof(double),
         memHash(&endpoints[0],endpoints.size()*sizeof(xy),
         memHash(&elevation,sizeof(double)))));
}

unsigned polyarc::hash()
{
  return memHash(&deltas[0],deltas.size()*sizeof(int),
         memHash(&lengths[0],lengths.size()*sizeof(double),
         memHash(&cumLengths[0],cumLengths.size()*sizeof(double),
         memHash(&endpoints[0],endpoints.size()*sizeof(xy),
         memHash(&elevation,sizeof(double))))));
}

unsigned polyspiral::hash()
{
  return memHash(&bearings[0],bearings.size()*sizeof(int),
         memHash(&delta2s[0],delta2s.size()*sizeof(int),
         memHash(&midbearings[0],midbearings.size()*sizeof(int),
         memHash(&midpoints[0],midpoints.size()*sizeof(xy),
         memHash(&curvatures[0],curvatures.size()*sizeof(double),
         memHash(&clothances[0],clothances.size()*sizeof(double),
         memHash(&deltas[0],deltas.size()*sizeof(int),
         memHash(&lengths[0],lengths.size()*sizeof(double),
         memHash(&cumLengths[0],cumLengths.size()*sizeof(double),
         memHash(&endpoints[0],endpoints.size()*sizeof(xy),
         memHash(&elevation,sizeof(double))))))))))));
}

segment polyline::getsegment(int i)
{
  i%=(signed)lengths.size();
  if (i<0)
    i+=lengths.size();
  return segment(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation));
}

arc polyarc::getarc(int i)
{
  i%=(signed)deltas.size();
  if (i<0)
    i+=deltas.size();
  return arc(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation),deltas[i]);
}

spiralarc polyspiral::getspiralarc(int i)
{
  i%=(signed)deltas.size();
  if (i<0)
    i+=deltas.size();
  return spiralarc(xyz(endpoints[i],elevation),xyz(midpoints[i],elevation),
		   xyz(endpoints[(i+1)%endpoints.size()],elevation),midbearings[i],
		   curvatures[i],clothances[i],lengths[i]);
}

xyz polyline::getEndpoint(int i)
{
  i%=endpoints.size();
  if (i<0)
    i+=endpoints.size();
  return xyz(endpoints[i],elevation);
}

xyz polyline::getstart()
{
  return getEndpoint(0);
}

xyz polyline::getend()
// If it's closed, this is equivalent to getstart.
{
  return getEndpoint(lengths.size());
}

bezier3d polyline::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getsegment(i).approx3d(precision);
  if (!isopen())
    ret.close();
  return ret;
}

bezier3d polyarc::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getarc(i).approx3d(precision);
  if (!isopen())
    ret.close();
  return ret;
}

bezier3d polyspiral::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getspiralarc(i).approx3d(precision);
  if (!isopen())
    ret.close();
  return ret;
}

vector<drawingElement> polyline::render3d(double precision,int layer,int color,int width,int linetype)
{
  vector<drawingElement> ret;
  ret.push_back(drawingElement(approx3d(precision),color,width,linetype));
  // polylines aren't filled, but if they are in the future, set filled=true
  return ret;
}

void polyline::dedup()
/* Collapses into one adjacent points that are too close together.
 * They result from running contourcept on two segments that are the same
 * in opposite directions.
 */
{
  int h,i,j,k;
  vector<xy>::iterator ptit;
  vector<double>::iterator lenit;
  vector<bcir>::iterator bcit;
  xy avg;
  //if (dist(endpoints[0],xy(999992.534,1499993.823))<0.001)
  //  cout<<"Debug contour\r";
  for (i=0;i<endpoints.size() && endpoints.size()>2;i++)
  {
    h=i-1;
    if (h<0)
      if (isopen())
	h=0;
      else
	h+=endpoints.size();
    j=i+1;
    if (j>=endpoints.size())
      if (isopen())
	j--;
      else
	j-=endpoints.size();
    k=j+1;
    if (k>=endpoints.size())
      if (isopen())
	k--;
      else
	k-=endpoints.size();
    if (i!=j && (dist(endpoints[i],endpoints[j])*16777216<=dist(endpoints[h],endpoints[i]) || dist(endpoints[i],endpoints[j])*16777216<=dist(endpoints[j],endpoints[k]) || dist(endpoints[i],endpoints[j])*281474976710656.<=dist(endpoints[i],-endpoints[j]) || endpoints[j].isnan()))
    {
      avg=(endpoints[i]+endpoints[j])/2;
      ptit=endpoints.begin()+i;
      lenit=lengths.begin()+i;
      endpoints.erase(ptit);
      lengths.erase(lenit);
      lenit=cumLengths.begin()+i;
      cumLengths.erase(lenit);
      bcit=boundCircles.begin()+i;
      boundCircles.erase(bcit);
      if (h>i)
	h--;
      if (k>i)
	k--;
      if (j>i)
        j=i;
      else
        i=j; // deleted the last element, what [i] was is no longer valid
      if (avg.isfinite())
	endpoints[i]=avg;
      if (i!=h)
	lengths[h]=dist(endpoints[h],endpoints[i]);
      if (j!=k)
	lengths[j]=dist(endpoints[k],endpoints[j]);
      i--; // in case three in a row are the same
    }
  }
}

void polyline::insert(xy newpoint,int pos)
/* Inserts newpoint in position pos. E.g. insert(xy(8,5),2) does
 * {(0,0),(1,1),(2,2),(3,3)} -> {(0,0),(1,1),(8,5),(2,2),(3,3)}.
 * If the polyline is open, inserting a point in position 0, -1, or after the last
 * (-1 means after the last) results in adding a line segment.
 * If the polyline is empty (and therefore closed), inserting a point results in
 * adding a line segment from that point to itself.
 * In all other cases, newpoint is inserted between two points and connected to
 * them with line segments.
 */
{
  bool wasopen;
  int i;
  vector<xy>::iterator ptit;
  vector<double>::iterator lenit;
  vector<bcir>::iterator bcit;
  if (newpoint.isnan())
    cerr<<"Inserting NaN"<<endl;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  endpoints.insert(ptit,newpoint);
  lengths.insert(lenit,0);
  bcit=boundCircles.begin()+pos;
  boundCircles.insert(bcit,{xy(0,0),0});
  lenit=cumLengths.begin()+pos;
  if (pos<cumLengths.size())
    cumLengths.insert(lenit,cumLengths[pos]);
  else
    cumLengths.insert(lenit,0);
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
/* Same as polyline::insert for beginning, end, and empty cases.
 * In all other cases, newpoint is inserted into an arc, whose delta is split
 * proportionally to the distances to the adjacent points.
 */
{
  bool wasopen;
  double totdist=0,totdelta=0;
  int i,savepos,newdelta[2];
  vector<xy>::iterator ptit;
  vector<int>::iterator arcit;
  vector<double>::iterator lenit;
  vector<bcir>::iterator bcit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  arcit=deltas.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
  lengths.insert(lenit,0);
  bcit=boundCircles.begin()+pos;
  boundCircles.insert(bcit,{xy(0,0),0});
  lenit=cumLengths.begin()+pos;
  if (pos<cumLengths.size())
    cumLengths.insert(lenit,cumLengths[pos]);
  else
    cumLengths.insert(lenit,0);
  pos--;
  if (pos<0)
    if (wasopen)
      pos=0;
    else
      pos+=endpoints.size();
  savepos=pos;
  for (i=0;i<2;i++)
  {
    if (pos+1<endpoints.size())
    {
      lengths[pos]=dist(endpoints[pos],endpoints[pos+1]);
      totdist+=lengths[pos];
      totdelta+=deltas[pos];
    }
    if (pos+1==endpoints.size() && !wasopen)
    {
      lengths[pos]=dist(endpoints[pos],endpoints[0]);
      totdist+=lengths[pos];
      totdelta+=deltas[pos];
    }
    pos++;
    if (pos>=lengths.size())
      pos=0;
  }
  pos=savepos;
  if (totdist)
  {
    newdelta[0]=rint(lengths[pos]*totdelta/totdist);
    newdelta[1]=totdelta-newdelta[0];
  }
  else
  {
    newdelta[0]=totdelta;
    newdelta[1]=0;
  }
  for (i=0;i<2;i++)
  {
    if (pos+1<endpoints.size() || !wasopen)
    {
      deltas[pos]=newdelta[i];
      lengths[pos]=getarc(i).length();
    }
    pos++;
    if (pos>=lengths.size())
      pos=0;
  }
}

/* After inserting, opening, or closing, call setlengths before calling
 * length or station. If insert called setlengths, manipulation would take
 * too long. So do a lot of inserts, then call setlengths.
 */
void polyline::setlengths()
{
  int i;
  manysum m;
  segment seg;
  assert(lengths.size()==cumLengths.size());
  for (i=0;i<lengths.size();i++)
  {
    seg=getsegment(i);
    lengths[i]=seg.length();
    boundCircles[i]=seg.boundCircle();
    m+=lengths[i];
    cumLengths[i]=m.total();
  }
}

void polyarc::setlengths()
{
  int i;
  manysum m;
  arc seg;
  assert(lengths.size()==cumLengths.size());
  assert(lengths.size()==deltas.size());
  for (i=0;i<deltas.size();i++)
  {
    seg=getarc(i);
    lengths[i]=seg.length();
    boundCircles[i]=seg.boundCircle();
    m+=lengths[i];
    cumLengths[i]=m.total();
  }
}

void polyspiral::setlengths()
{
  int i;
  manysum m;
  spiralarc seg;
  assert(lengths.size()==cumLengths.size());
  assert(lengths.size()==deltas.size());
  for (i=0;i<deltas.size();i++)
  {
    seg=getspiralarc(i);
    lengths[i]=seg.length();
    boundCircles[i]=seg.boundCircle();
    m+=lengths[i];
    cumLengths[i]=m.total();
  }
}

void polyarc::setdelta(int i,int delta)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  deltas[i]=delta;
  lengths[i]=getarc(i).length();
}

double polyline::in(xy point)
/* Returns 1 if the polyline winds once counterclockwise around point.
 * Returns 1/2 or -1/2 if point is on polyline's boundary, unless it
 * is a corner, in which case it returns another fraction.
 */
{
  double ret=0,subtarea;
  int i,subtended,sz=endpoints.size();
  for (i=0;i<lengths.size();i++)
  {
    if (point!=endpoints[i] && point!=endpoints[(i+1)%sz])
    {
      subtended=foldangle(dir(point,endpoints[(i+1)%sz])-dir(point,endpoints[i]));
      if (subtended==-DEG180)
      {
        subtarea=area3(endpoints[(i+1)%sz],point,endpoints[i]);
        if (subtarea>0)
          subtended=DEG180;
        if (subtarea==0)
          subtended=0;
      }
      ret+=bintorot(subtended);
    }
  }
  return ret;
}

double polyarc::in(xy point)
{
  double ret=polyline::in(point);
  int i;
  for (i=0;i<lengths.size();i++)
    ret+=getarc(i).in(point);
  return ret;
}

double polyspiral::in(xy point)
{
  double ret=polyline::in(point);
  int i;
  for (i=0;i<lengths.size();i++)
    ret+=getspiralarc(i).in(point);
  return ret;
}

double polyline::length()
{
  if (cumLengths.size())
    return cumLengths.back();
  else
    return 0;
}

double polyline::getCumLength(int i)
// Returns the total of the first i segments/arcs/spiralarcs.
{
  i--;
  if (i>=(signed)cumLengths.size())
    i=cumLengths.size()-1;
  if (i<0)
    return 0;
  else
    return cumLengths[i];
}

int polyline::stationSegment(double along)
{
  int before=-1,after=cumLengths.size();
  int middle,i;
  double midalong;
  while (before<after-1)
  {
    middle=(before+after+(i&1))/2;
    if (middle>=cumLengths.size())
      midalong=length();
    else if (middle<0)
      midalong=0;
    else
      midalong=cumLengths[middle];
    if (midalong>along)
      after=middle;
    else
      before=middle;
    ++i;
  }
  if (after==cumLengths.size() && after && along==cumLengths.back())
    after--; // station(length()) should return the endpoint, not NaN
  return after;
}

xyz polyline::station(double along)
{
  int seg=stationSegment(along);
  if (seg<0 || seg>=lengths.size())
    return xyz(NAN,NAN,NAN);
  else
    return getsegment(seg).station(along-(cumLengths[seg]-lengths[seg]));
}

xyz polyarc::station(double along)
{
  int seg=stationSegment(along);
  if (seg<0 || seg>=lengths.size())
    return xyz(NAN,NAN,NAN);
  else
    return getarc(seg).station(along-(cumLengths[seg]-lengths[seg]));
}

xyz polyspiral::station(double along)
{
  int seg=stationSegment(along);
  if (seg<0 || seg>=lengths.size())
    return xyz(NAN,NAN,NAN);
  else
    return getspiralarc(seg).station(along-(cumLengths[seg]-lengths[seg]));
}

double polyline::closest(xy topoint,bool offends)
/* offends is currently ignored. It has to be true when calling segment::closest
 * because of angle points.
 */
{
  int i,n,step,sz;
  double segclose,closesofar=INFINITY;
  segment si;
  xy sta;
  double alo,ret;
  sz=lengths.size();
  step=relprime(sz);
  for (i=n=0;i<sz;i++,n=(n+step)%sz)
  {
    if (dist(boundCircles[n].center,topoint)-boundCircles[n].radius<closesofar)
    {
      si=getsegment(n);
      alo=si.closest(topoint,closesofar,true);
      sta=si.station(alo);
      segclose=dist(sta,topoint);
      if (segclose<closesofar)
      {
	closesofar=segclose;
	ret=alo+(cumLengths[n]-lengths[n]);
      }
    }
  }
  return ret;
}

double polyarc::closest(xy topoint,bool offends)
{
  int i,n,step,sz;
  double segclose,closesofar=INFINITY;
  arc si;
  xy sta;
  double alo,ret;
  sz=lengths.size();
  step=relprime(sz);
  for (i=n=0;i<sz;i++,n=(n+step)%sz)
  {
    if (dist(boundCircles[n].center,topoint)-boundCircles[n].radius<closesofar)
    {
      si=getarc(n);
      alo=si.closest(topoint,closesofar,true);
      sta=si.station(alo);
      segclose=dist(sta,topoint);
      if (segclose<closesofar)
      {
	closesofar=segclose;
	ret=alo+(cumLengths[n]-lengths[n]);
      }
    }
  }
  return ret;
}

double polyspiral::closest(xy topoint,bool offends)
{
  int i,n,step,sz;
  double segclose,closesofar=INFINITY;
  spiralarc si;
  xy sta;
  double alo,ret;
  sz=lengths.size();
  step=relprime(sz);
  for (i=n=0;i<sz;i++,n=(n+step)%sz)
  {
    if (dist(boundCircles[n].center,topoint)-boundCircles[n].radius<closesofar)
    {
      si=getspiralarc(n);
      alo=si.closest(topoint,closesofar,true);
      sta=si.station(alo);
      segclose=dist(sta,topoint);
      if (segclose<closesofar)
      {
	closesofar=segclose;
	ret=alo+(cumLengths[n]-lengths[n]);
      }
    }
  }
  return ret;
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

double polyspiral::area()
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
      a+=getspiralarc(i).diffarea();
    }
  return a.total();
}

double polyline::dirbound(int angle,double boundsofar)
{
  int i;
  double bound;
  for (i=0;i<lengths.size();i++)
  {
    bound=getsegment(i).dirbound(angle,boundsofar);
    if (bound<boundsofar)
      boundsofar=bound;
  }
  return boundsofar;
}

double polyarc::dirbound(int angle,double boundsofar)
{
  int i;
  double bound;
  for (i=0;i<lengths.size();i++)
  {
    bound=getarc(i).dirbound(angle,boundsofar);
    if (bound<boundsofar)
      boundsofar=bound;
  }
  return boundsofar;
}

double polyspiral::dirbound(int angle,double boundsofar)
{
  int i;
  double bound;
  for (i=0;i<lengths.size();i++)
  {
    bound=getspiralarc(i).dirbound(angle,boundsofar);
    if (bound<boundsofar)
      boundsofar=bound;
  }
  return boundsofar;
}

void polyline::open()
{
  lengths.resize(endpoints.size()-1);
  cumLengths.resize(endpoints.size()-1);
  boundCircles.resize(endpoints.size()-1);
}

void polyarc::open()
{
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
  cumLengths.resize(endpoints.size()-1);
  boundCircles.resize(endpoints.size()-1);
}

void polyspiral::open()
{
  curvatures.resize(endpoints.size()-1);
  clothances.resize(endpoints.size()-1);
  midpoints.resize(endpoints.size()-1);
  midbearings.resize(endpoints.size()-1);
  delta2s.resize(endpoints.size()-1);
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
  cumLengths.resize(endpoints.size()-1);
  boundCircles.resize(endpoints.size()-1);
}

void polyline::close()
{
  lengths.resize(endpoints.size());
  cumLengths.resize(endpoints.size());
  boundCircles.resize(endpoints.size());
  if (lengths.size())
    if (lengths.size()>1)
      cumLengths[lengths.size()-1]=cumLengths[lengths.size()-2]+lengths[lengths.size()-1];
    else
      cumLengths[0]=lengths[0];
}

void polyarc::close()
{
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
  cumLengths.resize(endpoints.size());
  boundCircles.resize(endpoints.size());
  if (lengths.size())
    if (lengths.size()>1)
      cumLengths[lengths.size()-1]=cumLengths[lengths.size()-2]+lengths[lengths.size()-1];
    else
      cumLengths[0]=lengths[0];
}

void polyspiral::close()
{
  curvatures.resize(endpoints.size());
  clothances.resize(endpoints.size());
  midpoints.resize(endpoints.size());
  midbearings.resize(endpoints.size());
  delta2s.resize(endpoints.size());
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
  cumLengths.resize(endpoints.size());
  boundCircles.resize(endpoints.size());
  if (lengths.size())
    if (lengths.size()>1)
      cumLengths[lengths.size()-1]=cumLengths[lengths.size()-2]+lengths[lengths.size()-1];
    else
      cumLengths[0]=lengths[0];
}

void polyspiral::insert(xy newpoint,int pos)
/* If there is one point after insertion and the polyspiral is closed:
 * Adds a line from the point to itself.
 * If there are two points after insertion and the polyspiral is open:
 * Adds a line from one point to the other.
 * If it's closed:
 * Adds two 180° arcs, making a circle.
 * If there are at least three points after insertion:
 * Updates the bearings of the new point and two adjacent points (unless
 * it is the first or last of an open polyspiral, in which case one) to
 * match an arc passing through three points, then updates the clothances
 * and curvatures of four consecutive spirals (unless the polyspiral is open
 * and the point is one of the first two or last two) to match the bearings.
 */
{
  bool wasopen;
  int i,savepos,newBearing=0;
  vector<xy>::iterator ptit,midit;
  vector<int>::iterator arcit,brgit,d2it,mbrit;
  vector<double>::iterator lenit,cloit,crvit;
  vector<bcir>::iterator bcit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  if (bearings.size())
    if (pos<=bearings.size()/2)
      newBearing=bearings[pos];
    else
      newBearing=bearings[pos-1];
  ptit=endpoints.begin()+pos;
  brgit=bearings.begin()+pos;
  savepos=pos;
  pos--;
  if (pos<0)
    if (wasopen || endpoints.size()==0)
      pos=0;
    else
      pos+=endpoints.size();
  midit=midpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  arcit=deltas.begin()+pos;
  d2it=delta2s.begin()+pos;
  mbrit=midbearings.begin()+pos;
  cloit=clothances.begin()+pos;
  crvit=curvatures.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
  lengths.insert(lenit,1);
  midpoints.insert(midit,newpoint);
  bearings.insert(brgit,newBearing);
  delta2s.insert(d2it,0);
  midbearings.insert(mbrit,0);
  curvatures.insert(crvit,0);
  clothances.insert(cloit,0);
  lenit=cumLengths.begin()+pos;
  cumLengths.insert(lenit,0);
  bcit=boundCircles.begin()+pos;
  boundCircles.insert(bcit,{xy(0,0),0});
  pos=savepos;
  for (i=-1;i<2;i++)
    setbear((pos+i+endpoints.size())%endpoints.size());
  for (i=-1;i<3;i++)
    setspiral((pos+i+lengths.size())%lengths.size());
}

void polyline::_roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
{
  int i;
  for (i=0;i<endpoints.size();i++)
    endpoints[i]._roscat(tfrom,ro,sca,cis,tto);
  for (i=0;i<lengths.size();i++)
    lengths[i]*=sca;
}

void polyspiral::_roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
{
  int i;
  for (i=0;i<endpoints.size();i++)
  {
    endpoints[i]._roscat(tfrom,ro,sca,cis,tto);
    bearings[i]+=ro;
  }
  for (i=0;i<lengths.size();i++)
  {
    lengths[i]*=sca;
    cumLengths[i]*=sca;
    midbearings[i]+=ro;
    curvatures[i]/=sca;
    clothances[i]/=sqr(sca);
  }
}

void polyspiral::setbear(int i)
{
  int h,j,prevbear,nextbear,avgbear;
  i%=endpoints.size();
  if (i<0)
    i+=endpoints.size();
  h=i-1;
  j=i+1;
  if (h<0)
    if (isopen())
      h+=3;
    else
      h+=endpoints.size();
  if (j>=endpoints.size())
    if (isopen())
      j-=3;
    else
      j-=endpoints.size();
  if (endpoints.size()==2)
    if (isopen())
      bearings[i]=dir(endpoints[0],endpoints[1]);
    else
      bearings[i]=dir(endpoints[i],endpoints[j])-DEG90;
  if (endpoints.size()>2)
  {
    bearings[i]=midarcdir(endpoints[h],endpoints[i],endpoints[j]);
    prevbear=bearings[h];
    nextbear=bearings[j];
    if (i==0)
      if (isopen())
	prevbear=nextbear;
      else
	prevbear+=DEG360;
    if (i==endpoints.size()-1)
      if (isopen())
	nextbear=prevbear;
      else
	nextbear+=DEG360;
    avgbear=prevbear+(nextbear-prevbear)/2;
    bearings[i]=avgbear+foldangle(bearings[i]-avgbear);
  }
}

void polyspiral::setbear(int i,int bear)
{
  i%=endpoints.size();
  if (i<0)
    i+=endpoints.size();
  bearings[i]=bear;
}

void polyspiral::setspiral(int i)
{
  int j,d1,d2;
  spiralarc s;
  j=i+1;
  if (j>=endpoints.size())
    j=0;
  s=spiralarc(xyz(endpoints[i],elevation),xyz(endpoints[j],elevation));
  d1=bearings[j]-bearings[i]+DEG360*(j<i);
  d2=bearings[j]+bearings[i]+DEG360*(j<i)-2*dir(endpoints[i],endpoints[j]);
  if (!curvy || abs(d1)>=bendlimit || abs(d2)>=bendlimit || abs(d1)+abs(d2)>=bendlimit)
    d1=d2=0;
  s.setdelta(d1,d2);
  if (std::isnan(s.length()))
    s.setdelta(0,0);
  if (lengths[i]==0)
    cerr<<"length["<<i<<"]=0"<<endl;
  deltas[i]=s.getdelta();
  delta2s[i]=s.getdelta2();
  lengths[i]=s.length();
  //if (std::isnan(lengths[i]))
    //cerr<<"length["<<i<<"]=nan"<<endl;
  midbearings[i]=s.bearing(lengths[i]/2);
  midpoints[i]=s.station(lengths[i]/2);
  curvatures[i]=s.curvature(lengths[i]/2);
  clothances[i]=s.clothance();
}

void polyspiral::smooth()
{
  int i;
  curvy=true;
  for (i=0;i<endpoints.size();i++)
    setbear(i);
  for (i=0;i<lengths.size();i++)
    setspiral(i);
}

void crspace(ofstream &ofile,int i)
{
  if (i%10)
    ofile<<' ';
  else if (i)
    ofile<<endl;
}

bool polyline::hasProperty(int prop)
{
  return prop==PROP_LENGTH ||
         (prop>=PROP_START_RADIUS && prop<=PROP_END_CURVATURE) ||
         (prop>=PROP_STARTPOINT && prop<=PROP_ENDPOINT);
}

void polyline::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<polyline elevation=\""<<ldecimal(elevation)<<"\"><endpoints>";
  for (i=0;i<endpoints.size();i++)
  {
    crspace(ofile,i);
    endpoints[i].writeXml(ofile);
  }
  ofile<<"</endpoints>\n<lengths>";
  for (i=0;i<lengths.size();i++)
  {
    crspace(ofile,i);
    ofile<<ldecimal(lengths[i]);
  }
  ofile<<"</lengths></polyline>"<<endl;
}

void polyarc::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<polyarc elevation=\""<<ldecimal(elevation)<<"\"><endpoints>";
  for (i=0;i<endpoints.size();i++)
  {
    crspace(ofile,i);
    endpoints[i].writeXml(ofile);
  }
  ofile<<"</endpoints>\n<lengths>";
  for (i=0;i<lengths.size();i++)
  {
    crspace(ofile,i);
    ofile<<ldecimal(lengths[i]);
  }
  ofile<<"</lengths>\n<deltas>";
  for (i=0;i<deltas.size();i++)
  {
    crspace(ofile,i);
    ofile<<deltas[i];
  }
  ofile<<"</deltas></polyarc>"<<endl;
}

void polyspiral::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<polyspiral elevation=\""<<ldecimal(elevation)<<"\"><endpoints>";
  for (i=0;i<endpoints.size();i++)
  {
    crspace(ofile,i);
    endpoints[i].writeXml(ofile);
  }
  ofile<<"</endpoints>\n<lengths>";
  for (i=0;i<lengths.size();i++)
  {
    crspace(ofile,i);
    ofile<<ldecimal(lengths[i]);
  }
  ofile<<"</lengths>\n<deltas>";
  for (i=0;i<deltas.size();i++)
  {
    crspace(ofile,i);
    ofile<<deltas[i];
  }
  ofile<<"</deltas>\n<delta2s>";
  for (i=0;i<delta2s.size();i++)
  {
    crspace(ofile,i);
    ofile<<delta2s[i];
  }
  ofile<<"</delta2s></polyspiral>"<<endl;
}

void alignment::setVLength()
/* Sets the horizontal length of vertical curves equal to the length of
 * horizontal curves. If any vertical curves are beyond the total horizontal
 * length, they are clobbered.
 */
{
  int i;
  int origsize=vCumLengths.size();
  int newlast;
  if (hCumLengths.size() && !vCumLengths.size()) // should never happen
    vCumLengths.push_back(hCumLengths[0]);
  if (hCumLengths.size()>1 && vCumLengths.size()==1)
    vCumLengths.push_back(hCumLengths.back());
  for (i=0;i<vCumLengths.size() && vCumLengths[i]<hCumLengths.back();i++);
  newlast=i;
  if (newlast>=origsize)
    vCumLengths.push_back(hCumLengths.back());
  vLengths.resize(newlast);
  for (i=origsize;i<newlast;i++)
    vLengths[i]=vCumLengths[i+1]-vCumLengths[i];
  controlPoints.resize(3*newlast+1);
}

void alignment::setlengths()
/* Sets cumLengths, which contains all elements of both vCumLengths and hCumLengths,
 * and sets lengths to the differences between them.
 */
{
  int i,j;
  cumLengths.clear();
  lengths.clear();
  for (i=j=0;i<hCumLengths.size() && j<vCumLengths.size();)
    if (hCumLengths[i]<vCumLengths[j])
      cumLengths.push_back(hCumLengths[i++]);
    else if (hCumLengths[i]==vCumLengths[j])
    {
      cumLengths.push_back(hCumLengths[i++]);
      j++;
    }
    else
      cumLengths.push_back(vCumLengths[j++]);
  for (i=0;i<cumLengths.size()-1;i++)
    lengths.push_back(cumLengths[i+1]-cumLengths[i]);
}

alignment::alignment()
{
  hCumLengths.push_back(0);
  vCumLengths.push_back(0);
}

void alignment::clear()
{
  endpoints.clear();
  deltas.clear();
  delta2s.clear();
  bearings.clear();
  midbearings.clear();
  midpoints.clear();
  clothances.clear();
  curvatures.clear();
  hLengths.clear();
  hCumLengths.clear();
  vLengths.clear();
  vCumLengths.clear();
  lengths.clear();
  cumLengths.clear();
  controlPoints.clear();
  boundCircles.clear();
  hCumLengths.push_back(0);
  vCumLengths.push_back(0);
  cumLengths.push_back(0);
}

double alignment::startStation()
{
  return cumLengths.size()?cumLengths[0]:NAN;
}

double alignment::endStation()
{
  return cumLengths.size()?cumLengths.back():NAN;
}

double alignment::length()
{
  return cumLengths.size()?cumLengths.back()-cumLengths[0]:0;
}

unsigned alignment::hash()
{
  return memHash(&cumLengths[0],cumLengths.size()*sizeof(double),
         memHash(&controlPoints[0],controlPoints.size()*sizeof(double),
         memHash(&curvatures[0],curvatures.size()*sizeof(double),
         memHash(&clothances[0],clothances.size()*sizeof(double),
         memHash(&midpoints[0],midpoints.size()*sizeof(xy),
         memHash(&midbearings[0],midbearings.size()*sizeof(int),
         memHash(&delta2s[0],delta2s.size()*sizeof(int),
         memHash(&deltas[0],deltas.size()*sizeof(int),
         memHash(&endpoints[0],endpoints.size()*sizeof(xy))))))))));
}

void alignment::appendPoint(xy pnt)
{
  xy last;
  bcir boundCircle;
  if (endpoints.size())
    last=endpoints.back();
  endpoints.push_back(pnt);
  bearings.push_back(0);
  if (endpoints.size()>1)
  {
    deltas.push_back(0);
    delta2s.push_back(0);
    midbearings.push_back(dir(last,pnt));
    midpoints.push_back((last+pnt)/2);
    hLengths.push_back(dist(last,pnt));
    hCumLengths.push_back(hCumLengths.back()+hLengths.back());
    boundCircle.center=midpoints.back();
    boundCircle.radius=hLengths.back()/2;
    boundCircles.push_back(boundCircle);
  }
  setVLength();
  setlengths();
}

void alignment::prependPoint(xy pnt)
{
  xy first;
  bcir boundCircle;
  if (endpoints.size())
    first=endpoints[0];
  endpoints.insert(endpoints.begin(),pnt);
  bearings.insert(bearings.begin(),0);
  if (endpoints.size()>1)
  {
    deltas.insert(deltas.begin(),0);
    delta2s.insert(delta2s.begin(),0);
    midbearings.insert(midbearings.begin(),dir(pnt,first));
    midpoints.insert(midpoints.begin(),(pnt+first)/2);
    hLengths.insert(hLengths.begin(),dist(pnt,first));
    hCumLengths.insert(hCumLengths.begin(),hCumLengths[0]-hLengths[0]);
    boundCircle.center=midpoints[0];
    boundCircle.radius=hLengths[0]/2;
    boundCircles.insert(boundCircles.begin(),boundCircle);
  }
  setVLength();
  setlengths();
}

void alignment::appendTangentCurve(double startCurvature,double length,double endCurvature)
{
  double along=endStation();
  xyz startPoint=station(along);
  int startBearing=bearing(along);
  spiralarc newSpiral(startPoint,startBearing,startCurvature,endCurvature,length,0);
  bcir boundCircle;
  appendPoint(newSpiral.getend());
  deltas.back()=newSpiral.getdelta();
  delta2s.back()=newSpiral.getdelta2();
  midbearings.back()=newSpiral.bearing(length/2);
  midpoints.back()=newSpiral.station(length/2);
  hLengths.back()=length;
  hCumLengths.back()+=length-newSpiral.chordlength();
  boundCircle.center=midpoints.back();
  boundCircle.radius=length/2;
  boundCircles.back()=boundCircle;
  setVLength();
  setlengths();
}

void alignment::prependTangentCurve(double startCurvature,double length,double endCurvature)
{
  double along=startStation();
  xyz endPoint=station(along);
  int endBearing=bearing(along);
  spiralarc newSpiral(endPoint,endBearing+DEG180,-endCurvature,-startCurvature,length,0);
  bcir boundCircle;
  prependPoint(newSpiral.getend());
  deltas[0]=-newSpiral.getdelta();
  delta2s[0]=newSpiral.getdelta2();
  midbearings[0]=newSpiral.bearing(length/2)-DEG180;
  midpoints[0]=newSpiral.station(length/2);
  hLengths[0]=length;
  hCumLengths[0]-=length-newSpiral.chordlength();
  boundCircle.center=midpoints[0];
  boundCircle.radius=length/2;
  boundCircles[0]=boundCircle;
  setVLength();
  setlengths();
}

spiralarc alignment::getHorizontalCurve(int i)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  return spiralarc(xyz(endpoints[i],0),xyz(midpoints[i],0),
		   xyz(endpoints[(i+1)%endpoints.size()],0),midbearings[i],
		   curvatures[i],clothances[i],hLengths[i]);
}

segment alignment::getVerticalCurve(int i)
{
  i%=vLengths.size();
  if (i<0)
    i+=vLengths.size();
  return segment(xyz(vCumLengths[i],0,controlPoints[i*3]),
		 controlPoints[i*3+1],controlPoints[i*3+2],
		 xyz(vCumLengths[i+1],0,controlPoints[i*3+3]));
}

spiralarc alignment::getCurve(int i)
{
  xyz startpoint,midpoint,endpoint;
  int bea;
  double slo,cur,acc,clo,jer;
  double midalong,len;
  spiralarc ret;
  i%=lengths.size();
  if (i<0)
    i+=lengths.size();
  midalong=(cumLengths[i+1]+cumLengths[i])/2;
  len=cumLengths[i+1]-cumLengths[i];
  /* An alignment is continuous, and usually smooth, but it may be only
   * piecewise smooth. A spiralarc has its bearing, curvature, and clothance
   * set at its middle, but its slope, acceleration, and jerk are set with
   * control points at 1/3 and 2/3 of its length. Since the slope at the ends
   * may not be equal to the slope just past the ends, we must compute them
   * from the derivatives at the middle.
   */
  bea=bearing(midalong);
  slo=slope(midalong);
  cur=curvature(midalong);
  acc=accel(midalong);
  clo=clothance(midalong);
  jer=jerk(midalong);
  startpoint=station(cumLengths[i]);
  midpoint=station(midalong);
  endpoint=station(cumLengths[i+1]);
  ret=spiralarc(startpoint,midpoint,endpoint,bea,cur,clo,len);
  ret.setslope(START,slo-acc*len/2+jer*sqr(len)/8);
  ret.setslope(END,slo+acc*len/2+jer*sqr(len)/8);
  return ret;
}

void alignment::setStartStation(double along)
{
  int i;
  for (i=1;i<hCumLengths.size();i++)
    hCumLengths[i]+=along-hCumLengths[0];
  for (i=1;i<vCumLengths.size();i++)
    vCumLengths[i]+=along-vCumLengths[0];
  for (i=1;i<cumLengths.size();i++)
    cumLengths[i]+=along-cumLengths[0];
  hCumLengths[0]=vCumLengths[0]=cumLengths[0]=along;
}

void alignment::setHLengths()
// Unlike polyline/arc/spiral, hCumLengths has one more element than hLengths.
{
  int i;
  manysum m;
  spiralarc seg;
  assert(hLengths.size()==hCumLengths.size()-1);
  assert(hLengths.size()==deltas.size());
  m+=hCumLengths[0];
  for (i=0;i<deltas.size();i++)
  {
    seg=getHorizontalCurve(i);
    hLengths[i]=seg.length();
    boundCircles[i]=seg.boundCircle();
    m+=hLengths[i];
    hCumLengths[i+1]=m.total();
  }
  m.clear();
  m+=vCumLengths[0];
  for (i=0;i<vLengths.size();i++)
  {
    m+=vLengths[i];
    vCumLengths[i+1]=m.total();
  }
  setVLength();
  setlengths();
}

int alignment::xyStationSegment(double along)
{
  int before=-1,after=hCumLengths.size();
  int middle,i;
  double midalong;
  while (before<after-1)
  {
    middle=(before+after+(i&1))/2;
    if (middle>=hCumLengths.size())
      midalong=endStation();
    else if (middle<0)
      midalong=startStation();
    else
      midalong=hCumLengths[middle];
    if (midalong>along)
      after=middle;
    else
      before=middle;
    ++i;
  }
  return before; // Unlike polylines, hCumLengths and vCumLengths have an extra number at the beginning.
}

int alignment::zStationSegment(double along)
{
  int before=-1,after=vCumLengths.size();
  int middle,i;
  double midalong;
  while (before<after-1)
  {
    middle=(before+after+(i&1))/2;
    if (middle>=vCumLengths.size())
      midalong=endStation();
    else if (middle<0)
      midalong=startStation();
    else
      midalong=vCumLengths[middle];
    if (midalong>along)
      after=middle;
    else
      before=middle;
    ++i;
  }
  return before;
}

xy alignment::xyStation(double along)
{
  int seg=xyStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return xy(NAN,NAN);
  else
    return getHorizontalCurve(seg).station(along-cumLengths[seg]);
}

double alignment::zStation(double along)
{
  int seg=zStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getVerticalCurve(seg).station(along-cumLengths[seg]).elev();
}

xyz alignment::station(double along)
{
  return xyz(xyStation(along),zStation(along));
}

int alignment::bearing(double along)
{
  int seg=xyStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return 0;
  else
    return getHorizontalCurve(seg).bearing(along-cumLengths[seg]);
}

double alignment::slope(double along)
{
  int seg=zStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getVerticalCurve(seg).slope(along-cumLengths[seg]);
}

double alignment::curvature(double along)
{
  int seg=xyStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getHorizontalCurve(seg).curvature(along-cumLengths[seg]);
}

double alignment::accel(double along)
{
  int seg=zStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getVerticalCurve(seg).accel(along-cumLengths[seg]);
}

double alignment::clothance(double along)
{
  int seg=xyStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getHorizontalCurve(seg).clothance();
}

double alignment::jerk(double along)
{
  int seg=zStationSegment(along);
  if (seg<0 || seg>=hLengths.size())
    return NAN;
  else
    return getVerticalCurve(seg).jerk();
}
