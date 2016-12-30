/******************************************************/
/*                                                    */
/* geoid.cpp - geoidal undulation                     */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#include <climits>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <map>
#include "geoid.h"
#include "binio.h"
#include "angle.h"
#include "ldecimal.h"
using namespace std;

/* face=0: point is the center of the earth
 * face=1: in the Benin face; x=+y, y=+z
 * face=2: in the Bengal face; x=+z, y=+x
 * face=3: in the Arctic face; x=+x, y=+y
 * face=4: in the Antarctic face; x=+x, y=-y
 * face=5: in the Galápagos face; x=+z, y=-x
 * face=6: in the Howland face; x=+y, y=-z
 * face=7: a coordinate is NaN or at least two are infinite
 */
/* Format of Bezitopo's geoid files:
 * Start Len
 * 0000 0008 literal string "boldatni"
 * 0008 0008 hash identifier of this geoid file
 * 0010 0008 hash identifier of the geoid file this is an excerpt of, if any
 * 0018 0002 0000 file refers to the earth (other planets/moons have different
 *           sizes, so the limit of subdivision and smallest island are
 *           relatively different)
 * 001a 0001 00 type of data is geoidal undulation (others are not defined but
 *           include deflection of vertical or variation of gravity)
 * 001b 0001 01 encoding (00 is 4-byte big endian, 01 is variable length)
 * 001c 0001 01 data are scalar (order of data if there are more components
 *           is not yet defined)
 * 001d 0004 00000000 there are no x-y pairs of components
 * 0021 0002 fff0 scale factor as binary exponent is -16, one ulp is 1/65536 m
 * 0023 0008 tolerance of conversion
 * 002b 0008 limit of subdivision. If a geoquad is partly NaN and partly number,
 *           it will not be subdivided if it's smaller than this.
 * 0033 0008 smallest island or lacuna of data that won't be missed
 * 003b 0002 number of source files × 2
 * 003d vary names of source files alternating with names of formats, each
 *           null-terminated
 * vary vary six quadtrees of geoquads
 * 
 * Quadtrees look like this:
 * An empty face of the earth:
 * 00 20
 * A face with just one geoquad:
 * 00 9de923 739b 800563 819a18 42e2 7f06
 * Three quarters undivided, the upper right subdivided in quarters, all NaN:
 * 01 20 00 20 00 20 01 20 00 20 00 20 00 20
 *
 * If the data are vectors tangent to the surface, they are encoded according
 * to the angle they make with the center of the face. So a vector pointing
 * straight north at Chamchamal, at the northeast corner of the Benin face,
 * where the bearing to the center of the face is S60°W, is encoded as N15°W,
 * since the bearing from the center to Chamchamal is N45°E.
 */

cubemap cube;

vball::vball()
{
  face=0;
  x=y=0;
}

vball::vball(int f,xy p)
{
  face=f;
  x=p.getx();
  y=p.gety();
}

xy vball::getxy()
{
  return xy(x,y);
}

double vball::diag()
/* Used in geoidboundary as a fake distance, because all distances involved
 * are orthogonal.
 */
{
  return x+y;
}

vball encodedir(xyz dir)
{
  vball ret;
  double absx,absy,absz;
  absx=fabs(dir.getx());
  absy=fabs(dir.gety());
  absz=fabs(dir.getz());
  if (absx==0 && absy==0 && absz==0)
  {
    ret.face=0;
    ret.x=ret.y=0;
  }
  else if (std::isnan(absx) || std::isnan(absy) || std::isnan(absz) ||
           ((std::isinf(absx)+std::isinf(absy)+std::isinf(absz))>1))
  {
    ret.face=7;
    ret.x=ret.y=NAN;
  }
  else
  {
    if (absx>=absy && absx>=absz)
    {
      ret.face=1;
      ret.x=dir.gety()/absx;
      ret.y=dir.getz()/dir.getx();
      if (dir.getx()<0)
	ret.face=6;
    }
    if (absy>=absz && absy>=absx)
    {
      ret.face=2;
      ret.x=dir.getz()/absy;
      ret.y=dir.getx()/dir.gety();
      if (dir.gety()<0)
	ret.face=5;
    }
    if (absz>=absx && absz>=absy)
    {
      ret.face=3;
      ret.x=dir.getx()/absz;
      ret.y=dir.gety()/dir.getz();
      if (dir.getz()<0)
	ret.face=4;
    }
  }
  return ret;
}

