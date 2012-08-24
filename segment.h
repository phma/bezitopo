/******************************************************/
/*                                                    */
/* segment.h - 3d line segment                        */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/

#include "point.h"

class segment
{
private:
  xyz start,end;
  double control1,control2;
public:
  segment();
  segment(xyz kra,xyz fam);
  double length();
};
