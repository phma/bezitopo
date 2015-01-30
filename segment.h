/******************************************************/
/*                                                    */
/* segment.h - 3d line segment                        */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/

#ifndef SEGMENT_H
#define SEGMENT_H
#include <cstdlib>
#include <vector>
#include "point.h"
#include "angle.h"
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
  segment(xyz kra,double c1,double c2,xyz fam);
  virtual double length();
  std::vector<double> vextrema(bool withends);
  void setslope(int which,double s);
  void setctrl(int which,double el);
  double elev(double along);
  double slope(double along);
  virtual xyz station(double along);
  double avgslope()
  {
    return (end.elev()-start.elev())/length();
  }
  double chordlength()
  {
    return dist(xy(start),xy(end));
  }
  int chordbearing()
  {
    return atan2i(xy(end)-xy(start));
  }
  virtual int startbearing()
  {
    return chordbearing();
  }
  virtual int endbearing()
  {
    return chordbearing();
  }
  double radius(double along)
  {
    return strtod("inf",NULL);
  }
  double curvature(double along)
  {
    return 0;
  }
  virtual double clothance(double along)
  {
    return 0;
  }
  xy center();
  xyz midpoint();
  void split(double along,segment &a,segment &b);
};
#endif