xyz decodedir(vball code)
{
  xyz ret;
  switch (code.face&7)
  {
    case 0:
      ret=xyz(0,0,0);
      break;
    case 1:
      ret=xyz(1,code.x,code.y);
      break;
    case 2:
      ret=xyz(code.y,1,code.x);
      break;
    case 3:
      ret=xyz(code.x,code.y,1);
      break;
    case 4:
      ret=xyz(code.x,-code.y,-1);
      break;
    case 5:
      ret=xyz(-code.y,-1,code.x);
      break;
    case 6:
      ret=xyz(-1,code.x,-code.y);
      break;
    case 7:
      ret=xyz(NAN,NAN,NAN);
      break;
  }
  if ((code.face&7)%7)
    ret=ret*(EARTHRAD/ret.length());
  return ret;
}

double cylinterval::area()
{
  return -(sin(nbd)-sin(sbd))*bintorad(wbd-ebd)*EARTHRADSQ;
  // -(wbd-ebd) because DEG360 is negative as a signed integer
}

void cylinterval::setfull()
{
  sbd=-DEG90;
  nbd=DEG90;
  wbd=-DEG180;
  ebd=DEG180;
}

void cylinterval::setempty()
{
  sbd=nbd=wbd=ebd=0;
}

void cylinterval::round(int fineness)
/* fineness is the number of units in a half circle, typically 10800 (1 minute).
 * Rounding is done to the excerptinterval. Rounding the inputbounds could
 * result in extra columns or rows of NaN at the border.
 */
{
  sbd=rottobin(rint(bintorot(sbd)*2*fineness)/2/fineness);
  nbd=rottobin(rint(bintorot(nbd)*2*fineness)/2/fineness);
  wbd=rottobin(rint(bintorot(wbd)*2*fineness)/2/fineness);
  ebd=rottobin(rint(bintorot(ebd)*2*fineness)/2/fineness);
}

cylinterval combine(cylinterval a,cylinterval b)
/* Given two cylintervals, returns the smallest cylinterval containing both.
 * Order is ignored except in the following cases:
 * • a and b are both empty. Returns b.
 * • a and b both have 360° longitude intervals. Returns the longitude interval of a.
 * • a's central meridian is 180° from b's.
 */
{
  cylinterval ret;
  assert(a.sbd>=-DEG90);
  assert(a.nbd>=a.sbd);
  assert(a.nbd<=DEG90);
  assert(a.wbd-a.ebd<1);
  assert(b.sbd>=-DEG90);
  assert(b.nbd>=b.sbd);
  assert(b.nbd<=DEG90);
  assert(b.wbd-b.ebd<1);
  if (a.nbd>b.nbd)
    ret.nbd=a.nbd;
  else
    ret.nbd=b.nbd;
  if (a.sbd<b.sbd)
    ret.sbd=a.sbd;
  else
    ret.sbd=b.sbd;
  if (a.ebd==a.wbd || a.nbd==a.sbd)
    ret=b;
  else if (b.ebd==b.wbd || b.nbd==b.sbd)
    ret=a;
  else if ((a.ebd^a.wbd)==DEG360)
  {
    ret.ebd=a.ebd;
    ret.wbd=a.wbd;
  }
  else if ((b.ebd^b.wbd)==DEG360)
  {
    ret.ebd=b.ebd;
    ret.wbd=b.wbd;
  }
  else
  {
    if (a.ebd+a.wbd-b.ebd-b.wbd>0)
      swap(a,b);
    if ((a.ebd+(a.wbd-a.ebd)/2)-(b.ebd+(b.wbd-b.ebd)/2)>0)
    {
      b.ebd+=DEG360;
      b.wbd+=DEG360;
    }
    if (b.ebd-a.ebd>0)
      ret.ebd=b.ebd;
    else
      ret.ebd=a.ebd;
    if (b.wbd-a.wbd>0)
      ret.wbd=a.wbd;
    else
      ret.wbd=b.wbd;
    if (ret.wbd-ret.ebd>0)
      ret.ebd=ret.wbd+DEG360;
  }
  assert(ret.wbd-ret.ebd<1);
  return ret;
}

