/******************************************************/
/*                                                    */
/* relprime.cpp - relatively prime numbers            */
/*                                                    */
/******************************************************/
#include <map>
#include <cmath>
#include "relprime.h"

using namespace std;

map<unsigned,unsigned> relprimes;

unsigned gcd(unsigned a,unsigned b)
{
  while (a&&b)
  {
    if (a>b)
    {
      b^=a;
      a^=b;
      b^=a;
    }
    b%=a;
  }
  return a+b;
}

unsigned relprime(unsigned n)
// Returns the integer closest to n/φ of those relatively prime to n.
{
  unsigned ret,twice;
  double phin;
  ret=relprimes[n];
  if (!ret)
  {
    phin=n*M_1PHI;
    ret=rint(phin);
    twice=2*ret-(ret>phin);
    while (gcd(ret,n)!=1)
      ret=twice-ret+(ret<=phin);
    relprimes[n]=ret;
  }
  return ret;
}
