/******************************************************/
/*                                                    */
/* intloop.cpp - loops of integers (point numbers)    */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include <cassert>
#include <iostream>
#include <cfloat>
#include "intloop.h"
#include "spolygon.h"
#include "manysum.h"
#include "projection.h"
#include "random.h"
#include "relprime.h"
using namespace std;

/* This module takes the convex hull of a pointlist and the boundary of
 * its triangles and computes the boundary of the empty space within
 * the convex hull. This empty space has to be triangulated for the algorithm
 * to find the triangle containing a point to work. The algorithm is the same
 * as for consolidating geoid boundaries (geoidboundary.cpp), but runs on point
 * numbers rather than pointers to points for speed (4 bytes vs. 8 on x86_64).
 * Unlike geoid boundaries, there is no notion of level, and since it's working
 * with integers, there's no way to compute area or perimeter.
 */

int inv2adic(int n)
/* Used for hashing.
 * If n is odd, returns the multiplicative inverse.
 * If n is even, returns 2*(inv2adic(n/2));
 * If n is 0, returns 0.
 */
{
  int ret,exp=0,prod;
  while (n && (n&1)==0)
  {
    exp++;
    n>>=1;
  }
  prod=n*n;
  for (ret=n;prod&~1;)
  {
    prod=ret*n;
    ret+=(1-prod)*ret;
  }
  ret<<=exp;
  return ret;
}

bool int1loop::isempty()
{
  return !bdy.size();
}

int int1loop::size()
{
  return bdy.size();
}

void int1loop::clear()
{
  bdy.clear();
}

void int1loop::push_back(int x)
{
  bdy.push_back(x);
}

int int1loop::operator[](int n)
{
  assert(bdy.size());
  n%=bdy.size();
  if (n<0)
    n+=bdy.size();
  return bdy[n];
}

array<int,2> int1loop::seg(int n)
{
  array<int,2> ret;
  assert(bdy.size());
  n%=(signed)bdy.size();
  if (n<0)
    n+=bdy.size();
  ret[0]=bdy[n];
  ret[1]=bdy[(n+1)%bdy.size()];
  return ret;
}

vector<int> int1loop::nullSegments()
{
  int i;
  vector<int> ret;
  array<int,2> vseg;
  for (i=0;i<bdy.size();i++)
  {
    vseg=seg(i);
    if (vseg[0]==vseg[1])
      ret.push_back(i);
  }
  return ret;
}

void int1loop::positionSegment(int n)
/* Rolls the vector of ints so that the one at n becomes last and the one
 * at n+1 becomes 0th, so that boundaries can be easily spliced.
 */
{
  int m=n+1;
  vector<int> bdy1(bdy);
  if (bdy.size())
  {
    n%=bdy.size();
    if (n<0)
      n+=bdy.size();
    m%=bdy.size();
    if (m<0)
      m+=bdy.size();
    if (m>n)
    {
      memmove(&bdy1[0],&bdy[m],(bdy.size()-m)*sizeof(int));
      memmove(&bdy1[bdy.size()-m],&bdy[0],m*sizeof(int));
      swap(bdy,bdy1);
    }
  }
}

void int1loop::splice(int1loop &b)
{
  int oldsize=bdy.size();
  bdy.resize(oldsize+b.bdy.size());
  memmove(&bdy[oldsize],&b.bdy[0],b.bdy.size()*sizeof(int));
  b.bdy.clear();
}

void int1loop::split(int n,int1loop &b)
{
  n%=(signed int)bdy.size();
  /* "n%=bdy.size()" is wrong. If size()=10, and n=-4, this results in 2,
   * because (4294967296-4)%10=2.
   */
  if (n<0)
    n+=bdy.size();
  b.bdy.resize(bdy.size()-n);
  memmove(&b.bdy[0],&bdy[n],(bdy.size()-n)*sizeof(int));
  bdy.resize(n);
}

void int1loop::splice(int m,int1loop &b,int n)
/* Splice together this, at its mth segment, and b, at its nth segment.
 * this is left with one of the resulting segments between the back and front.
 * b is left empty.
 */
{
  positionSegment(m);
  b.positionSegment(n);
  splice(b);
}

void int1loop::split(int m,int n,int1loop &b)
/* Splits this into two loops, cutting segments m and n and making new ones.
 * Any previous content of b is overwritten.
 */
{
  positionSegment(m);
  split(n-m,b);
}

