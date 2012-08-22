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
#include "vcurve.h"

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