cylinterval intersect(cylinterval a,cylinterval b)
/* Given two cylintervals, returns the largest cylinterval contained in both.
 * Order is ignored except in the following cases:
 * • a and b are both empty. Returns a.
 * • a and b both have 360° longitude intervals. Returns the longitude interval of b.
 * • a's central meridian is 180° from b's.
 */
{
  cylinterval ret;
  assert(a.sbd>=-DEG90);
  assert(a.nbd>=a.sbd);
  assert(a.nbd<=DEG90);
  assert(a.wbd-a.ebd<1);
  assert(b.sbd>=-DEG90);
  assert(b.nbd>=b.sbd);
  assert(b.nbd<=DEG90);
  assert(b.wbd-b.ebd<1);
  if (a.nbd>b.nbd)
    ret.nbd=b.nbd;
  else
    ret.nbd=a.nbd;
  if (a.sbd<b.sbd)
    ret.sbd=b.sbd;
  else
    ret.sbd=a.sbd;
  if (a.ebd==a.wbd || a.nbd==a.sbd)
    ret=a;
  else if (b.ebd==b.wbd || b.nbd==b.sbd)
    ret=b;
  else if ((a.ebd^a.wbd)==DEG360)
  {
    ret.ebd=b.ebd;
    ret.wbd=b.wbd;
  }
  else if ((b.ebd^b.wbd)==DEG360)
  {
    ret.ebd=a.ebd;
    ret.wbd=a.wbd;
  }
  else
  {
    if (a.ebd+a.wbd-b.ebd-b.wbd>0)
      swap(a,b);
    if ((a.ebd+(a.wbd-a.ebd)/2)-(b.ebd+(b.wbd-b.ebd)/2)>0)
    {
      b.ebd+=DEG360;
      b.wbd+=DEG360;
    }
    if (b.ebd-a.ebd>0)
      ret.ebd=a.ebd;
    else
      ret.ebd=b.ebd;
    if (b.wbd-a.wbd>0)
      ret.wbd=b.wbd;
    else
      ret.wbd=a.wbd;
    if (ret.wbd-ret.ebd>0)
      ret.ebd=ret.wbd;
    if (ret.nbd<ret.sbd)
      ret.nbd=ret.sbd;
  }
  assert(ret.wbd-ret.ebd<1);
  return ret;
}

int gap(cylinterval a,cylinterval b)
/* Returns the smaller gap between the longitudes of a and b.
 * If the longitudes overlap, the gap is negative.
 */
{
  if (a.ebd+a.wbd-b.ebd-b.wbd>0)
    swap(a,b);
  if ((double)(b.ebd-a.ebd)+(double)(b.wbd-a.wbd)<0)
  {
    b.ebd+=DEG360;
    b.wbd+=DEG360;
  }
  return b.wbd-a.ebd;
}

bool westof(cylinterval a,cylinterval b)
// This is a linear, not circular, comparison, for sorting.
{
  return a.ebd+a.wbd<b.ebd+b.wbd;
}

