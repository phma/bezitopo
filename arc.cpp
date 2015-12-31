/******************************************************/
/*                                                    */
/* arc.cpp - horizontal circular arcs                 */
/*                                                    */
/******************************************************/

#include <cstdio>
#include "point.h"
#include "arc.h"
#include "vcurve.h"

arc::arc()
{
  start=end=xyz(0,0,0);
  rchordbearing=control1=control2=delta=0;
}

arc::arc(xyz kra,xyz fam)
{
  start=kra;
  end=fam;
  delta=0;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
  rchordbearing=atan2(end.north()-start.north(),end.east()-start.east());
}

arc::arc(xyz kra,xyz mij,xyz fam)
{
  double p,q,r;
  start=kra;
  end=fam;
  delta=2*(atan2i(fam-mij)-atan2i(mij-kra));
  if (delta)
    p=(double)(2*(atan2i(fam-mij)-atan2i(fam-kra)))/delta;
  else
    p=dist(xy(kra),xy(mij))/dist(xy(kra),xy(fam));
  q=1-p;
  r=(mij.elev()-start.elev()-p*(end.elev()-start.elev()))/(p*q)/3;
  control1=(2*start.elev()+end.elev())/3+r;
  control2=(start.elev()+2*end.elev())/3+r;
  rchordbearing=atan2(end.north()-start.north(),end.east()-start.east());
}

arc::arc(xyz kra,xyz fam,int d)
{
  start=kra;
  end=fam;
  delta=d;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
  rchordbearing=atan2(end.north()-start.north(),end.east()-start.east());
}

void arc::setdelta(int d,int s) // s is for spirals and is ignored for circular arcs
{
  delta=d;
}

xy arc::center()
{
  return ((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))/2/tanhalf(delta)));
}

double arc::length() const
{
  if (delta)
    return chordlength()*bintorad(delta)/sinhalf(delta)/2;
  else
    return chordlength();
}

double arc::diffarea()
{
  double r;
  if (delta)
  {
    r=radius(0);
    return r*r*(bintorad(delta)-sin(delta))/2;
    //FIXME fix numerical stability for small delta
  }
  else
    return 0;
}

xyz arc::station(double along) const
{
  double gnola,len,angalong,rdelta;
  len=length();
  if (delta)
  {
    rdelta=bintorad(delta);
    angalong=along/len*bintorad(delta);
    gnola=len-along;
    //printf("arc::station angalong=%f startbearing=%f\n",bintodeg(angalong),bintodeg(startbearing()));
    return xyz(xy(start)+cossin((angalong-rdelta)/2+rchordbearing)*sin(angalong/2)*radius(0)*2,
	      elev(along));
  }
  else
    return segment::station(along);
}

int arc::bearing(double along)
{
  double len;
  int angalong;
  angalong=lrint((along/len-0.5)*delta);
  return chordbearing()+angalong;
}

void arc::split(double along,arc &a,arc &b)
{
  double dummy;
  int deltaa,deltab;
  xyz splitpoint=station(along);
  deltaa=lrint(delta*along/length());
  deltab=delta-deltaa;
  a=arc(start,splitpoint,deltaa);
  b=arc(splitpoint,end,deltab);
  vsplit(start.elev(),control1,control2,end.elev(),along/length(),a.control1,a.control2,dummy,b.control1,b.control2);
  //printf("split: %f,%f\n",a.end.east(),a.end.north());
}

int arc::in(xy pnt)
{
  int beardiff;
  beardiff=foldangle(2*(dir(pnt,end)-dir(start,pnt)));
  if (delta && (abs(beardiff-delta)<2 || beardiff==0))
    return (delta>0)?1:-1; // call spiralarc::in once that's defined
  else if (delta && (pnt==start || pnt==end))
    return IN_AT_CORNER;
  else
    return 2*((beardiff>0)-(beardiff>delta));
}

/*xyz arc::midpoint()
{
  return xyz((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))*2*tanquarter(delta)),
             (start.elev()+end.elev()+3*(control1+control2))/8);
}*/

/* To find the nearest point on the arc to a point:
   If delta is less than 0x1000000 (2°48'45") in absolute value, use linear
   interpolation to find a starting point. If it's between 0x1000000 and
   0x40000000 (180°), use the bearing from the center. Between 0x40000000
   and 0x7f000000 (357°11'15"), use the bearing from the center, but use
   calong() instead of along(). From 0x7f000000 to 0x80000000, use linear
   interpolation and calong(). Then use parabolic interpolation to find
   the closest point on the circle.
   */
