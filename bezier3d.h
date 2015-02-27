/* bezier3d.h
 */

#ifndef BEZIER3D_H
#define BEZIER3D_H
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
  int size() const; // number of Bézier segments
  std::vector<xyz> operator[](int n);
  xyz station(double along);
  friend bezier3d operator+(const bezier3d &l,const bezier3d &r); // concatenates, not adds
};

double bez3destimate(xy kra,int bear0,double len,int bear1,xy fam);
#endif
