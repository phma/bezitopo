/******************************************************/
/*                                                    */
/* hnum.cpp - human-graspable number format           */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include <cstdio>
#include <cmath>
#include <cfloat>
#include <cstring>
#include "hnum.h"
using namespace std;

// This should be (std::numeric_limits<int>::digits10 + 1) for the sign
// However that is only a compile-time constant in C++11 or higher, so would
// need to commit to building as C++11.
//
// Hardcode as 11 for now, which is the maximum for 32-bit integers
//
#define MAX_CHARS_FOR_INT_IN_PRINTF 11

char prefixes[]="yzafpnum kMGTPEZY"; // u should be µ but that's two bytes
string hnum(double x)
{
  int exponent=0,subexp,mantissa,sign=1,i;
  string expstr,mantstr;
  char expbuf[MAX_CHARS_FOR_INT_IN_PRINTF + 1 + 1]; // +1 for terminator, +1 for e
  if (x<0)
  {
    x=-x;
    sign=-1;
  }
  if (x>0)
  {
    while (x>=1e12)
    {
      x/=1e12;
      exponent+=12;
    }
    if (x<1)
    {
      x*=1e6;
      exponent-=6;
    }
    while (x>=999.5)
    {
      x/=1e3;
      exponent+=3;
    }
  }
  if (x<9.995)
  {
    subexp=0;
    mantissa=rint(x*100);
  }
  else if (x<99.95)
  {
    subexp=1;
    mantissa=rint(x*10);
  }
  else
  {
    subexp=2;
    mantissa=rint(x);
  }
  mantstr=(char)(mantissa/100+'0');
  mantstr+=(char)((mantissa%100)/10+'0');
  mantstr+=(char)(mantissa%10+'0');
  mantstr.insert(subexp+1,".");
  if (sign<0)
    mantstr="-"+mantstr;
  while (mantstr.back()=='0')
    mantstr.pop_back();
  if (mantstr.back()=='.')
    mantstr.pop_back();
  if (exponent>=-24 && exponent<=24)
  {
    expstr=prefixes[exponent/3+8];
    if (expstr==" ")
      expstr="";
    if (expstr=="u")
      expstr="µ";
  }
  else
  {
    sprintf(expbuf,"e%d",exponent);
    expstr=expbuf;
  }
  return mantstr+expstr;
}