cylinterval combine(vector<cylinterval> cyls)
{
  vector<cylinterval> cyls1;
  multimap<int,cylinterval> sortcyl;
  multimap<int,cylinterval>::iterator cyli;
  int biggap,littlegap,ibiggap,i,thisgap,csize;
  if (cyls.size()==0)
  {
    cylinterval cyl{0,0,0,0};
    cyls.push_back(cyl);
  }
  stable_sort(cyls.begin(),cyls.end(),westof); // sort() makes the program crash
  do
  {
    csize=cyls.size();
    for (biggap=DEG360,littlegap=~DEG360,ibiggap=i=0;i<csize;i++)
    {
      thisgap=gap(cyls[i],cyls[(i+1)%csize]);
      if (thisgap>biggap)
      {
	biggap=thisgap;
	ibiggap=i+1;
      }
      if (thisgap<littlegap)
        littlegap=thisgap;
    }
    cyls1.clear();
    if (littlegap<0)
      littlegap=0;
    //cout<<"biggap "<<bintodeg(biggap)<<" at "<<ibiggap<<"; littlegap "<<bintodeg(littlegap)<<endl;
    for (i=0;i<csize;i++)
    {
      //if (i)
	//cout<<' ';
      //cout<<bintodeg(cyls[(i+ibiggap)%csize].wbd)<<'-'<<bintodeg(cyls[(i+ibiggap)%csize].ebd);
      //cout<<bintodeg(cyls[(i+ibiggap)%csize].wbd+cyls[(i+ibiggap)%csize].ebd)/2;
    }
    //cout<<endl;
    for (i=0;i<csize;i++)
    {
      if (i<csize-1 && gap(cyls[(i+ibiggap)%csize],cyls[(i+1+ibiggap)%csize])<=littlegap)
      {
	cyls1.push_back(combine(cyls[(i+ibiggap)%csize],cyls[(i+1+ibiggap)%csize]));
	i++;
	//cout<<"<>";
      }
      else
      {
	cyls1.push_back(cyls[(i+ibiggap)%csize]);
	//cout<<'*';
      }
    }
    //cout<<endl;
    swap(cyls,cyls1);
  } while (cyls.size()>1);
  return cyls[0];
}

bool geoquad::subdivided() const
/* Unlike qindex, this is architecture-dependent.
 * On 64-bit Intel/AMD architecture, pointer is 8 bytes and int is 4.
 * SSSSSSSSssssssssSSSSSSSSssssssss
 * UUUUuuuuUUUUuuuuUUUUuuuu
 * The last sub is NULL if it is not subdivided and points to the upper right
 * quadrant if it is.
 * If int is 8 bytes (which will require rewriting writegeint):
 * SSSSSSSSssssssssSSSSSSSSssssssss
 * UUUUUUUUuuuuuuuuUUUUUUUUuuuuuuuuUUUUUUUUuuuuuuuu
 * The last und is set to 0x8000000000000000 (which means NAN) if it is subdivided
 * and some small value if it isn't. To indicate that the undulation is unknown,
 * set the first und, but not the last, to NAN.
 * If int is 4 bytes and pointers are 6 bytes, this won't work.
 */
{
  if (sizeof(geoquad *)*3>=sizeof(int)*6)
    return sub[3]!=nullptr;
  if (sizeof(int)*5>=sizeof(geoquad *)*4)
    return und[5]>8850*65536 || und[5]<-11000*65536;
}

bool geoquad::isnan()
{
  return und[0]>8850*65536 || und[0]<-11000*65536;
}

geoquad::geoquad()
{
  int i;
  for (i=0;i<4;i++)
    sub[i]=nullptr;
  for (i=1;i<6;i++)
    und[i]=0;
  und[0]=INT_MIN;
  scale=1;
}

geoquad::~geoquad()
{
  int i;
  if (subdivided())
    for (i=0;i<4;i++)
      delete(sub[i]);
}

geoquad::geoquad(const geoquad& b)
{
  int i;
  if (b.subdivided())
  {
    und[5]=INT_MIN;
    for (i=0;i<4;i++)
      sub[i]=new geoquad(*b.sub[i]);
  }
  else
  {
    sub[3]=nullptr;
    for (i=0;i<6;i++)
      und[i]=b.und[i];
  }
  center=b.center;
  scale=b.scale;
  face=b.face;
#ifdef NUMSGEOID
  nums=b.nums;
  nans=b.nans;
#endif
}

geoquad& geoquad::operator=(geoquad b)
{
  if (sizeof(sub)>sizeof(und))
    swap(sub,b.sub);
  else
    swap(und,b.und);
  swap(center,b.center);
  swap(scale,b.scale);
  swap(face,b.face);
#ifdef NUMSGEOID
  swap(nums,b.nums);
  swap(nans,b.nans);
#endif
  return *this;
}

void geoquad::clear()
{
  int i;
  if (subdivided())
    for (i=0;i<4;i++)
      delete(sub[i]);
  for (i=0;i<4;i++)
    sub[i]=nullptr;
  for (i=1;i<6;i++)
    und[i]=0;
  und[0]=0x80000000;
}

vball geoquad::vcenter() const
{
  return vball(face,center);
}

