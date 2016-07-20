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
#include <cmath>
#include <iostream>
#include "geoid.h"
#include "binio.h"
#include "angle.h"
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
 * 0008 0002 0000 file refers to the earth (other planets/moons have different
 *           sizes, so the limit of subdivision and smallest island are
 *           relatively different)
 * 0012 0001 00 type of data is geoidal undulation (others are not defined but
 *           include deflection of vertical or variation of gravity)
 * 0013 0001 01 encoding (00 is 4-byte big endian, 01 is variable length)
 * 0014 0001 01 data are scalar (order of data if there are more components
 *           is not yet defined)
 * 0015 0002 fff0 scale factor as binary exponent is -16, one ulp is 1/65536 m
 * 0017 0008 tolerance of conversion
 * 001f 0008 limit of subdivision. If a geoquad is partly NaN and partly number,
 *           it will not be subdivided if it's smaller than this.
 * 0027 0008 smallest island or lacuna of data that won't be missed
 * 002f 0002 number of source files × 2
 * 0031 vary names of source files alternating with names of formats, each
 *           null-terminated
 * vary vary six quadtrees of geoquads
 * 
 * Quadtrees look like this:
 * An empty face of the earth:
 * 00 8000
 * A face with just one geoquad:
 * 00 1e0943 fff382 002583 01ba38 000302 fffeed
 * Three quarters undivided, the upper right subdivided in quarters, all NaN:
 * 01 8000 00 8000 00 8000 01 8000 00 8000 00 8000 00 8000
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
    ret=ret*(6371e3/ret.length());
  return ret;
}

bool geoquad::subdivided()
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
    return und[5]>8850*256 || und[5]<-11000*256;
}

bool geoquad::isnan()
{
  return und[0]>8850*256 || und[0]<-11000*256;
}

geoquad::geoquad()
{
  int i;
  for (i=0;i<4;i++)
    sub[i]=nullptr;
  for (i=1;i<6;i++)
    und[i]=0;
  und[0]=0x80000000;
  scale=1;
}

geoquad::~geoquad()
{
  int i;
  if (subdivided())
    for (i=0;i<4;i++)
      delete(sub[i]);
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

vball geoquad::vcenter()
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
    for (j=0;j<nans.size();j++)
      if (sub[i]->in(nans[j]))
	sub[i]->nans.push_back(nans[j]);
    for (j=0;j<nums.size();j++)
      if (sub[i]->in(nums[j]))
	sub[i]->nums.push_back(nums[j]);
  }
  nans.clear();
  nums.clear();
  nans.shrink_to_fit();
  nums.shrink_to_fit();
}

bool geoquad::in(xy pnt)
{
  return fabs(pnt.east()-center.east())<=scale && fabs(pnt.north()-center.north())<=scale;
}

bool geoquad::in(vball pnt)
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
  return 6371e3*2*scale/r;
}

double geoquad::width()
{
  double r;
  r=xyz(center,1).length();
  return 6371e3*2*scale/sqr(r);
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

int geoquad::isfull()
/* Returns -1 if the square has been interrogated and all points found to have no geoid data.
 * Returns 0 if some points have geoid data and some do not, or if no points have been tested.
 * Returns 1 if all points tested have geoid data.
 */
{
  return (nums.size()>0)-(nans.size()>0);
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

void geoheader::writeBinary(std::ostream &ofile)
{
  int i;
  ofile<<"boldatni";
  writebeint(ofile,hash[0]);
  writebeint(ofile,hash[1]);
  writebeshort(ofile,planet);
  ofile.put(dataType);
  ofile.put(encoding);
  ofile.put(ncomponents);
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
  planet=readbeshort(ifile);
  dataType=ifile.get();
  encoding=ifile.get();
  ncomponents=ifile.get();
  logScale=readbeshort(ifile);
  tolerance=readbedouble(ifile);
  sublimit=readbedouble(ifile);
  spacing=readbedouble(ifile);
  nstrings=readbeshort(ifile);
  namesFormats.clear();
  for (i=0;i<nstrings;i++)
    namesFormats.push_back(readustring(ifile));
}
