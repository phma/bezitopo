/******************************************************/
/*                                                    */
/* vcurve.cpp - vertical curve arithmetic             */
/*                                                    */
/******************************************************/

/* A vertical curve, in surveying, is a small-degree polynomial used for
 * the ups and downs of roads. Normally a quadratic is used, but this is
 * Bezitopo, so it uses a Bézier spline, which is a cubic. Of course
 * any quadratic can be expressed as a cubic with the cube term zero.
 */
#include <cmath>
#include <cstdio>
#include "vcurve.h"

using namespace std;

double vcurve(double a,double b,double c,double d,double p)
{
  double q=1-p;
  double offset,result;
  p=1-q;
  result=a*q*q*q+3*b*p*q*q+3*c*p*p*q+d*p*p*p;
  return result;
}

double vslope(double a,double b,double c,double d,double p)
{
  double q=1-p;
  double offset,result;
  p=1-q;
  result=-3*a*q*q+3*b*(q-2*p)*q-3*c*p*(p-2*q)+3*d*p*p;
  return result;
}

double vaccel(double a,double b,double c,double d,double p)
{
  double q=1-p;
  double offset,result;
  p=1-q;
  result=6*a*q-6*b*(2*q-p)-6*c*(2*p-q)+6*d*p;
  return result;
}

double vlength_estimate(double a,double b,double c,double d,double hlength,int n)
{
  double sum,prev,curr;
  int i;
  for (sum=i=0;i<=n;i++)
  {
    prev=curr;
    curr=vcurve(a,b,c,d,(double)i/n);
    if (i)
      sum+=hypot(prev-curr,hlength/n);
  }
  return sum;
}

double vlength(double a,double b,double c,double d,double hlength)
{
  int n,prevn=0;
  double currestimate=-1,prevestimate=-2,currextrapolate=-1,prevextrapolate=-2;
  for (n=1;n>0 && fabs(currestimate-prevestimate)*16777216>(currestimate+prevestimate);n+=n+1)
  {
    prevestimate=currestimate;
    prevextrapolate=currextrapolate;
    currestimate=vlength_estimate(a,b,c,d,hlength,n);
    currextrapolate=(currestimate*n*n-prevestimate*prevn*prevn)/(n*n-prevn*prevn);
    //printf("%d %17.13f %17.13f\n",n,currestimate,currextrapolate);
    prevn=n;
  }
  return currextrapolate;
}

vector<double> vextrema(double a,double b,double c,double d)
/* This is used for two quite different purposes:
 * 1. to place storm sewer grates at the lowest point along a street
 * 2. to divide sides of a triangle into monotonic parts and decide which of
 *    two crossing segments to delete when subdividing a triangle.
 * For 1, extrema at the endpoints count. For 2, they do not. Extrema
 * at the endpoints must therefore be placed exactly at the endpoints.
 */
{
  double slopes[3],quad,vertex,minslope,root,discriminant;
  vector<double> ext;
  slopes[0]=vslope(a,b,c,d,0);
  slopes[1]=vslope(a,b,c,d,0.5);
  slopes[2]=vslope(a,b,c,d,1);
  quad=slopes[0]+slopes[2]-slopes[1]*2;
  minslope=fabs(slopes[0]);
  if (minslope>fabs(slopes[1]))
    minslope=fabs(slopes[1]);
  if (minslope>fabs(slopes[2]))
    minslope=fabs(slopes[2]);
  if (quad) //TODO: improve numerical stability in the case that one root is in [0,1] and the other is far away
  {
    if (minslope==0 && slopes[1]!=0)
    {
      if (slopes[0]==0 && slopes[2]==0)
      {
	ext.push_back(0);
	ext.push_back(1);
      }
      else
      {
	vertex=0.5-(slopes[2]-slopes[0])*0.25/quad;
	if (slopes[0]==0)
	{
	  ext.push_back(0);
	  if (vertex<0.5 && vertex>0)
	    ext.push_back(2*vertex);
	}
	if (slopes[2]==0)
	{
	  if (vertex>0.5 && vertex<1)
	    ext.push_back(2*vertex-1);
	  ext.push_back(1);
	}
      }
    }
    else if (minslope<=fabs(quad)/4 || slopes[0]*slopes[2]<=0)
    {
      vertex=0.5-(slopes[2]-slopes[0])*0.25/quad;
      discriminant=-vslope(a,b,c,d,vertex)/quad/2;
      if (discriminant>=0)
      {
        root=vertex-sqrt(discriminant);
	if (root>=0 && root<=1)
	  ext.push_back(root);
	root=vertex+sqrt(discriminant);
	if (root>=0 && root<=1 && discriminant)
	  ext.push_back(root);
      }
    }
  }
  else // not a quadratic
  {
    if (slopes[0]*slopes[2]<=0)
      ext.push_back(slopes[0]/(slopes[0]-slopes[2]));
  } 
  return ext;
}

void vsplit(double a,double b,double c,double d,double p,
	    double &b1,double &c1,double &d1a2,double &b2,double &c2)
{
  double q=1-p;
  double s=vslope(a,b,c,d,p);
  p=1-q;
  b1=a+(b-a)*p;
  d1a2=vcurve(a,b,c,d,p);
  c1=d1a2-p*s/3;
  b2=d1a2+q*s/3;
  c2=d+(c-d)*q;
}
