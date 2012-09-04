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
  arc(xyz kra,xyz fam,int d);
  void setdelta(int d);
  double radius(double along)
  {
    return chordlength()/sinhalf(delta)/2;
  }
  double curvature(double along)
  {
    return 2*sinhalf(delta)/chordlength();
  }
  int startbearing()
  {
    return chordbearing()-delta/2;
  }
  int endbearing()
  {
    return chordbearing()+delta/2;
  }
  double length();
  virtual xyz station(double along);
  xy center();
  void split(double along,arc &a,arc &b);
  //xyz midpoint();
  };
