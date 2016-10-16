/******************************************************/
/*                                                    */
/* brent.cpp - Brent's root-finding method            */
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
#include <utility>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "cogo.h"
#include "ldecimal.h"
#include "brent.h"

using namespace std;

/* Input:
 * 9s trit is the sign of the contrapoint,
 * 3s trit is the sign of the new point,
 * 1s trit is the sign of the old point.
 * Output:
 * Bits 0 and 1:
 * 0: done
 * 1: new point becomes old point
 * 2: new point becomes contrapoint
 * 3: should never happen.
 * Bit 2: true if new point should be moved one ulp toward old point, if s==x.
 */
char sidetable[27]=
{
  3,7,6, 3,0,0, 3,3,1,
  3,3,7, 3,0,3, 7,3,3,
  1,3,3, 0,0,3, 6,7,3
};

double invquad(double x0,double y0,double x1,double y1,double x2,double y2)
{
  double z0,z1,z2,offx;
  z0=x0;
  z1=x1;
  z2=x2;
  if (z0>z1)
    swap(z0,z1);
  if (z1>z2)
    swap(z1,z2);
  if (z0>z1)
    swap(z0,z1);
  //cout<<"Brent: "<<ldecimal(x0)<<' '<<ldecimal(x1)<<' '<<ldecimal(x2)<<endl;
  //cout<<"Before: "<<ldecimal(x0-x1)<<' '<<ldecimal(x1-x2)<<' '<<ldecimal(x2-x0)<<endl;
  if (z0<0 && z2>0)
    offx=0;
  if (z0>=0)
    offx=z0;
  if (z2<=0)
    offx=z2;
  x0-=offx;
  x1-=offx;
  x2-=offx;
  //cout<<"After:  "<<ldecimal(x0-x1)<<' '<<ldecimal(x1-x2)<<' '<<ldecimal(x2-x0)<<endl;
  z0=x0*y1*y2/(y0-y1)/(y0-y2);
  z1=x1*y2*y0/(y1-y2)/(y1-y0);
  z2=x2*y0*y1/(y2-y0)/(y2-y1);
  return (z0+z1+z2)+offx;
}

bool brent::between(double s)
{
  double g=(3*a+b)/4;
  return (g<s && s<=b) || (b<=s && s<g);
  /* <= to make sqrt(3) compute faster, since 1.7320508075688774
   * and 1.7320508075688772 have equal and opposite errors, resulting
   * in repeated unnecessary bisection if the test is b<s
   */
}

double brent::init(double x0,double y0,double x1,double y1,bool intmode)
{
  imode=intmode;
  debug=false;
  if (fabs(y0)>fabs(y1))
  {
    c=a=x0;
    fc=fa=y0;
    b=x1;
    fb=y1;
  }
  else
  {
    c=a=x1;
    fc=fa=y1;
    b=x0;
    fb=y0;
  }
  mflag=true;
  lflag=false;
  side=1;
  x=b-fb*(a-b)/(fa-fb);
  if (imode)
    x=rint(x);
  if (!between(x))
    x=(a+b)/2;
  if (imode)
    x=rint(x);
  if ((y0>0 && y1>0) || (y0<0 && y1<0))
    x=NAN;
  return x;
}

double brent::step(double y)
{
  double s,bsave=b,fbsave=fb;
  bool iq,lf=false;
  if (lflag)
    if (sign(y)*sign(fb)<=0)
      lf=true;
    else
      lflag=false;
  if (fa==fb || fb==y || y==fa)
  {
    s=x-y*(b-x)/(fb-y);
    iq=false;
  }
  else
  {
    s=invquad(a,fa,b,fb,x,y);
    iq=true;
  }
  if (imode)
    s=rint(s);
  if (x==s)
  {
    if (debug)
      cout<<"Same as last time"<<endl;
    side=sidetable[9*sign(fa)+3*sign(y)+sign(fb)+13]&4;
    if (imode)
    {
      if (b<x)
        side^=4;
      if (side)
        s++;
      else
        s--;
    }
    else
      s=nextafter(s,side?b:a);
    lflag=true;
  }
  if (debug)
  {
    cout<<setw(23)<<ldecimal(a)<<setw(23)<<ldecimal(b)<<setw(23)<<ldecimal(x)<<' '<<iq<<endl;
    cout<<setw(23)<<ldecimal(fa)<<setw(23)<<ldecimal(fb)<<setw(23)<<ldecimal(y)<<endl;
    cout<<"s="<<ldecimal(s);
  }
  if (lf)
  {
    mflag=true;
    s=(b+x)/2;
  }
  else if (between(s) && fabs(s-x)<fabs(mflag?x-b:b-c)/2)
    mflag=false;
  else
  {
    mflag=true;
    s=(a+b)/2;
    /* In tolted, init is fed two angles 60° apart, with values (the 3rd derivative,
     * whose zero is being sought) -907.24943 and +907.24943. It produces the
     * midpoint, 30° between them. The first iter also produces s=30°. This causes
     * premature termination by the "same as last time" rule. To prevent this,
     * if s==x, check y (which is 826.96926) and set s to the midpoint of
     * the interval with opposite signs.
     */
    if (s==x)
      if (sign(y)==sign(fa))
        s=(b+x)/2;
      else
        s=(a+x)/2;
  }
  if (imode)
    s=rint(s);
  if (debug)
    cout<<' '<<ldecimal(s);
  side=sidetable[9*sign(fa)+3*sign(y)+sign(fb)+13]&3;
  /* The following pathological case came up running testcontour on the Raspberry Pi:
   * a=1.477900881127366 (hereinafter 3660), b and x alternate between 3657 and 3662.
   * f(3660)=2.22e-16 (hereinafter ε), f(3662)=f(3657)=-ε.
   */
  //if (side%3 && ((b<a && a<x) || (x<a && a<b)))
  //  side=0;
  switch (side)
  {
    case 0:
      s=x;
      break;
    case 1:
      b=x;
      fb=y;
      break;
    case 2:
      a=x;
      fa=y;
      break;
    case 3:
      s=NAN;
  }
  if (mflag && (s==a || s==b)) // interval [a,b] is too small to bisect, we're done
  {
    s=b;
    side=0;
  }
  if (side%3)
  {
    if (fabs(fb)>fabs(fa))
    {
      swap(fa,fb);
      swap(a,b);
    }
    d=c;
    c=bsave;
    x=s;
    fd=fc;
    fc=fbsave;
  }
  if (debug)
    cout<<" side="<<side<<endl;
  return s;
}
