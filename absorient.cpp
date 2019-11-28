/******************************************************/
/*                                                    */
/* absorient.cpp - 2D absolute orientation            */
/*                                                    */
/******************************************************/
/* Copyright 2015,2018,2019 Pierre Abbat.
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
/* Given two pointlists and a list of matching points,
 * find the rotation and translation to match them with the
 * least sum of square distances. This is called the
 * absolute orientation problem.
 */
#include <assert.h>
#include "absorient.h"
#include "except.h"
#include "manysum.h"
#include "minquad.h"
using namespace std;

double sumsqdist(vector<xy> a,vector<xy> b)
{
  int i;
  vector<double> dists;
  assert(a.size()==b.size());
  for (i=0;i<a.size();i++)
    dists.push_back(sqr(dist(a[i],b[i])));
  return pairwisesum(dists);
}

xy pointCentroid(vector<xy> a)
{
  int i;
  vector<double> x,y;
  for (i=0;i<a.size();i++)
  {
    x.push_back(a[i].getx());
    y.push_back(a[i].gety());
  }
  return xy(pairwisesum(x)/i,pairwisesum(y)/i);
}

void sort4(int *ang,double *sqdist)
{
  int i,j;
  for (i=1;i<4;i++)
    for (j=i-1;j>=0;j--)
      if (sqdist[j]>sqdist[j+1])
      {
	swap(sqdist[j],sqdist[j+1]);
	swap(ang[j],ang[j+1]);
      }
  if (sqdist[2]==sqdist[3] && ((ang[1]-ang[0])^(ang[0]-ang[2]))<0)
    swap(ang[2],ang[3]);
}

RoscatStruct absorient(vector<xy> a,vector<xy> b)
// Returns the way to rotate, scale, and translate a to best match b.
{
  int i,j;
  vector<xy> aslide,bslide,arot;
  int ang[4];
  double sqdist[4],newang;
  RoscatStruct ret;
  if (a.size()<2 || b.size()<2)
    throw BeziExcept(badAbsOrient);
  ret.tfrom=pointCentroid(a);
  ret.tto=pointCentroid(b);
  for (i=0;i<a.size();i++)
    aslide.push_back(a[i]-ret.tfrom);
  for (i=0;i<b.size();i++)
    bslide.push_back(b[i]-ret.tto);
  arot=aslide;
  for (i=0;i<4;i++)
  {
    ang[i]=(i-2)*DEG90;
    for (j=0;j<aslide.size();j++)
      arot[j]=turn(aslide[j],ang[i]);
    sqdist[i]=sumsqdist(arot,bslide);
  }
  sort4(ang,sqdist);
  if ((ang[0]-ang[3])&DEG90)
  {
    swap(ang[2],ang[3]);
    swap(sqdist[2],sqdist[3]);
  }
  for (i=1;i<4;i++)
    ang[i]=ang[0]+foldangle(ang[i]-ang[0]);
  while (abs(ang[1]-ang[2])>2)
  {
    newang=minquad(ang[1],sqdist[1],ang[0],sqdist[0],ang[2],sqdist[2]);
    if (!isfinite(newang))
      throw BeziExcept(badAbsOrient);
    ang[3]=rint(newang);
    if (ang[3]==ang[0])
      if (ang[1]+ang[2]-2*ang[0])
	if (abs(ang[2]-ang[0])>abs(ang[1]-ang[0]))
	  ang[3]=ang[0]+(ang[2]-ang[0])/2;
	else
	  ang[3]=ang[0]+(ang[1]-ang[0])/2;
      else
	if (ang[1]-ang[0]<0)
	  ang[3]--;
	else
	  ang[3]++;
    if (ang[3]==ang[1])
      if (ang[0]-ang[1]<0)
      {
	ang[3]--;
	if (ang[3]==ang[0])
	  ang[3]--;
      }
      else
      {
	ang[3]++;
	if (ang[3]==ang[0])
	  ang[3]++;
      }
    if (ang[3]==ang[2])
      if (ang[0]-ang[2]<0)
      {
	ang[3]--;
	if (ang[3]==ang[0])
	  ang[3]--;
      }
      else
      {
	ang[3]++;
	if (ang[3]==ang[0])
	  ang[3]++;
      }
    for (j=0;j<aslide.size();j++)
      arot[j]=turn(aslide[j],ang[3]);
    sqdist[3]=sumsqdist(arot,bslide);
    sort4(ang,sqdist);
  }
  ret.ro=ang[0];
  ret.sca=1;
  return ret;
}

RoscatStruct absorient(pointlist &a,vector<int> ai,pointlist &b,vector<int> bi)
{
  vector<xy> axy,bxy;
  int i;
  for (i=0;i<ai.size();i++)
    axy.push_back(a.points[ai[i]]);
  for (i=0;i<bi.size();i++)
    bxy.push_back(b.points[bi[i]]);
  return absorient(axy,bxy);
}
