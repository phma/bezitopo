/******************************************************/
/*                                                    */
/*                                                    */
/*                                                    */
/******************************************************/

#include "segment.h"
#include "angle.h"
#include "point.h"

class arc: public segment
{
public:
  int delta; // angle subtended - "delta" is a surveying term
  double chord()
  {
    return dist(start,end);
  }
  double radius(double along)
  {
    return chord()/sinhalf(delta);
  }
  double curvature(double along)
  {
    return sinhalf(delta)/chord();
  }
  double length()
  {
    return chord()/cosquarter(delta);
  }
  xy center();
  //xyz midpoint();
  };
