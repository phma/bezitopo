/******************************************************/
/*                                                    */
/*                                                    */
/*                                                    */
/******************************************************/

#include "angle.h"
#include "point.h"

class arc
{public:
 xy start,end;
 int delta; // angle subtended - "delta" is a surveying term
 double chord()
 {return dist(start,end);
  }
 double radius()
 {return chord()/sinhalf(delta);
  }
 double curvature()
 {return sinhalf(delta)/chord();
  }
 double length()
 {return chord()/cosquarter(delta);
  }
 xy center();
 xy midpoint();
 /*xy along(double length);*/
 /*xy nearest(xy pnt);*/
 };
