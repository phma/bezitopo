/******************************************************/
/*                                                    */
/* spiral.cpp - Cornu or Euler spirals                */
/*                                                    */
/******************************************************/

/* The Cornu spiral is a complex-valued function of a real argument,
 * the graph in the complex plane of integral(cis(t²) dt).
 * The curvature at t equals 2t.
 * 
 * Traditionally spirals have been used mostly for transitions between
 * a straight section and a circular arc, i.e. at one end of every spiral
 * arc, t=0. Parkways, though, require a greater variety of spirals.
 * See http://www.fhwa.dot.gov/publications/publicroads/01septoct/spiral.cfm .
 * 
 * I originally wrote the one-argument cornu() function for spiral arcs
 * with one end straight, then wrote the three-argument cornu() function
 * for more general spiral arcs.
 */

#include <vector>
#include <cmath>
#include <cstdio>
#include <iostream>
#include "spiral.h"
#include "angle.h"
#include "vcurve.h"
using namespace std;

xy cornu(double t)
/* If |t|>=6, it returns the limit points rather than a value with no precision.
 * The largest t useful in surveying is 1.430067.
 */
{
  vector<long double> realparts,imagparts;
  int i;
  long double facpower,rsum,isum,t2,bigpart;
  double precision;
  t2=t*t;
  for (i=0,facpower=t;0.9+facpower!=0.9 || !i;i++)
  {
    realparts.push_back(facpower/(8*i+1));
    facpower*=t2/(4*i+1);
    imagparts.push_back(facpower/(8*i+3));
    facpower*=t2/(4*i+2);
    realparts.push_back(-facpower/(8*i+5));
    facpower*=t2/(4*i+3);
    imagparts.push_back(-facpower/(8*i+7));
    facpower*=t2/(4*i+4);
  }
  for (i=realparts.size()-1,rsum=isum=bigpart=0;i>=0;i--)
  {
    rsum+=realparts[i];
    isum+=imagparts[i];
    if (fabsl(realparts[i])>bigpart)
      bigpart=fabsl(realparts[i]);
    if (fabsl(imagparts[i])>bigpart)
      bigpart=fabsl(imagparts[i]);
  }
  precision=nextafterl(bigpart,2*bigpart)-bigpart;
  //printf("precision %e\n",precision);
  if (precision>1e-6)
    rsum=isum=sqrt(M_PI/8)*(t/fabs(t));
  return xy(rsum,isum);
}

xy cornu(double t,double curvature,double clothance)
/* Evaluates the integral of cis(clothance×t²/2+curvature×t).
 * 1+(cl×t²/2+cu×t)i-(cl×t²/2+cu×t)²/2-(cl×t²/2+cu×t)³i/6+(cl×t²/2+cu×t)⁴/24+...
 * 1+cl×t²×i/2  +cu×t×i   -cl²×t⁴/8  -cl×cu×t³×2/4  -cu²×t²/2  -cl³×t⁶×i/6/8  -cl²×cu×t⁵×3i/6/4  -cl×cu²×t⁴×3i/6/2  -cu³×t³i/6  +cl⁴×t⁸/24/16  +cl³×cu×t⁷×4/24/8  +cl²×cu²×t⁶6/24/4  +cl×cu³×t⁵×4/24/2  +cu⁴×t⁴/24+...
 * t+cl×t³×i/3/2+cu×t²×i/2-cl²×t⁵/5/8-cl×cu×t⁴×2/4/4-cu²×t³/3/2-cl³×t⁷×i/7/6/8-cl³×cu×t⁶×3i/6/6/4-cl×cu²×t⁵×3i/5/6/2-cu³×t⁴i/4/6+cl⁴×t⁹/9/24/16+cl³×cu×t⁸×4/8/24/8+cl²×cu²×t⁷6/7/24/4+cl×cu³×t⁶×4/6/24/2+cu⁴×t⁵/5/24+...
 * If clothance=0, you get a circle of radius 1/curvature.
 * If curvature=0 and clothance=2, you get cornu(t).
 */
{
  vector<long double> realparts,imagparts,cupower,clpower;
  int i,j;
  long double facpower,rsum,isum,t2,bigpart,binom,clotht,term,bigterm;
  double precision;
  t2=t*t;
  clotht=clothance*t/2;
  cupower.push_back(1);
  clpower.push_back(1);
  for (i=0,facpower=t;0.9+bigterm!=0.9 || !i;i++)
  {
    for (bigterm=j=0,binom=1;j<=i;j++)
    {
      term=clpower[j]*cupower[i-j]*binom*facpower/(i+j+1);
      //cout<<"i="<<i<<" j="<<j<<" term="<<cupower[j]<<'*'<<clpower[i-j]<<'*'<<binom<<'*'<<facpower<<'/'<<i+j+1<<'='<<term<<endl;
      if (fabsl(term)>bigterm)
	bigterm=fabsl(term);
      switch (i&3)
      {
	case 0:
	  realparts.push_back(term);
	  break;
	case 1:
	  imagparts.push_back(term);
	  break;
	case 2:
	  realparts.push_back(-term);
	  break;
	case 3:
	  imagparts.push_back(-term);
	  break;
      }
      binom=binom*(i-j)/(j+1);
    }
    cupower.push_back(cupower.back()*curvature);
    clpower.push_back(clpower.back()*clotht);
    facpower*=t/(i+1);
  }
  for (i=realparts.size()-1,rsum=bigpart=0;i>=0;i--)
  {
    rsum+=realparts[i];
    if (fabsl(realparts[i])>bigpart)
      bigpart=fabsl(realparts[i]);
  }
  for (i=imagparts.size()-1,isum=0;i>=0;i--)
  {
    isum+=imagparts[i];
    if (fabsl(imagparts[i])>bigpart)
      bigpart=fabsl(imagparts[i]);
  }
  precision=nextafterl(bigpart,2*bigpart)-bigpart;
  //printf("precision %e\n",precision);
  if (precision>1e-6)
    rsum=isum=nan("cornu");
  return xy(rsum,isum);
}

