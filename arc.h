/******************************************************/
/*                                                    */
/* arc.h - horizontal circular arcs                   */
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
  double radius(double along)
  {
    return chordlength()/sinhalf(delta)/2;
  }
  double curvature(double along)
  {
    return 2*sinhalf(delta)/chordlength();
  }
  double length();
  virtual xyz station(double along);
  xy center();
  //xyz midpoint();
  };