void geoquad::subdivide()
/* This makes no attempt to subdivide the surface.
 * The four subsquares are initialized to NAN.
 */
{
  int i,j;
  und[5]=0x80000000;
  for (i=0;i<4;i++)
  {
    sub[i]=new(geoquad);
    sub[i]->scale=scale/2;
    sub[i]->face=face;
    sub[i]->center=xy(center.east()+scale/((i&1)?2:-2),center.north()+scale/((i&2)?2:-2));
#ifdef NUMSGEOID
    for (j=0;j<nans.size();j++)
      if (sub[i]->in(nans[j]))
	sub[i]->nans.push_back(nans[j]);
    for (j=0;j<nums.size();j++)
      if (sub[i]->in(nums[j]))
	sub[i]->nums.push_back(nums[j]);
#endif
  }
#ifdef NUMSGEOID
  nans.clear();
  nums.clear();
  nans.shrink_to_fit();
  nums.shrink_to_fit();
#endif
}

void geoquad::filldepth(int depth)
/* Makes all subdivisions at depth depth exist, if they don't already.
 * The number of leaves after this operation is at least 4**depth.
 */
{
  int i;
  if (depth>0)
  {
    if (!subdivided())
      subdivide();
    for (i=0;i<4;i++)
      sub[i]->filldepth(depth-1);
  }
}

bool geoquad::in(xy pnt) const
{
  return fabs(pnt.east()-center.east())<=scale && fabs(pnt.north()-center.north())<=scale;
}

bool geoquad::in(vball pnt) const
{
  return face==pnt.face && in(xy(pnt.x,pnt.y));
}

double geoquad::undulation(double x,double y)
{
  int xbit,ybit;
  double u;
  if (subdivided())
  {
    xbit=x>=0;
    ybit=y>=0;
    x=2*(x-(xbit-0.5));
    y=2*(y-(ybit-0.5));
    u=sub[(ybit<<1)|xbit]->undulation(x,y);
  }
  else
  {
    u=(und[0]+und[1]*x+und[2]*y+und[3]*(x*x-1/3.)+und[4]*x*y+und[5]*(y*y-1/3.));
    if (u>8850*65536 || u<-11000*65536)
      u=NAN;
  }
  return u;
}

xyz geoquad::centeronearth()
{
  return decodedir(vball(face,center));
}

double geoquad::length()
{
  double r;
  r=xyz(center,1).length();
  return EARTHRAD*2*scale/r;
}

double geoquad::width()
{
  double r;
  r=xyz(center,1).length();
  return EARTHRAD*2*scale/sqr(r);
}

double geoquad::apxarea()
/* apxarea is 6/π (1.9099) times as big as area for a whole face;
 * for a quarter face it is 4% too big;
 * for anything else it is within 1%.
 */
{
  return length()*width();
}

double anglexs(xy pnt)
{
  return asin(pnt.getx()/sqrt(sqr(pnt.getx())+1)*pnt.gety()/sqrt(sqr(pnt.gety())+1));
}

double geoquad::angarea()
{
  xy ne(scale,scale),nw(-scale,scale);
  return ((anglexs(center+ne)+anglexs(center-ne))-(anglexs(center+nw)+anglexs(center-nw)))*EARTHRADSQ;
}

double geoquad::area()
/* apxarea is the area of a parallelogram tangent to the geoquad at its middle.
 * It is accurate for small geoquads, but not large ones.
 * angarea is the exact area computed by angle excess. It is accurate for large
 * geoquads, but for small ones subtracts two nearly equal numbers.
 * The difference between them is least at level 12 or 13, so switch between them there.
 */
{
  if (scale>0.0002)
    return angarea();
  else
    return apxarea();
}

#ifdef NUMSGEOID
int geoquad::isfull()
/* Returns -1 if the square has been interrogated and all points found to have no geoid data.
 * Returns 0 if some points have geoid data and some do not, or if no points have been tested.
 * Returns 1 if all points tested have geoid data.
 */
{
  return (nums.size()>0)-(nans.size()>0);
}
#endif

