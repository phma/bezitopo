/******************************************************/
/*                                                    */
/* absorient.cpp - 2D absolute orientation            */
/*                                                    */
/******************************************************/
/* Given two pointlists and a list of matching points,
 * find the rotation and translation to match them with the
 * least sum of square distances. This is called the
 * absolute orientation problem.
 */
#include <assert.h>
#include "absorient.h"
#include "manysum.h"
using namespace std;

double sumsqdist(vector<xy> a,vector<xy> b)
{
  int i;
  manysum acc;
  assert(a.size()==b.size());
  for (i=0;i<a.size();i++)
    acc+=sqr(dist(a[i],b[i]));
  return acc.total();
}
