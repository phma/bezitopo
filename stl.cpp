/******************************************************/
/*                                                    */
/* stl.cpp - stereolithography (3D printing) export   */
/*                                                    */
/******************************************************/

#include "stl.h"

/* The STL polyhedron consists of three kinds of face: bottom, side, and top.
 * The bottom is a convex polygon which is triangulated. The sides are
 * trapezoids, each of which is drawn as two triangles. The top is the
 * TIN surface. Each edge is split into some number of pieces which is a
 * power of 2, enough to make it smooth at the printing scale, then some
 * are split into more pieces because every triangle must have one side that
 * is split into a number of pieces that is a multiple of the number of
 * pieces that the other two sides are split into, which must be equal.
 */

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
