/******************************************************/
/*                                                    */
/* cogo.h - coordinate geometry                       */
/*                                                    */
/******************************************************/

#ifndef COGO_H
#define COGO_H
#include "point.h"

#define sign(x) ((x>0)-(x<0))
#define IN_AT_CORNER 0x69969669
/* IN_AT_CORNER is set so that if you add a few of them, as results of in()
 * are computed by adding results of in() for smaller components, you will
 * get an obviously wrong answer.
 */

enum inttype {NOINT, ACXBD, BDTAC, ACTBD, ACVBD, COINC, COLIN, IMPOS};
extern int debugdel;
extern FILE *randfil;

double area3(xy a,xy b,xy c);
xy intersection (xy a,xy c,xy b,xy d);
//Intersection of lines ab and cd.
inttype intersection_type(xy a,xy c,xy b,xy d);
/* NOINT  don't intersect
   ACXBD  intersection is in the midst of both AC and BD
   BDTAC  one end of BD is in the midst of AC
   ACTBD  one end of AC is in the midst of BD
   ACVBD  one end of AC is one end of BD
   COINC  A=C or B=D
   COLIN  all four points are collinear
   IMPOS  impossible, probably caused by roundoff error
   */
int in3(xy p,xy a,xy b,xy c);
/* Returns twice the winding number of abc arount p.
 * If abc is counterclockwise and p is inside it, returns 2.
 * If p is on the boundary of abc, returns 1 or -1, depending on the orientation
 * of abc.
 * If abc is flat, returns 0.
 * If p is one of a, b, and c, and abc is not flat, returns IN_AT_CORNER,
 * which must be handled specially.
 */
double pldist(xy a,xy b,xy c);
// Signed distance from a to the line bc.
bool delaunay(xy a,xy c,xy b,xy d);
//Returns true if ac satisfies the criterion in the quadrilateral abcd.
//If false, the edge should be flipped to bd.
xy rand2p(xy a,xy b);
/* A random point in the circle with diameter ab. */
char *inttype_str(inttype i);

#endif
