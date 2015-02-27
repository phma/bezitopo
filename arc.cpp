/******************************************************/
/*                                                    */
/* arc.cpp - horizontal circular arcs                 */
/*                                                    */
/******************************************************/

#include <cstdio>
#include "arc.h"
#include "vcurve.h"

arc::arc()
{
  start=end=xyz(0,0,0);
  control1=control2=delta=0;
}

arc::arc(xyz kra,xyz fam)
{
  start=kra;
  end=fam;
  delta=0;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
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
}

arc::arc(xyz kra,xyz fam,int d)
{
  start=kra;
  end=fam;
  delta=d;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
}

void arc::setdelta(int d,int s) // s is for spirals and is ignored for circular arcs
{
  delta=d;
}

xy arc::center()
{
  return ((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))/2/tanhalf(delta)));
}

double arc::length()
{
  if (delta)
    return chordlength()*bintorad(delta)/sinhalf(delta)/2;
  else
    return chordlength();
}

xyz arc::station(double along)
{
  double gnola,len;
  int angalong;
  len=length();
  if (delta) // FIXME if delta is less than 1 second or so, this isn't accurate
  {
    angalong=lrint(along/len*delta);
    gnola=len-along;
    //printf("arc::station angalong=%f startbearing=%f\n",bintodeg(angalong),bintodeg(startbearing()));
    return xyz(xy(start)+cossinhalf(angalong+2*startbearing())*sinhalf(angalong)*radius(0)*2,
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

/*xy arc::midpoint()
{return ((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))*2*tanquarter(delta)));
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
