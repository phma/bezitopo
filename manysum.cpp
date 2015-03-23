/******************************************************/
/*                                                    */
/* manysum.cpp - add many numbers                     */
/*                                                    */
/******************************************************/
#include <cfloat>
#include <iostream>
#include <cmath>
#include "manysum.h"
using namespace std;

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

manysum& manysum::operator+=(double x)
{
  int i,j;
  frexp(x,&i);
  bucket[i]+=x;
}
