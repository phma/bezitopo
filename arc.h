/******************************************************/
/*                                                    */
/* arc.h - horizontal circular arcs                   */
/*                                                    */
/******************************************************/

#ifndef ARC_H
#define ARC_H
#include "segment.h"
#include "angle.h"
#include "point.h"

class arc: public segment
{
private:
  int delta; // angle subtended - "delta" is a surveying term
  double rchordbearing; // chordbearing in radians
public:
  arc();
  arc(xyz kra,xyz fam);
  arc(xyz kra,xyz mij,xyz fam);
  arc(xyz kra,xyz fam,int d);
  void setdelta(int d,int s=0);
  virtual double radius(double along) const
  {
    return chordlength()/sinhalf(delta)/2;
  }
  virtual double curvature(double along) const
  {
    return 2*sinhalf(delta)/chordlength();
  }
  virtual double diffarea();
  int startbearing() const
  {
    return chordbearing()-delta/2;
  }
  int endbearing() const
  {
    return chordbearing()+delta/2;
  }
  virtual int getdelta()
  {
    return delta;
  }
  virtual int getdelta2()
  {
    return 0;
  }
  double length() const;
  virtual xyz station(double along) const;
  virtual int bearing(double along);
  xy center();
  void split(double along,arc &a,arc &b);
  //xyz midpoint();
  };

#endif
