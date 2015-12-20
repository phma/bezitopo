/******************************************************/
/*                                                    */
/* cogospiral.cpp - intersections of spirals          */
/*                                                    */
/******************************************************/
#include "cogospiral.h"

using namespace std;

struct alosta
{
  double along;
  xy station;
  alosta(double a,xy s);
};

alosta::alosta(double a,xy s)
{
  along=a;
  station=s;
}

vector<double> intersection1(spiralarc a,double a1,double a2,spiralarc b,double b1,double b2,bool extend)
/* Returns two distances along, one for a and one for b, or nothing.
 * If extend is true, the spiralarcs are extended to twice their length;
 * e.g. one of length 5 extends from station -2.5 to station 7.5.
 */
{
  vector<alosta> aalosta,balosta;
  vector<double> ret;
  aalosta.emplace_back(a1,a.station(a1));
  aalosta.emplace_back(a2,a.station(a2));
  balosta.emplace_back(b1,b.station(b1));
  balosta.emplace_back(b2,b.station(b2));
  return ret;
}

