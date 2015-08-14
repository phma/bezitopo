/******************************************************/
/*                                                    */
/* drawobj.cpp - drawing object base class            */
/*                                                    */
/******************************************************/

#include <cmath>
#include "drawobj.h"

bsph drawobj::boundsphere()
{
  bsph ret;
  ret.center=xyz(NAN,NAN,NAN);
  ret.radius=0;
  return ret;
}

bool drawobj::hittest(hline hitline)
{
  return false;
}

void drawobj::roscat(xy tfrom,int ro,double sca,xy tto)
/* Rotate, scale, and translate. Two xys are provided to accurately rotate
 * and scale something around a point, simultaneously rotate and translate, etc.
 * Does not affect the z scale. Always calls _roscat to avoid repeatedly
 * computing the sine and cosine of ro, except in the base drawobj class,
 * which has no location. 
 */
{
}
