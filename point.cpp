/******************************************************/
/*                                                    */
/* point.cpp - classes for points and gradients       */
/*                                                    */
/******************************************************/

#include "point.h"
#include <cstdlib>
#include <cmath>
#include <string.h>
#include "tin.h"
#include "pointlist.h"

xy::xy(double e,double n)
{x=e;
 y=n;
 }

xy::xy()
{x=0;
 y=0;
 }

xy::xy(xyz point)
{x=point.x;
 y=point.y;
 }

double xy::east()
{return x;
 }

double xy::north()
{return y;
 }

xy operator+(const xy &l,const xy &r)
{xy sum(l.x+r.x,l.y+r.y);
 return sum;
 }

xy operator+=(xy &l,const xy &r)
{l.x+=r.x;
 l.y+=r.y;
 return l;
 }

xy operator*(const xy &l,double r)
{xy prod(l.x*r,l.y*r);
 return prod;
 }

xy operator-(const xy &l,const xy &r)
{xy sum(l.x-r.x,l.y-r.y);
 return sum;
 }

xy operator/(const xy &l,double r)
{xy prod(l.x/r,l.y/r);
 return prod;
 }

xy operator/=(xy &l,double r)
{l.x/=r;
 l.y/=r;
 return l;
 }

bool operator!=(const xy &l,const xy &r)
{
  return l.x!=r.x || l.y!=r.y;
}

bool operator==(const xy &l,const xy &r)
{
  return l.x==r.x && l.y==r.y;
}

xy turn90(xy a)
{return xy(-a.y,a.x);
 }

double dist(xy a,xy b)
{return hypot(a.x-b.x,a.y-b.y);
 }

double dir(xy a,xy b)
{return atan2(a.y-b.y,a.x-b.x);
 }

double dot(xy a,xy b)
{return (a.y*b.y+a.x*b.x);
 }

xyz::xyz(double e,double n,double h)
{x=e;
 y=n;
 z=h;
 }

double xyz::east()
{return x;
 }

double xyz::north()
{return y;
 }

double xyz::elev()
{return z;
 }

xyz::xyz()
{x=y=z=0;
 }

bool operator==(const xyz &l,const xyz &r)
{
  return l.x==r.x && l.y==r.y && l.z==r.z;
}

point::point()
{x=y=z=0;
 line=NULL;
 flags=0;
 note="";
 }

point::point(double e,double n,double h,string desc)
{x=e;
 y=n;
 z=h;
 line=0;
 note=desc;
 }

point::point(xy pnt,double h,string desc)
{x=pnt.x;
 y=pnt.y;
 z=h;
 line=0;
 note=desc;
 }

point::point(const point &rhs)
{x=rhs.x;
 y=rhs.y;
 z=rhs.z;
 line=rhs.line;
 note=rhs.note;
 }

const point& point::operator=(const point &rhs)
{if (this!=&rhs)
    {x=rhs.x;
     y=rhs.y;
     z=rhs.z;
     flags=rhs.flags;
     line=rhs.line;
     note=rhs.note;
     }
 }

void point::dump()
{printf("address=%p\nnum=%d\n(%f,%f,%f)\nline=%p\n",this,topopoints.revpoints[this],x,y,z,line);
 }

/*void point::setedge(point *oend)
{int i;
 edge *oldline;
 for (i=0,oldline=line;(!i || oldline==line) && line->next(this)->otherend(this)!=oend;i++)
     line=line->next(this);
 }*/

