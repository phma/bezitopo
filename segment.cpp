/******************************************************/
/*                                                    */
/* segment.cpp - 3d line segment                      */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/

#include <cmath>
#include <typeinfo>
#include <iostream>
#include "segment.h"
#include "arc.h"
#include "spiral.h"
#include "vcurve.h"

using namespace std;

segment::segment()
{
  start=end=xyz(0,0,0);
  control1=control2=0;
}

segment::segment(xyz kra,xyz fam)
{
  start=kra;
  end=fam;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
  /* This can result in slight parabolicity.
   * start=end=        2.916666666666667
   * control1=control2=2.9166666666666665
   * setslope(which,0) will fix it.
   */
}

segment::segment(xyz kra,double c1,double c2,xyz fam)
{
  start=kra;
  end=fam;
  control1=c1;
  control2=c2;
}

double segment::length()
{
  return dist(xy(start),xy(end));
}

void segment::setslope(int which,double s)
{
  switch(which)
  {
    case START:
      control1=start.elev()+s*length()/3;
      break;
    case END:
      control2=end.elev()-s*length()/3;
      break;
  }
}

void segment::setctrl(int which,double el)
{
  switch(which)
  {
    case START:
      control1=el;
      break;
    case END:
      control2=el;
      break;
  }
}

double segment::elev(double along)
{
  return vcurve(start.elev(),control1,control2,end.elev(),along/length());
}

double segment::slope(double along)
{
  return vslope(start.elev(),control1,control2,end.elev(),along/length())/length();
}

double segment::startslope()
{
  return (control1-start.elev())*3/length();
}

double segment::endslope()
{
  return (end.elev()-control2)*3/length();
}

xyz segment::station(double along)
{
  double gnola,len;
  len=length();
  gnola=len-along;
  return xyz((start.east()*gnola+end.east()*along)/len,(start.north()*gnola+end.north()*along)/len,
	     elev(along));
}

vector<double> segment::vextrema(bool withends)
{
  double len;
  int i;
  vector<double> ret;
  ret=::vextrema(start.elev(),control1,control2,end.elev());
  for (i=ret.size()-1;i>=0 && !withends;i--)
    if (ret[i]==0 || ret[i]==1 || ::isnan(ret[i]))
      ret.erase(ret.begin()+i);
  if (ret.size())
    len=length();
  for (i=0;i<ret.size();i++)
    ret[i]*=len;
  return ret;
}

xyz segment::midpoint()
{
  return station(length()/2);
}

xy segment::center()
{
  return xy(nan(""),nan(""));
}

void segment::split(double along,segment &a,segment &b)
{
  double dummy;
  xyz splitpoint=station(along);
  a=segment(start,splitpoint);
  b=segment(splitpoint,end);
  vsplit(start.elev(),control1,control2,end.elev(),along/length(),a.control1,a.control2,dummy,b.control1,b.control2);
}

bezier3d segment::approx3d(double precision)
/* Returns a chain of bezier3d splines which approximate the segment within precision.
 * Of course, for a segment, only one spline is needed and it is exact,
 * but for arcs and spiralarcs, more may be needed. Since startbearing, endbearing,
 * and length are virtual, this doesn't need to be overridden in the derived classes,
 * but it needs to construct two arcs or spiralarcs if it needs to split them.
 */
{
  segment *a,*b;
  int sb,eb,cb;
  double est;
  bezier3d ret;
  sb=startbearing();
  eb=endbearing();
  cb=chordbearing();
  if (abs(foldangle(sb-cb))<DEG30 && abs(foldangle(eb-cb))<DEG30 && abs(foldangle(sb+eb-2*cb))<DEG30)
    est=bez3destimate(start,sb,length(),eb,end);
  else
    est=fabs(precision*2)+1;
  //cout<<"sb "<<bintodeg(sb)<<" eb "<<bintodeg(eb)<<" est "<<est<<endl;
  if (est<=precision)
    ret=bezier3d(start,sb,startslope(),endslope(),eb,end);
  else
  {
    if (typeid(*this)==typeid(spiralarc))
    {
      a=new spiralarc;
      b=new spiralarc;
      ((spiralarc *)this)->split(length()/2,*(spiralarc *)a,*(spiralarc *)b);
    }
    else
    {
      a=new arc;
      b=new arc;
      ((arc *)this)->split(length()/2,*(arc *)a,*(arc *)b);
    }
    //cout<<"{"<<endl;
    ret=a->approx3d(precision)+b->approx3d(precision);
    //cout<<"}"<<endl;
    delete a;
    delete b;
  }
  return ret;
}

xy intersection (segment seg1,segment seg2)
/* This might should return a vector of xyz,
 * and will need versions for arc and maybe spiralarc.
 */
{
  return intersection(seg1.start,seg1.end,seg2.start,seg2.end);
}

inttype intersection_type(segment seg1,segment seg2)
{
  return intersection_type(seg1.start,seg1.end,seg2.start,seg2.end);
}
