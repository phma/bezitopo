/******************************************************/
/*                                                    */
/* ldecimal.cpp - lossless decimal representation     */
/*                                                    */
/******************************************************/

#include <cstdio>
#include <cfloat>
#include "ldecimal.h"
using namespace std;

string ldecimal(double x)
{
  double x2;
  int i;
  string ret,mantissa,exponent;
  char buffer[32],fmt[8];
  for (i=DBL_DIG-1,x2=-x;x!=x2;i++)
  {
    sprintf(fmt,"%%.%de",i);
    sprintf(buffer,fmt,x);
    x2=atof(buffer);
  }
  ret=buffer;
  return ret;
}
