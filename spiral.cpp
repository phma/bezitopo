/******************************************************/
/*                                                    */
/* spiral.cpp - Cornu or Euler spirals                */
/*                                                    */
/******************************************************/

/* The Cornu spiral is a complex-valued function of a real argument,
 * the graph in the complex plane of integral(cis(t²) dt).
 * Evaluating the function in its curly tails should not be necessary
 * in surveying, but if it is, methods of evaluating the
 * normal distribution function in its tail may be relevant.
 */

#include <vector>
#include <cmath>
#include <cstdio>
#include "spiral.h"
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
