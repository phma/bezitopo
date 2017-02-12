/******************************************************/
/*                                                    */
/* manysum.cpp - add many numbers                     */
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
#include <cfloat>
#include <iostream>
#include <cmath>
#include <vector>
#include "manysum.h"
using namespace std;

int manysum::cnt=0;

void manysum::clear()
{
  bucket.clear();
}

double manysum::total()
{
  map<int,double>::iterator i;
  double t;
  for (t=0,i=bucket.begin();i!=bucket.end();i++)
    t+=i->second;
  return t;
}

void manysum::dump()
{
  map<int,double>::iterator i;
  for (i=bucket.begin();i!=bucket.end();i++)
    cout<<i->first<<' '<<i->second<<endl;
}

void manysum::prune()
{
  vector<int> delenda;
  int j;
  map<int,double>::iterator i;
  for (i=bucket.begin();i!=bucket.end();i++)
    if (i->second==0)
      delenda.push_back(i->first);
  for (j=0;j<delenda.size();j++)
    bucket.erase(delenda[j]);
}

manysum& manysum::operator+=(double x)
{
  int i=DBL_MAX_EXP+3,j=DBL_MAX_EXP+3;
  double d;
  while (x!=0)
  {
    /* frexp(NAN) on Linux sets i to 0. On DragonFly BSD,
     * it leaves i unchanged. This causes the program to hang
     * if j>i. Setting it to DBL_MAX_EXP+5 insures that NAN
     * uses a bucket separate from finite numbers.
     */
    if (std::isfinite(x))
      frexp(x,&i);
    else
      i=DBL_MAX_EXP+5;
    bucket[i]+=x;
    frexp(d=bucket[i],&j);
    if (j>i)
    {
      x=d;
      bucket[i]=0;
      if (((++cnt)&0xff)==0)
	prune();
    }
    else
      x=0;
  }
}

manysum& manysum::operator-=(double x)
{
  int i,j;
  double d;
  x=-x;
  while (x!=0)
  {
    frexp(x,&i);
    bucket[i]+=x;
    frexp(d=bucket[i],&j);
    if (j>i)
    {
      x=d;
      bucket[i]=0;
      if (((++cnt)&0xff)==0)
	prune();
    }
    else
      x=0;
  }
}

double pairwisesum(double *a,unsigned n)
// a is clobbered.
{
  unsigned i,j;
  if (n)
  {
    for (i=1;i<n;i*=2)
      for (j=0;j+i<n;j+=2*i)
	a[j]+=a[j+i];
    return a[0];
  }
  else
    return 0;
}

long double pairwisesum(long double *a,unsigned n)
// a is clobbered.
{
  unsigned i,j;
  if (n)
  {
    for (i=1;i<n;i*=2)
      for (j=0;j+i<n;j+=2*i)
	a[j]+=a[j+i];
    return a[0];
  }
  else
    return 0;
}

double pairwisesum(vector<double> &a)
{
  return pairwisesum(&a[0],a.size());
}
