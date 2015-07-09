/******************************************************/
/*                                                    */
/* tin.h - triangulated irregular network             */
/*                                                    */
/******************************************************/

#ifndef TIN_H
#define TIN_H
#include <exception>
#include <map>
#include <vector>
#include <string>
#include "cogo.h"
#include "bezier.h"
//#include "bezitopo.h"
#include "pointlist.h"
#include "segment.h"

using std::exception;
class pointlist;

class edge
{
public:
  point *a,*b;
  edge *nexta,*nextb;
  triangle *tria,*trib;
  double extrema[2];
  bool broken; //true if a breakline crosses this edge
  char contour;
  /* When drawing a contour, set one of the low 3 bits of edge::contour to true
   * for each edge that crosses the contour. Keep the flags set when you go to
   * the next contour of the same elevation. When you go to the next elevation,
   * clear the flags.
   */
  char stlsplit;
  /* Log base 2 of the number of pieces this edge is split into
   * when writing an STL file.
   */
  void flip(pointlist *topopoints);
  point* otherend(point* end);
  triangle* othertri(triangle* t);
  edge* next(point* end);
  triangle* tri(point* end);
  xy midpoint();
  void setnext(point* end,edge* enext);
  bool isinterior();
  bool delaunay();
  void dump(pointlist *topopoints);
  double length();
  segment getsegment();
  xyz critpoint(int i);
  void findextrema();
  void clearmarks();
  void mark(int n);
  bool ismarked(int n);
};

typedef std::pair<double,point*> ipoint;
/*
class notriangle: public exception
{virtual const char* what() const throw()
   {return "Must have at least three points to define a surface.";
    }
 } notri;

class samepoints: public exception
{virtual const char* what() const throw()
   {return "Two of the points are the same.";
    }
 } samepnts;
*/
#define notri 1
#define samepnts 2
#define flattri 3

#endif
