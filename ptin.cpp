/******************************************************/
/*                                                    */
/* ptin.cpp - PerfectTIN files                        */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <cmath>
#include <cstring>
#include "binio.h"
#include "angle.h"
#include "ptin.h"
using namespace std;

CoordCheck zCheck;

PtinHeader::PtinHeader()
{
  conversionTime=tolRatio=numPoints=numConvexHull=numTriangles=0;
  tolerance=NAN;
}

void CoordCheck::clear()
{
  count=0;
  memset(stage0,0,sizeof(stage0));
  memset(stage1,0,sizeof(stage1));
  memset(stage2,0,sizeof(stage2));
  memset(stage3,0,sizeof(stage3));
  memset(stage4,0,sizeof(stage4));
}

CoordCheck& CoordCheck::operator<<(double val)
{
  int i;
  double lastStageSum;
  for (i=0;i<13;i++)
    if ((count>>i)&1)
      stage0[i][count&8191]=-val;
    else
      stage0[i][count&8191]=val;
  stage0[13][count&8191]=val;
  if ((count&8191)==8191)
  {
    for (i=0;i<13;i++)
      stage1[i][(count>>13)&8191]=pairwisesum(stage0[i],8192);
    lastStageSum=pairwisesum(stage0[13],8192);
    for (i=13;i<26;i++)
      if ((count>>i)&1)
	stage1[i][(count>>13)&8191]=-lastStageSum;
      else
	stage1[i][(count>>13)&8191]=lastStageSum;
    stage1[26][(count>>13)&8191]=lastStageSum;
    memset(stage0,0,sizeof(stage0));
  }
  if ((count&0x3ffffff)==0x3ffffff)
  {
    for (i=0;i<26;i++)
      stage2[i][(count>>26)&8191]=pairwisesum(stage1[i],8192);
    lastStageSum=pairwisesum(stage1[26],8192);
    for (i=26;i<39;i++)
      if ((count>>i)&1)
	stage2[i][(count>>26)&8191]=-lastStageSum;
      else
	stage2[i][(count>>26)&8191]=lastStageSum;
    stage2[39][(count>>26)&8191]=lastStageSum;
    memset(stage1,0,sizeof(stage1));
  }
  if ((count&0x7fffffffff)==0x7fffffffff)
  {
    for (i=0;i<39;i++)
      stage3[i][(count>>39)&8191]=pairwisesum(stage2[i],8192);
    lastStageSum=pairwisesum(stage2[39],8192);
    for (i=39;i<52;i++)
      if ((count>>i)&1)
	stage3[i][(count>>39)&8191]=-lastStageSum;
      else
	stage3[i][(count>>39)&8191]=lastStageSum;
    stage3[52][(count>>39)&8191]=lastStageSum;
    memset(stage2,0,sizeof(stage2));
  }
  if ((count&0xfffffffffffff)==0xfffffffffffff)
  {
    for (i=0;i<52;i++)
      stage4[i][(count>>52)&8191]=pairwisesum(stage3[i],8192);
    lastStageSum=pairwisesum(stage3[52],8192);
    for (i=52;i<64;i++)
      if ((count>>i)&1)
	stage4[i][(count>>52)&8191]=-lastStageSum;
      else
	stage4[i][(count>>52)&8191]=lastStageSum;
    memset(stage3,0,sizeof(stage3));
  }
  count++;
  return *this;
}

double CoordCheck::operator[](int n)
{
  int n0=n,n1=n,n2=n,n3=n;
  int s0=1,s1=1,s2=1,s3=1;
  if (n0>13)
  {
    n0=13;
    s0=1-2*((count>>n)&1);
  }
  if (n1>26)
  {
    n1=26;
    s1=1-2*((count>>n)&1);
  }
  if (n2>39)
  {
    n2=39;
    s2=1-2*((count>>n)&1);
  }
  if (n3>52)
  {
    n3=52;
    s3=1-2*((count>>n)&1);
  }
  return pairwisesum(stage0[n0],8192)*s0+pairwisesum(stage1[n1],8192)*s1+
	 pairwisesum(stage2[n2],8192)*s2+pairwisesum(stage3[n3],8192)*s3+
         pairwisesum(stage4[n],4096);
}

