/******************************************************/
/*                                                    */
/* cogo.h - coordinate geometry                       */
/*                                                    */
/******************************************************/

#ifndef COGO_H
#define COGO_H
#include "point.h"

#define sign(x) ((x>0)-(x<0))

extern int debugdel;

double area3(xy a,xy b,xy c);
xy intersection (xy a,xy c,xy b,xy d);
//Intersection of lines ab and cd.
int intersection_type(xy a,xy c,xy b,xy d);
double pldist(xy a,xy b,xy c);
// Signed distance from a to the line bc.
bool delaunay(xy a,xy c,xy b,xy d);
//Returns true if ac satisfies the criterion in the quadrilateral abcd.
//If false, the edge should be flipped to bd.
xy rand2p(xy a,xy b);
/* A random point in the circle with diameter ab. */

#endif
