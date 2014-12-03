/* bezier3d.h
 */
#include <vector>
#include "point.h"

class bezier3d
{
private:
  std::vector<xyz> controlpoints;
public:
  bezier3d(xyz kra,xyz con1,xyz con2,xyz fam);
  bezier3d(xyz kra,int bear0,double slp0,double slp1,int bear1,xyz fam);
  bezier3d();
  int size(); // number of Bézier segments
  xyz station(double along);
  //friend bezier3d operator*(const bezier3d &l,const bezier3d &r); // concatenates, not multiplies
  friend bezier3d operator+(const bezier3d &l,const bezier3d &r);
};