xyz readPoint(istream &file)
{
  double x,y,z;
  x=readledouble(file);
  y=readledouble(file);
  z=readledouble(file);
  return xyz(x,y,z);
}

xyz readPoint4(istream &file)
{
  double x,y,z;
  if (file.eof())
  {
    x=INFINITY;
    y=z=NAN;
  }
  else
  {
    z=y=x=readlefloat(file);
    if (std::isfinite(y))
      z=y=readlefloat(file);
    if (std::isfinite(z))
      z=readlefloat(file);
  }
  return xyz(x,y,z);
}

PtinHeader readPtinHeader(istream &inputFile)
{
  PtinHeader ret;
  int headerFormat;
  if (readleshort(inputFile)==6 && readleshort(inputFile)==28 &&
      readleshort(inputFile)==496 && readleshort(inputFile)==8128)
  {
    headerFormat=readleint(inputFile);
    switch (headerFormat)
    {
      case 0x00000020:
	ret.conversionTime=readlelong(inputFile);
	ret.tolRatio=readleint(inputFile);
	ret.tolerance=readledouble(inputFile);
	ret.numPoints=readleint(inputFile);
	ret.numConvexHull=readleint(inputFile);
	ret.numTriangles=readleint(inputFile);
	break;
      default:
	ret.tolRatio=PT_UNKNOWN_HEADER_FORMAT;
    }
    if (ret.numTriangles!=2*ret.numPoints-ret.numConvexHull-2)
      ret.tolRatio=PT_COUNT_MISMATCH;
  }
  else
    ret.tolRatio=PT_NOT_PTIN_FILE;
  return ret;
}

int skewsym(int a,int b)
/* skewsym(a,b)=-skewsym(b,a). This function is used to check that every edge
 * (a,b) in a triangle is matched by another edge (b,a) in another triangle
 * or in the convex hull. It is nonlinear so that omitting a triangle will not
 * result in 0.
 */
{
  int a1,a2,b1,b2;
  a1=a*0x69969669;
  a2=a*PHITURN;
  b1=b*0x69969669;
  b2=b*PHITURN;
  a1=((a1&0xffe00000)>>21)|(a1&0x1ff800)|((a1&0x7ff)<<21);
  a2=((a2&0xffff0000)>>16)|((a2&0xffff)<<16);
  b1=((b1&0xffe00000)>>21)|(b1&0x1ff800)|((b1&0x7ff)<<21);
  b2=((b2&0xffff0000)>>16)|((b2&0xffff)<<16);
  return a*b1*a2-b*a1*b2;
}

PtinHeader readPtinHeader(std::string inputFile)
{
  ifstream ptinFile(inputFile,ios::binary);
  return readPtinHeader(ptinFile);
}

