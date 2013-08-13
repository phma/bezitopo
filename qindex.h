/******************************************************/
/*                                                    */
/* qindex.h - quad index to tin                       */
/*                                                    */
/******************************************************/

#include <vector>
#include "bezier.h"
#include "point.h"
class qindex
{
public:
  double x,y,side;
  union
  {
    qindex *sub[4]; // Either all four subs are set,
    triangle *tri;  // or tri alone is set, or they're all NULL.
  };
  triangle *findt(xy pnt);
  int quarter(xy pnt);
  xy middle();
  void sizefit(std::vector<xy> pnts);
  void split(std::vector<xy> pnts);
  void clear();
  void draw(bool root=true);
  std::vector<qindex*> traverse(int dir=0);
  qindex();
  ~qindex();
  int size(); // This returns the total number of nodes, which is 4n+1. The number of leaves is 3n+1.
};
