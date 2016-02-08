/******************************************************/
/*                                                    */
/* bicubic.cpp - bicubic interpolation                */
/*                                                    */
/******************************************************/
#include <cstring>
#include "bicubic.h"

double beziersquare(array<double,16> controlPoints,double x,double y)
{
  int i,j;
  double xhill[4],yhill[4],xn,yn,isum[4],ret;
  xn=1-x;
  yn=1-y;
  xhill[0]=xn*xn*xn;
  xhill[1]=xn*xn*x;
  xhill[2]=xn*x*x;
  xhill[3]=x*x*x;
  yhill[0]=yn*yn*yn;
  yhill[1]=yn*yn*y;
  yhill[2]=yn*y*y;
  yhill[3]=y*y*y;
  memset(isum,0,sizeof(isum));
  for (i=0;i<4;i++)
    for (j=0;j<4;j++)
      isum[i^j]+=controlPoints[(i<<2)+j]*yhill[j]*xhill[i];
  for (ret=i=0;i<4;i++)
    ret+=isum[i];
  return ret;
}