PtinHeader readPtin(std::string inputFile,pointlist &pl)
{
  ifstream ptinFile(inputFile,ios::binary);
  PtinHeader header;
  int i,j,m,n,a,b,c;
  int edgeCheck=0;
  vector<int> convexHull;
  vector<double> areas;
  triangle *tri;
  xyz pnt,ctr;
  bool readingStarted=false;
  double zError=0,high=-INFINITY,low=INFINITY;
  vector<double> zcheck;
  zCheck.clear();
  header=readPtinHeader(ptinFile);
  if (header.tolRatio>0 && header.tolerance>0)
  {
    pl.clear();
    readingStarted=true;
    for (i=1;i<=header.numPoints;i++)
    {
      pl.points[i]=point(readPoint(ptinFile),"");
      pl.revpoints[&pl.points[i]]=i;
      if (outOfGeoRange(pl.points[i].getx(),pl.points[i].gety(),pl.points[i].getz()))
	header.tolRatio=PT_OUT_OF_RANGE;
      if (ptinFile.eof())
      {
	header.tolRatio=PT_EOF;
	break;
      }
    }
  }
  if (header.tolRatio>0 && header.tolerance>0)
    for (i=0;i<header.numConvexHull;i++)
    {
      n=readleint(ptinFile);
      if (n<1 || n>header.numPoints)
	header.tolRatio=PT_INVALID_POINT_NUMBER;
      if (i)
	edgeCheck+=skewsym(n,convexHull.back());
      if (ptinFile.eof())
      {
	header.tolRatio=PT_EOF;
	break;
      }
      convexHull.push_back(n);
    }
  if (convexHull.size())
    edgeCheck+=skewsym(convexHull[0],convexHull.back());
  //if (header.tolRatio>0 && header.tolerance>0)
    //if (!pl.validConvexHull())
      //header.tolRatio=PT_INVALID_CONVEX_HULL;
  if (header.tolRatio>0 && header.tolerance>0)
    for (i=0;i<header.numTriangles && header.tolRatio>0;i++)
    {
      n=pl.addtriangle();
      //cout<<n<<' ';
      tri=&pl.triangles[n];
      a=readleint(ptinFile);
      //cout<<a<<' ';
      if (a<1 || a>header.numPoints)
	header.tolRatio=PT_INVALID_POINT_NUMBER;
      tri->a=&pl.points[a];
      b=readleint(ptinFile);
      //cout<<b<<' ';
      if (b<1 || b>header.numPoints)
	header.tolRatio=PT_INVALID_POINT_NUMBER;
      tri->b=&pl.points[b];
      c=readleint(ptinFile);
      //cout<<c<<'\n';
      if (c<1 || c>header.numPoints)
	header.tolRatio=PT_INVALID_POINT_NUMBER;
      tri->c=&pl.points[c];
      ctr=((xyz)*tri->a+(xyz)*tri->b+(xyz)*tri->c)/3;
      tri->flatten();
      //cout<<i<<' '<<tri->sarea<<endl;
      if (!(tri->sarea>0)) // so written to catch the NaN case
	header.tolRatio=PT_BACKWARD_TRIANGLE;
      areas.push_back(tri->sarea);
      edgeCheck+=skewsym(a,b)+skewsym(b,c)+skewsym(c,a);
      m=ptinFile.get()&255;
      if (m<255)
	for (j=0;j<m;j++)
	{
	  pnt=readPoint4(ptinFile);
	  if (xy(pnt).length()>tri->peri/3)
	    header.tolRatio=PT_DOT_OUTSIDE;
	  pnt+=ctr;
	  zCheck<<pnt.getz();
	  if (pnt.getz()>high)
	    high=pnt.getz();
	  if (pnt.getz()<low)
	    low=pnt.getz();
	}
      else
	while (true)
	{
	  pnt=readPoint4(ptinFile);
	  if (xy(pnt).length()>tri->peri/3)
	    header.tolRatio=PT_DOT_OUTSIDE;
	  pnt+=ctr;
	  if (pnt.isnan())
	  {
	    if (std::isinf(pnt.getx()))
	      header.tolRatio=PT_EOF;
	    break;
	  }
	  zCheck<<pnt.getz();
	  if (pnt.getz()>high)
	    high=pnt.getz();
	  if (pnt.getz()<low)
	    low=pnt.getz();
	}
    }
  //cout<<"edgeCheck="<<edgeCheck<<endl;
  if (header.tolRatio>0 && header.tolerance>0 && edgeCheck)
    header.tolRatio=PT_EDGE_MISMATCH;
  if (header.tolRatio>0 && header.tolerance>0)
  {
    n=ptinFile.get()&255;
    for (i=0;i<n;i++)
      zcheck.push_back(readledouble(ptinFile));
    if (n==0)
      zcheck.push_back(0);
    while (zcheck.size()<64)
      zcheck.push_back(zcheck.back());
    //cout<<header.tolRatio*header.tolerance*sqrt(zCheck.getCount())<<endl;
    //for (i=0;i<n;i++)
      //cout<<i<<' '<<zcheck[i]<<' '<<zCheck[i]<<' '<<zcheck[i]-zCheck[i]<<endl;
    for (i=0;i<64;i++)
      if (fabs(zcheck[i]-zCheck[i])>zError)
	zError=fabs(zcheck[i]-zCheck[i]);
    //cout<<zError/(header.tolRatio*header.tolerance*sqrt(zCheck.getCount()))<<endl;
    if (zError>header.tolRatio*header.tolerance*sqrt(zCheck.getCount())/65536)
      header.tolRatio=PT_ZCHECK_FAIL;
  }
  if (header.tolRatio>0 && header.tolerance>0)
  {
    pl.makeEdges();
  }
  else if (readingStarted)
    pl.clear();
  return header;
}
