/* bezier3d.cpp
 * 3d Bézier splines, used for approximations to spirals and arcs for display.
 * Of course the 3d approximation of a vertical curve is exact.
 */
#include <cmath>
#include <stdexcept>
#include "bezier3d.h"
#include "angle.h"
using namespace std;

bezier3d::bezier3d(xyz kra,xyz con1,xyz con2,xyz fam)
{
  controlpoints.push_back(kra);
  controlpoints.push_back(con1);
  controlpoints.push_back(con2);
  controlpoints.push_back(fam);
}

bezier3d::bezier3d(xyz kra,int bear0,double slp0,double slp1,int bear1,xyz fam)
{
  double len0,len1,len,corr0,corr1,corr2;
  int direc;
  len=dist(xy(kra),xy(fam));
  direc=dir(xy(kra),xy(fam));
  corr0=1-cos(bear0-direc);
  corr1=1-cos(bear1-direc);
  corr2=1-cos(bear0+bear1-2*direc); //this is 0 for circular curves
  len0=len/3.*(1+corr0/2-corr2/24);
  len1=len/3.*(1+corr1/2-corr2/24);
  controlpoints.push_back(kra);
  controlpoints.push_back(kra+xyz(cossin(bear0),slp0)*len0);
  controlpoints.push_back(fam-xyz(cossin(bear1),slp1)*len1);
  controlpoints.push_back(fam);
}

bezier3d::bezier3d()
{
  controlpoints.push_back(xyz(0,0,0));
}

int bezier3d::size()
{
  return controlpoints.size()/3;
}

xyz bezier3d::station(double along)
{
  int segment;
  double p,q;
  xyz result;
  segment=floor(along);
  if (segment==size())
    segment--;
  if (segment<0 || segment>=size())
    throw(range_error("bezier3d::station: along out of range"));
  p=along-segment;
  q=1-p;
  p=1-q;
  result=controlpoints[3*segment]*q*q*q+3*controlpoints[3*segment+1]*p*q*q+3*controlpoints[3*segment+2]*p*p*q+controlpoints[3*segment+3]*p*p*p;
  return result;
}
