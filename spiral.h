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
 *
 * d and s: d is the bearing at the end - the bearing at the start.
 * s is the sum of the start and end bearings - twice the chord bearing
 * (in setdelta) or - twice the midpoint bearing (in _setdelta).
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
  virtual int bearing(double along)
  {
    return midbear+ispiralbearing(along-len/2,cur,clo);
  }
  void _setdelta(int d,int s=0);
};
