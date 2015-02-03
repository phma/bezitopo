/******************************************************/
/*                                                    */
/* spiral.h - Cornu or Euler spirals                  */
/*                                                    */
/******************************************************/

#include "point.h"
#include "segment.h"

xy cornu(double t);
xy cornu(double t,double curvature,double clothance);
double spiralbearing(double t,double curvature,double clothance);
int ispiralbearing(double t,double curvature,double clothance);
double spiralcurvature(double t,double curvature,double clothance);

class spiralarc: public segment
/* station() ignores the x and y coordinates of start and end.
 * mid, midbear, etc. must be set so that the station() values
 * match start and end. This can take several iterations.
 */
{
private:
  xy mid;
  double cur,clo,len;
  int midbear;
public:
  spiralarc();
  spiralarc(xyz kra,xyz fam);
  spiralarc(xyz kra,double c1,double c2,xyz fam);
  double length()
  {
    return len;
  }
};
