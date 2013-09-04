/******************************************************/
/*                                                    */
/* stl.cpp - stereolithography (3D printing) export   */
/*                                                    */
/******************************************************/

#include "stl.h"

stltriangle::stltriangle()
{
  normal=a=b=c=xyz(0,0,0);
}

stltriangle::stltriangle(xyz A,xyz B,xyz C)
{
  a=A;
  b=B;
  c=C;
}
