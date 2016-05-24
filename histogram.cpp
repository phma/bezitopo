/******************************************************/
/*                                                    */
/* histogram.cpp - streaming histogram                */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include <cstring>
#include <iostream>
#include <iomanip>
#include "angle.h"
#include "histogram.h"
using namespace std;

histogram::histogram()
{
  bin.push_back(0);
  bin.push_back(1);
  count.push_back(0);
  discrete=0;
  total=0;
}

histogram::histogram(double least,double most)
{
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  discrete=0;
  total=0;
}

void histogram::clear()
{
  double least,most;
  least=bin[0];
  most=bin[count.size()];
  bin.clear();
  count.clear();
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  total=0;
}

void histogram::setdiscrete(double d)
{
  discrete=d;
}

void histogram::clear(double least,double most)
{
  bin.clear();
  count.clear();
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  total=0;
}

void histogram::clearcount()
{
  int i;
  for (i=0;i<count.size();i++)
    count[i]=0;
  total=0;
}

void histogram::split(int n)
{
  bin.push_back(0);
  count.push_back(0);
  memmove(&bin[n+1],&bin[n],sizeof(double)*(bin.size()-n-1));
  memmove(&count[n+1],&count[n],sizeof(unsigned)*(count.size()-n-1));
  count[n+1]=count[n]/2;
  count[n]-=count[n+1];
  bin[n+1]=(bin[n]*count[n+1]+bin[n+2]*count[n])/(count[n]+count[n+1]);
}

int histogram::find(double val)
{
  int lower=0,upper=count.size();
  if (val<bin[lower])
    return -1;
  else if (val>=bin[upper])
    return upper;
  else while (upper-lower>1)
  {
    if (val<bin[(upper+lower)/2])
      upper=(upper+lower)/2;
    else
      lower=(upper+lower)/2;
  }
  return lower;
}

histogram& histogram::operator<<(double val)
{
  int theBin;
  double newlimit;
  theBin=find(val);
  if (theBin>=count.size())
  {
    newlimit=val+(bin[count.size()]-bin[count.size()-1]);
    bin.push_back(newlimit);
    count.push_back(0);
  }
  if (theBin<0)
  {
    newlimit=val+(bin[0]-bin[1]);
    bin.push_back(0);
    count.push_back(0);
    memmove(&bin[1],&bin[0],sizeof(double)*(bin.size()-1));
    memmove(&count[1],&count[0],sizeof(unsigned)*(count.size()-1));
    bin[0]=newlimit;
    count[0]=0;
    theBin=0;
  }
  count[theBin]++;
  total++;
  if (sqr(count[theBin])>total && bin[theBin+1]-bin[theBin]>discrete)
    split(theBin);
  return *this;
}

unsigned histogram::nbars()
{
  return count.size();
}

histobar histogram::getbar(unsigned n)
{
  histobar ret;
  ret.start=bin[n];
  ret.end=bin[n+1];
  ret.count=count[n];
  return ret;
}

unsigned histogram::gettotal()
{
  return total;
}

void histogram::dump()
{
  int i,n;
  histobar bar;
  n=nbars();
  for (i=0;i<n;i++)
  {
    bar=getbar(i);
    cout<<setw(10)<<bar.start<<setw(10)<<bar.end<<setw(10)<<bar.count
    <<setw(10)<<bar.count/(bar.end-bar.start)<<endl;
  }
}