/* It should be possible to fit a spiral to be tangent to two given circular
 * or straight curves by successive approximation using these functions.
 */

double spiralbearing(double t,double curvature=0,double clothance=1)
{
  return t*t*clothance/2+t*curvature;
}

int ispiralbearing(double t,double curvature=0,double clothance=1)
{
  return radtobin(t*t*clothance/2+t*curvature);
}

double spiralcurvature(double t,double curvature=0,double clothance=1)
{
  return t*clothance+curvature;
}

spiralarc::spiralarc()
{
  mid=start=end=xyz(0,0,0);
  control1=control2=0;
  cur=clo=len=0;
  midbear=0;
}

spiralarc::spiralarc(xyz kra,xyz fam)
{
  start=kra;
  end=fam;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
  mid=(start+end)/2;
  len=dist(xy(start),xy(end));
  cur=clo=0;
  midbear=atan2i(xy(end-start));
}

xyz spiralarc::station(double along)
{
  double midlong;
  xy relpos;
  midlong=along-len/2;
  relpos=cornu(midlong,cur,clo);
  return xyz(turn(relpos,midbear)+mid,elev(along));
}

void spiralarc::_setdelta(int d,int s)
{
  cur=bintorad(d)/len;
  clo=4*bintorad(s)/len/len;
}

void spiralarc::_fixends()
{
  xy kra,fam;
  int turnangle;
  double scale;
  kra=station(0);
  fam=station(len);
  turnangle=atan2i(end-start)-atan2i(fam-kra);
  if (((unsigned)turnangle>>30)%3)
    turnangle^=0x80000000; // don't turn by more than 180° either way
  midbear+=turnangle;
  scale=dist(xy(end),xy(start))/dist(fam,kra);
  len*=scale;
  cur/=scale;
  clo/=scale*scale;
  kra=station(0);
  fam=station(len);
  mid+=((end-fam)+(start-kra))/2;
}

void spiralarc::split(double along,spiralarc &a,spiralarc &b)
{
  double dummy;
  xyz mida,midb;
  int midbeara,midbearb;
  double cura,curb;
  xyz splitpoint=station(along);
  mida=station(along/2);
  midb=station((along+len)/2);
  midbeara=bearing(along/2);
  midbearb=bearing((along+len)/2);
  cura=curvature(along/2);
  curb=curvature((along+len)/2);
  a.mid=mida;
  a.midbear=midbeara;
  a.cur=cura;
  a.len=along;
  a.start=start;
  b.mid=midb;
  b.midbear=midbearb;
  b.cur=curb;
  b.len=len-along;
  b.end=end;
  a.clo=b.clo=clo;
  a.end=b.start=splitpoint;
  vsplit(start.elev(),control1,control2,end.elev(),along/length(),a.control1,a.control2,dummy,b.control1,b.control2);
  printf("split: %f,%f\n",a.end.east(),a.end.north());
}

void spiralarc::setdelta(int d,int s)
{
  int lastmidbear,chordbear,rot,i;
  xy lastmid;
  chordbear=chordbearing();
  i=0;
  do
  {
    rot=2*(chordbear-midbear);
    lastmidbear=midbear;
    lastmid=mid;
    _setdelta(d,s+rot);
    _fixends();
    i++;
    //cout<<"iter "<<i<<" midbear "<<midbear<<endl;
  }
  while (abs(midbear-lastmidbear)>1 || dist(mid,lastmid)>1e-6);
}