void int1loop::deleteRetrace()
{
  int i,sz;
  bool found;
  do
  {
    found=false;
    sz=bdy.size();
    for (i=0;i<sz && !found;i++)
      if (bdy[i]==bdy[(i+2)%sz] || bdy[i]==bdy[(i+1)%sz] || bdy[(i+1)%sz]==bdy[(i+2)%sz])
      {
        found=true;
        positionSegment(i+1);
        bdy.resize(sz-1);
      }
  } while (found);
}

bool operator==(const int1loop l,const int1loop r)
/* If one is rotated from the other, returns false. They have to start at
 * the same place for it to return true.
 */
{
  int i,minsize;
  minsize=l.bdy.size();
  if (r.bdy.size()<minsize)
    minsize=r.bdy.size();
  for (i=0;i<minsize && l.bdy[i]==r.bdy[i];i++);
  return i==l.bdy.size() && i==r.bdy.size();
}

void intloop::push_back(int1loop g1)
{
  bdy.push_back(g1);
}

int1loop intloop::operator[](int n)
{
  return bdy[n];
}

int intloop::size() const
{
  return bdy.size();
}

int intloop::totalSegments()
{
  int i,total;
  for (i=total=0;i<bdy.size();i++)
    total+=bdy[i].size();
  return total;
}

array<int,4> intloop::seg(int n)
{
  array<int,4> ret;
  array<int,2> seg1;
  int i;
  for (i=0;i<bdy.size() && n>=0;i++)
  {
    if (n>=0 && n<bdy[i].size())
    {
      seg1=bdy[i].seg(n);
      ret[0]=seg1[0];
      ret[1]=seg1[1];
      ret[2]=i;
      ret[3]=n;
    }
    n-=bdy[i].size();
  }
  return ret;
}

array<int,4> intloop::someSeg()
// Returns a different segment each time; eventually returns all segments.
{
  int t=totalSegments();
  if (t)
  {
    segNum=(segNum+relprime(t))%t;
    if (segNum<0)
      segNum+=t;
  }
  return seg(segNum);
}

array<int,4> intloop::dupSeg()
/* Returns the loop and segment numbers of two segments which are equal and
 * opposite, or {-1,-1,-1,-1} if there are none. Used by consolidate.
 * If both found and exhausted are set, there is a loop consisting of
 * only one point, which will be removed by deleteNullSegments and deleteEmpty.
 */
{
  map<int,vector<array<int,4> > > hashTable;
  int i,j,hash;
  array<int,4> aseg,ret;
  bool found=false,exhausted=false;
  while (!found && !exhausted)
  {
    aseg=someSeg();
    hash=inv2adic(aseg[0])^inv2adic(aseg[1]);
    for (i=0;i<hashTable[hash].size();i++)
    {
      if (hashTable[hash][i][0]==aseg[0])
      {
	ret[0]=ret[1]=ret[2]=ret[3]=-1;
	exhausted=true;
      }
      if (hashTable[hash][i][0]==aseg[1])
      {
	ret[0]=hashTable[hash][i][2];
	ret[1]=hashTable[hash][i][3];
	ret[2]=aseg[2];
	ret[3]=aseg[3];
	found=true;
      }
    }
    hashTable[hash].push_back(aseg);
  }
  return ret;
}

void intloop::clear()
{
  bdy.clear();
}

void intloop::deleteRetrace()
{
  int i;
  for (i=0;i<bdy.size();i++)
    bdy[i].deleteRetrace();
}

void intloop::deleteNullSegments()
{
  int i;
  vector<int> iseg;
  int1loop tmp;
  for (i=0;i<bdy.size();i++)
  {
    while (true)
    {
      iseg=bdy[i].nullSegments();
      if (!iseg.size())
        break;
      bdy[i].split(iseg[0]+1,iseg[0],tmp);
    }
  }
}

void intloop::deleteEmpty()
{
  int i,j;
  for (i=0,j=size()-1;i<=j;)
  {
    while (i<size() && bdy[i].size()>0)
      i++;
    while (j>=0 && bdy[j].size()==0)
      j--;
    if (i<j)
      swap(bdy[i],bdy[j]);
    //cout<<"i="<<i<<" j="<<j<<endl;
  }
  bdy.resize(i);
}

void intloop::erase(int n)
// When erasing many int1loops, erase them in reverse order.
{
  if (n<bdy.size()-1 && n>=0)
    swap(bdy[n],bdy[bdy.size()-1]);
  if (n<bdy.size() && n>=0)
    bdy.resize(bdy.size()-1);
}

intloop operator+(const intloop &l,const intloop &r)
{
  intloop ret;
  int i;
  for (i=0;i<l.size();i++)
    ret.push_back(l.bdy[i]);
  for (i=0;i<r.size();i++)
    ret.push_back(r.bdy[i]);
  return ret;
}