vector<cylinterval> geoquad::boundrects()
{
  vector<cylinterval> ret,subret;
  int i,j;
  vector<int> lats,lons;
  vball v;
  xyz pnt;
  cylinterval cyl;
  if (subdivided())
    for (i=0;i<4;i++)
    {
      subret=sub[i]->boundrects();
      for (j=0;j<subret.size();j++)
	ret.push_back(subret[j]);
    }
  else if (!isnan())
  {
    for (i=-1;i<=1;i++)
      for (j=-1;j<=1;j++)
      {
	v=vball(face,center+xy(j*scale,i*scale));
	pnt=decodedir(v);
	lats.push_back(pnt.lati());
	lons.push_back(pnt.loni());
      }
    for (i=0;i<9;i++)
      lons[i]=lons[4]+foldangle(lons[i]-lons[4]);
    cyl.nbd=cyl.ebd=-DEG270;
    cyl.sbd=cyl.wbd=DEG270;
    for (i=0;i<9;i++)
    {
      if (lats[i]>cyl.nbd)
	cyl.nbd=lats[i];
      if (lats[i]<cyl.sbd)
	cyl.sbd=lats[i];
      if (lons[i]>cyl.ebd)
	cyl.ebd=lons[i];
      if (lons[i]<cyl.wbd)
	cyl.wbd=lons[i];
    }
    if (cyl.ebd-cyl.wbd>DEG180) // This happens ONLY if it's the entire Arctic or Antarctic face.
      cyl.ebd=cyl.wbd+DEG360;
    ret.push_back(cyl);
  }
  return ret;
}

array<vball,4> geoquad::bounds() const
/* The four sides of a geoquad are segments of great circles.
 * This returns them as the centers of the great circles.
 */
{
  array<vball,4> ret;
  switch (face)
  {
    case 1:
      ret[0].face=2;
      ret[1].face=3;
      ret[2].face=5;
      ret[3].face=4;
      break;
    case 2:
      ret[0].face=3;
      ret[1].face=1;
      ret[2].face=4;
      ret[3].face=6;
      break;
    case 3:
      ret[0].face=1;
      ret[1].face=2;
      ret[2].face=6;
      ret[3].face=5;
      break;
    case 6:
      ret[0].face=5;
      ret[1].face=4;
      ret[2].face=2;
      ret[3].face=3;
      break;
    case 5:
      ret[0].face=4;
      ret[1].face=6;
      ret[2].face=3;
      ret[3].face=1;
      break;
    case 4:
      ret[0].face=6;
      ret[1].face=5;
      ret[2].face=1;
      ret[3].face=2;
      break;
    default:
      ret[0].face=ret[1].face=ret[2].face=ret[3].face=face;
  }
  if (face&4)
  {
    ret[0].y=center.getx()-scale;
    ret[1].x=center.gety()+scale;
    ret[2].y=center.getx()+scale;
    ret[3].x=-center.gety()+scale;
  }
  else
  {
    ret[0].y=-center.getx()-scale;
    ret[1].x=-center.gety()-scale;
    ret[2].y=-center.getx()+scale;
    ret[3].x=center.gety()-scale;
  }
  ret[0].x=ret[1].y=ret[2].x=ret[3].y=0;
  return ret;
}

void geoquad::writeBinary(ostream &ofile,int nesting)
{
  int i;
  if (subdivided())
    for (i=0;i<4;i++)
    {
      sub[i]->writeBinary(ofile,nesting+1);
      nesting=-1;
    }
  else
  {
    ofile<<(char)nesting;
    for (i=0;i<(isnan()?1:6);i++)
      writegeint(ofile,und[i]);
  }
}

void geoquad::readBinary(istream &ifile,int nesting)
{
  int i;
  clear();
  if (nesting<0)
  {
    nesting=ifile.get();
    //cout<<"Read nesting "<<nesting<<endl;
  }
  if (nesting<0 || nesting>56)
    throw baddata;
  if (nesting>0)
  {
    subdivide();
    for (i=0;i<4;i++)
    {
      sub[i]->readBinary(ifile,nesting-1);
      nesting=0;
    }
  }
  else
  {
    und[0]=readgeint(ifile);
    if (!isnan())
      for (i=1;i<6;i++)
	und[i]=readgeint(ifile);
  }
}

