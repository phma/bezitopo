/******************************************************/
/*                                                    */
/* ldecimal.cpp - lossless decimal representation     */
/*                                                    */
/******************************************************/

#include <cstdio>
#include <cfloat>
#include <cstring>
#include "ldecimal.h"
using namespace std;

string ldecimal(double x)
{
  double x2;
  int i,iexp;
  size_t zpos;
  char *dotpos,*epos;
  string ret,m,antissa,exponent;
  char buffer[32],fmt[8];
  for (i=DBL_DIG-1,x2=-1/x;x!=x2 && i<DBL_DIG+3;i++)
  {
    sprintf(fmt,"%%.%de",i);
    sprintf(buffer,fmt,x);
    x2=atof(buffer);
  }
  dotpos=strchr(buffer,'.');
  epos=strchr(buffer,'e');
  if (dotpos&&epos)
  {
    m=string(buffer,dotpos-buffer);
    antissa=string(dotpos+1,epos-dotpos-1);
    exponent=string(epos+1);
    iexp=atoi(exponent.c_str());
    zpos=antissa.find_last_not_of('0');
    antissa.erase(zpos+1);
    ret=m+'.'+antissa+'e'+exponent;
  }
  else
    ret=buffer;
  return ret;
}
