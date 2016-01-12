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
#include "ldecimal.h"
#include "angle.h"
#include "document.h"
using namespace std;

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

double xy::east() const
{
  return x;
}

double xy::north() const
{
  return y;
}

double xy::getx() const
{
  return x;
}

double xy::gety() const
{
  return y;
}

double xy::length() const
{
  return hypot(x,y);
}

bool xy::isfinite() const
{
  return std::isfinite(x) && std::isfinite(y);
}

bool xy::isnan() const
{
  return std::isnan(x) || std::isnan(y);
}

void xy::_roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
{
  double tx,ty;
  x-=tfrom.x;
  y-=tfrom.y;
  tx=x*cis.x-y*cis.y;
  ty=y*cis.x+x*cis.y;
  x=tx+tto.x;
  y=ty+tto.y;
}

void xy::roscat(xy tfrom,int ro,double sca,xy tto)
{
  _roscat(tfrom,ro,sca,cossin(ro)*sca,tto);
}

void xy::writeXml(ofstream &ofile)
{
  ofile<<"<xy>"<<ldecimal(x)<<' '<<ldecimal(y)<<"</xy>";
}

xy operator+(const xy &l,const xy &r)
{xy sum(l.x+r.x,l.y+r.y);
 return sum;
 }

xy operator+=(xy &l,const xy &r)
{
  l.x+=r.x;
  l.y+=r.y;
  return l;
}

xy operator*(double l,const xy &r)
{
  xy prod(l*r.x,l*r.y);
  return prod;
}

xy operator*(const xy &l,double r)
{
  xy prod(l.x*r,l.y*r);
  return prod;
}

xy operator-(const xy &l,const xy &r)
{
  xy sum(l.x-r.x,l.y-r.y);
  return sum;
}

xy operator-(const xy &r)
{
  xy sum(-r.x,-r.y);
  return sum;
}

xy operator/(const xy &l,double r)
{xy prod(l.x/r,l.y/r);
 return prod;
 }

xy operator/=(xy &l,double r)
{
  l.x/=r;
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
{
  return xy(-a.y,a.x);
}

xy turn(xy a,int angle)
{
  double s,c;
  s=sin(angle);
  c=cos(angle);
  return xy(c*a.x-s*a.y,s*a.x+c*a.y);
}

double dist(xy a,xy b)
{return hypot(a.x-b.x,a.y-b.y);
 }

int dir(xy a,xy b)
{return atan2i(b-a);
 }

double dot(xy a,xy b)
{return (a.y*b.y+a.x*b.x);
 }

const xy beforestart(-INFINITY,-INFINITY);
const xy afterend(INFINITY,INFINITY);

xyz::xyz(double e,double n,double h)
{x=e;
 y=n;
 z=h;
 }

xyz::xyz(xy en,double h)
{x=en.x;
 y=en.y;
 z=h;
 }

double xyz::east() const
{
  return x;
}

double xyz::north() const
{
  return y;
}

double xyz::elev() const
{
  return z;
}

double xyz::getx() const
{
  return x;
}

double xyz::gety() const
{
  return y;
}

bool xyz::isfinite() const
{
  return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}

bool xyz::isnan() const
{
  return std::isnan(x) || std::isnan(y) || std::isnan(z);
}

double xyz::getz() const
{
  return z;
}

double xyz::lat()
{
  return atan2(z,hypot(x,y));
}

double xyz::lon()
{
  return atan2(y,x);
}

int xyz::lati()
{
  return atan2i(z,hypot(x,y));
}

int xyz::loni()
{
  return atan2i(y,x);
}

double xyz::length()
{
  return sqrt(x*x+y*y+z*z);
}

xyz::xyz()
{x=y=z=0;
 }

void xyz::_roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
{
  double tx,ty;
  x-=tfrom.x;
  y-=tfrom.y;
  tx=x*cis.x-y*cis.y;
  ty=y*cis.x+x*cis.y;
  x=tx+tto.x;
  y=ty+tto.y;
}

void xyz::roscat(xy tfrom,int ro,double sca,xy tto)
{
  _roscat(tfrom,ro,sca,cossin(ro)*sca,tto);
}

void xyz::writeXml(ofstream &ofile)
{
  ofile<<"<xyz>"<<ldecimal(x)<<' '<<ldecimal(y)<<' '<<ldecimal(z)<<"</xyz>";
}

bool operator==(const xyz &l,const xyz &r)
{
  return l.x==r.x && l.y==r.y && l.z==r.z;
}

xyz operator/(const xyz &l,const double r)
{
  return xyz(l.x/r,l.y/r,l.z/r);
}

xyz operator*=(xyz &l,double r)
{
  l.x*=r;
  l.y*=r;
  l.z*=r;
  return l;
}

xyz operator/=(xyz &l,double r)
{
  l.x/=r;
  l.y/=r;
  l.z/=r;
  return l;
}

xyz operator+=(xyz &l,const xyz &r)
{
  l.x+=r.x;
  l.y+=r.y;
  l.z+=r.z;
  return l;
}

xyz operator-=(xyz &l,const xyz &r)
{
  l.x-=r.x;
  l.y-=r.y;
  l.z-=r.z;
  return l;
}

xyz operator*(const xyz &l,const double r)
{
  return xyz(l.x*r,l.y*r,l.z*r);
}

xyz operator*(const double l,const xyz &r)
{
  return xyz(l*r.x,l*r.y,l*r.z);
}

xyz operator*(const xyz &l,const xyz &r) // cross product
{
  return xyz(l.y*r.z-l.z*r.y,l.z*r.x-l.x*r.z,l.x*r.y-l.y*r.x);
}

xyz operator+(const xyz &l,const xyz &r)
{
  return xyz(l.x+r.x,l.y+r.y,l.z+r.z);
}

xyz operator-(const xyz &l,const xyz &r)
{
  return xyz(l.x-r.x,l.y-r.y,l.z-r.z);
}

double dist(xyz a,xyz b)
{return hypot(hypot(a.x-b.x,a.y-b.y),a.z-b.z);
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

//void point::dump(document doc)
//{
//  printf("address=%p\nnum=%d\n(%f,%f,%f)\nline=%p\n",this,doc.pl[1].revpoints[this],x,y,z,line);
//}

void point::writeXml(ofstream &ofile,pointlist &pl)
{
  ofile<<"<point n=\""<<pl.revpoints[this]<<"\" d=\""<<xmlEscape(note)<<"\">"<<ldecimal(x)<<' '<<ldecimal(y)<<' '<<ldecimal(z)<<"</point>";
}

int point::valence()
{
  int i;
  edge *oldline;
  for (i=0,oldline=line;(!i || oldline!=line);i++)
    line=line->next(this);
  return i;
}

edge *point::edg(triangle *tri)
{
  int i;
  edge *oldline,*ret;
  for (i=0,oldline=line,ret=NULL;!ret && (!i || oldline!=line);i++)
  {
    if (line->tri(this)==tri)
      ret=line;
    line=line->next(this);
  }
  return ret;
}

/*void point::setedge(point *oend)
{int i;
 edge *oldline;
 for (i=0,oldline=line;(!i || oldline==line) && line->next(this)->otherend(this)!=oend;i++)
     line=line->next(this);
 }*/

