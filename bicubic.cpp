/******************************************************/
/*                                                    */
/* bicubic.cpp - bicubic interpolation                */
/*                                                    */
/******************************************************/
#include <cstring>
#include "bicubic.h"
using namespace std;

double beziersquare(array<double,16> controlPoints,double x,double y)
{
  int i,j;
  double xhill[4],yhill[4],xn,yn,isum[4],ret;
  xn=1-x;
  yn=1-y;
  xhill[0]=xn*xn*xn;
  xhill[1]=3*xn*xn*x;
  xhill[2]=3*xn*x*x;
  xhill[3]=x*x*x;
  yhill[0]=yn*yn*yn;
  yhill[1]=3*yn*yn*y;
  yhill[2]=3*yn*y*y;
  yhill[3]=y*y*y;
  memset(isum,0,sizeof(isum));
  for (i=0;i<4;i++)
    for (j=0;j<4;j++)
      isum[i^j]+=controlPoints[(i<<2)+j]*yhill[j]*xhill[i];
  for (ret=i=0;i<4;i++)
    ret+=isum[i];
  return ret;
}

double bicubic(double swelev,xy swslope,double seelev,xy seslope,
	       double nwelev,xy nwslope,double neelev,xy neslope,
	       double x,double y)
// The slopes assume a unit square.
{
  array<double,16> controlPoints;
  controlPoints[ 0]=swelev;
  controlPoints[ 3]=seelev;
  controlPoints[12]=nwelev;
  controlPoints[15]=neelev;
  controlPoints[ 1]=swelev+swslope.getx()/3;
  controlPoints[ 4]=swelev+swslope.gety()/3;
  controlPoints[ 2]=seelev-seslope.getx()/3;
  controlPoints[ 7]=seelev+seslope.gety()/3;
  controlPoints[13]=nwelev+nwslope.getx()/3;
  controlPoints[ 8]=nwelev-nwslope.gety()/3;
  controlPoints[14]=neelev-neslope.getx()/3;
  controlPoints[11]=neelev-neslope.gety()/3;
  controlPoints[ 5]=controlPoints[ 1]+controlPoints[ 4]-controlPoints[ 0];
  controlPoints[ 6]=controlPoints[ 2]+controlPoints[ 7]-controlPoints[ 3];
  controlPoints[ 9]=controlPoints[13]+controlPoints[ 8]-controlPoints[12];
  controlPoints[10]=controlPoints[14]+controlPoints[11]-controlPoints[15];
  return beziersquare(controlPoints,x,y);
}
