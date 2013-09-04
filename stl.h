/******************************************************/
/*                                                    */
/* stl.h - stereolithography (3D printing) export     */
/*                                                    */
/******************************************************/

#include "point.h"
#include "config.h"

struct stltriangle
{
  xyz normal,a,b,c;
  string attributes;
  stltriangle();
  stltriangle(xyz A,xyz B,xyz C);
};