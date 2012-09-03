/******************************************************/
/*                                                    */
/* segment.h - 3d line segment                        */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/

#ifndef SEGMENT_H
#define SEGMENT_H
#include <cstdlib>
#include "point.h"
#define START 1
#define END 2

class segment
{
protected:
  xyz start,end;
  double control1,control2;
public:
  segment();
  segment(xyz kra,xyz fam);
  double length();
  void setslope(int which,double s);
  double elev(double along);
  double slope(double along);
  xyz station(double along);
  double chord()
  {
    return dist(xy(start),xy(end));
  }
  double radius(double along)
  {
    return strtod("inf",NULL);
  }
  double curvature(double along)
  {
    return 0;
  }
  xy center();
  xyz midpoint();
  void split(double along,segment &a,segment &b);
};
#endif