void geoquad::dump(ostream &ofile,int nesting)
{
  int i;
  latlong ll;
  if (subdivided())
    for (i=0;i<4;i++)
    {
      sub[i]->dump(ofile,nesting+1);
      nesting=-1;
    }
  else
  {
    ll=centeronearth().latlon();
    ofile<<formatlatlong(ll,DEGREE+SEXAG2);
    ofile<<' '<<face<<' '<<ldecimal(center.getx())<<' '<<ldecimal(center.gety());
    ofile<<" 1/"<<1/scale;
    for (i=0;i<(isnan()?1:6);i++)
      ofile<<' '<<und[i];
    ofile<<endl;
  }
}

unsigned byteswap(unsigned n)
{
  return ((n&0xff000000)>>24)|((n&0xff0000)>>8)|((n&0xff00)<<8)|((n&0xff)<<24);
}

array<unsigned,2> geoquad::hash()
{
  array<unsigned,2> ret,subhash;
  array<unsigned,8> subhashes;
  int i;
  if (subdivided())
  {
    for (i=0;i<4;i++)
    {
      subhash=sub[i]->hash();
      subhashes[2*i]=subhash[0];
      subhashes[2*i+1]=subhash[1];
    }
    for (i=ret[0]=ret[1]=0;i<8;i++)
    {
      ret[0]=byteswap((ret[0]^subhashes[i])*1657);
      ret[1]=byteswap((ret[1]^subhashes[7-i])*6371);
    }
  }
  else
    for (i=ret[0]=ret[1]=0;i<6;i++)
    {
      ret[0]=byteswap((ret[0]^und[i])*99421);
      ret[1]=byteswap((ret[1]^und[5-i])*47935);
    }
  return ret;
}

array<int,6> geoquad::undrange()
/* The first two are the minimum and maximum of the constant term,
 * the middle two are the range of the linear terms, and the last
 * two are the range of the quadratic terms.
 */
{
  int i,j;
  array<int,6> ret,subret;
  ret[0]=ret[2]=ret[4]=INT_MAX;
  ret[1]=ret[3]=ret[5]=INT_MIN;
  if (subdivided())
    for (i=0;i<4;i++)
    {
      subret=sub[i]->undrange();
      for (j=0;j<6;j+=2)
      {
        if (subret[j]<ret[j])
          ret[j]=subret[j];
        if (subret[j+1]>ret[j+1])
          ret[j+1]=subret[j+1];
      }
    }
  else if (!isnan())
  {
    ret[0]=ret[1]=und[0];
    for (i=1;i<6;i++)
    {
      if (und[i]<ret[(i/3)*2+2])
        ret[(i/3)*2+2]=und[i];
      if (und[i]>ret[(i/3)*2+3])
        ret[(i/3)*2+3]=und[i];
    }
  }
  return ret;
}

array<int,5> geoquad::undhisto()
/* Count how many undulation values would take 1, 2, 3, or 4 bytes in
 * a certain encoding.
 */
{
  int i,j;
  array<int,5> ret,subret;
  ret.fill(0);
  if (subdivided())
    for (i=0;i<4;i++)
    {
      subret=sub[i]->undhisto();
      for (j=0;j<5;j++)
        ret[j]+=subret[j];
    }
  else if (!isnan())
  {
    for (i=0;i<6;i++)
    {
      if (abs(und[i])<32)
        ret[0]++;
      else if (abs(und[i])<8224)
        ret[1]++;
      else if (abs(und[i])<2105376)
        ret[2]++;
      else if (abs(und[i])<538976288)
        ret[3]++;
      else
        ret[4]++; // This never happens in geoid files; it means >8224 m.
    }
  }
  else
    ret[0]=1;
  return ret;
}

cubemap::cubemap()
{
  int i;
  for (i=0;i<6;i++)
    faces[i].face=i+1;
}

void cubemap::clear()
{
  int i;
  for (i=0;i<6;i++)
    faces[i].clear();
}

cubemap::~cubemap()
{
  clear();
}

double cubemap::undulation(int lat,int lon)
{
  return undulation(Sphere.geoc(lat,lon,0));
}

double cubemap::undulation(latlong ll)
{
  return undulation(Sphere.geoc(ll,0));
}

double cubemap::undulation(xyz dir)
{
  vball v=encodedir(dir);
  if (v.face<1 || v.face>6)
    return NAN;
  else
    return faces[v.face-1].undulation(v.x,v.y)*scale;
}

