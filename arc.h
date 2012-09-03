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
private:
  int delta; // angle subtended - "delta" is a surveying term
public:
  arc();
  arc(xyz kra,xyz fam);
  void setdelta(int d);
  double chord()
  {
    return dist(xy(start),xy(end));
  }
  double radius(double along)
  {
    return chord()/sinhalf(delta)/2;
  }
  double curvature(double along)
  {
    return 2*sinhalf(delta)/chord();
  }
  double length();
  xy center();
  //xyz midpoint();
  };
