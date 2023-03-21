/******************************************************/
/*                                                    */
/* ldecimal.cpp - lossless decimal representation     */
/*                                                    */
/******************************************************/
/* Copyright 2015,2017,2019,2020,2023 Pierre Abbat.
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
#include <cfloat>
#include <cstring>
#include <cmath>
#include <cassert>
#include <clocale>
#include "ldecimal.h"
using namespace std;

string ldecimal(double x,double toler,bool noexp)
/* Returns a string which represents x in decimal to within toler and is
 * as short as possible. If toler is 0, the result of converting
 * the string back to double is exactly x. If noexp is true, returns
 * the shortest string which represents x without exponential notation.
 */
{
  double x2;
  int h,i,iexp,chexp;
  size_t zpos;
  char *dotpos,*epos,*pLcNumeric;
  string ret,s,m,antissa,exponent,saveLcNumeric;
  char buffer[32],fmt[8];
  assert(toler>=0);
  pLcNumeric=setlocale(LC_NUMERIC,nullptr);
  if (pLcNumeric)
    saveLcNumeric=pLcNumeric;
  setlocale(LC_NUMERIC,"C");
  if (toler>0 && x!=0)
  {
    iexp=floor(log10(fabs(x/toler))-1);
    if (iexp<0)
      iexp=0;
  }
  else
    iexp=DBL_DIG-1;
  if (iexp>DBL_DIG)
    iexp=DBL_DIG+1;
  h=-1;
  i=iexp;
  while (true)
  {
    sprintf(fmt,"%%.%de",i);
    sprintf(buffer,fmt,x);
    x2=atof(buffer);
    if (h>0 && (fabs(x-x2)<=toler || i>=DBL_DIG+3))
      break;
    // GCC atof("inf")==0. MSVC atof("inf")=INFINITY.
    if (fabs(x-x2)>toler || std::isnan(x-x2) || i<=0)
      h=1;
    i+=h;
  }
  dotpos=strchr(buffer,'.');
  epos=strchr(buffer,'e');
  if (epos && !dotpos) // e.g. 2e+00 becomes 2.e+00
  {
    memmove(epos+1,epos,buffer+31-epos);
    dotpos=epos++;
    *dotpos='.';
  }
  if (dotpos && epos)
  {
    m=string(buffer,dotpos-buffer);
    antissa=string(dotpos+1,epos-dotpos-1);
    exponent=string(epos+1);
    if (m.length()>1)
    {
      s=m.substr(0,1);
      m.erase(0,1);
    }
    zpos=antissa.find_last_not_of('0');
    antissa.erase(zpos+1);
    iexp=stoi(exponent);
    if (iexp<0 && (noexp || iexp>-5))
    {
      antissa=m+antissa;
      m="";
      iexp++;
    }
    if (iexp>0)
    {
      chexp=iexp;
      if (chexp>antissa.length())
	chexp=antissa.length();
      m+=antissa.substr(0,chexp);
      antissa.erase(0,chexp);
      iexp-=chexp;
    }
    while ((noexp || iexp>-5) && iexp<0 && m.length()==0)
    {
      antissa="0"+antissa;
      iexp++;
    }
    while ((noexp || iexp<3) && iexp>0 && antissa.length()==0)
    {
      m+='0';
      iexp--;
    }
    sprintf(buffer,"%d",iexp);
    exponent=buffer;
    ret=s+m;
    if (antissa.length())
      ret+='.'+antissa;
    if (iexp)
      ret+='e'+exponent;
  }
  else
    ret=buffer;
  setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  return ret;
}