array<unsigned,2> cubemap::hash()
{
  array<unsigned,2> ret,subhash;
  array<unsigned,12> subhashes;
  int i;
  for (i=0;i<6;i++)
  {
    subhash=faces[i].hash();
    subhashes[2*i]=subhash[0];
    subhashes[2*i+1]=subhash[1];
  }
  for (i=ret[0]=ret[1]=0;i<12;i++)
  {
    ret[0]=byteswap((ret[0]^subhashes[i])*7225);
    ret[1]=byteswap((ret[1]^subhashes[11-i])*3937);
  }
  return ret;
}

vector<cylinterval> cubemap::boundrects()
{
  vector<cylinterval> ret,subret;
  int i,j;
  for (i=0;i<6;i++)
  {
    subret=faces[i].boundrects();
    for (j=0;j<subret.size();j++)
      ret.push_back(subret[j]);
  }
  return ret;
}

cylinterval cubemap::boundrect()
{
  return combine(boundrects());
}

void cubemap::writeBinary(ostream &ofile)
{
  int i;
  for (i=0;i<6;i++)
    faces[i].writeBinary(ofile);
}

void cubemap::readBinary(istream &ifile)
{
  int i;
  for (i=0;i<6;i++)
    faces[i].readBinary(ifile);
}

void cubemap::dump(ostream &ofile)
{
  int i;
  ofile<<"Scale ";
  if (scale>0 && scale<1)
    ofile<<"1/"<<1/scale<<endl;
  else
    ofile<<scale<<endl;
  for (i=0;i<6;i++)
    faces[i].dump(ofile);
}

array<int,6> cubemap::undrange()
{
  int i,j;
  array<int,6> ret,subret;
  ret[0]=ret[2]=ret[4]=INT_MAX;
  ret[1]=ret[3]=ret[5]=INT_MIN;
  for (i=0;i<6;i++)
  {
    subret=faces[i].undrange();
    for (j=0;j<6;j+=2)
    {
      if (subret[j]<ret[j])
        ret[j]=subret[j];
      if (subret[j+1]>ret[j+1])
        ret[j+1]=subret[j+1];
    }
  }
  return ret;
}

array<int,5> cubemap::undhisto()
{
  int i,j;
  array<int,5> ret,subret;
  ret.fill(0);
  for (i=0;i<6;i++)
  {
    subret=faces[i].undhisto();
    for (j=0;j<5;j++)
    {
      ret[j]+=subret[j];
    }
  }
  return ret;
}

void geoheader::writeBinary(std::ostream &ofile)
{
  int i;
  ofile<<"boldatni";
  writebeint(ofile,hash[0]);
  writebeint(ofile,hash[1]);
  writebeint(ofile,origHash[0]);
  writebeint(ofile,origHash[1]);
  writebeshort(ofile,planet);
  ofile.put(dataType);
  ofile.put(encoding);
  ofile.put(ncomponents);
  writebeint(ofile,xComponentBits);
  writebeshort(ofile,logScale);
  writebedouble(ofile,tolerance);
  writebedouble(ofile,sublimit);
  writebedouble(ofile,spacing);
  writebeshort(ofile,namesFormats.size());
  for (i=0;i<namesFormats.size();i++)
    writeustring(ofile,namesFormats[i]);
}

void geoheader::readBinary(std::istream &ifile)
{
  int i,nstrings;
  char magic[8]="ABCDEFG";
  ifile.read(magic,8);
  if (memcmp(magic,"boldatni",8))
    throw badheader;
  hash[0]=readbeint(ifile);
  hash[1]=readbeint(ifile);
  origHash[0]=readbeint(ifile);
  origHash[1]=readbeint(ifile);
  planet=readbeshort(ifile);
  dataType=ifile.get();
  encoding=ifile.get();
  ncomponents=ifile.get();
  xComponentBits=readbeint(ifile);
  logScale=readbeshort(ifile);
  tolerance=readbedouble(ifile);
  sublimit=readbedouble(ifile);
  spacing=readbedouble(ifile);
  nstrings=readbeshort(ifile);
  namesFormats.clear();
  for (i=0;i<nstrings;i++)
    namesFormats.push_back(readustring(ifile));
}
