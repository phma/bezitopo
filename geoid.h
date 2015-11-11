/******************************************************/
/*                                                    */
/* geoid.h - geoidal undulation                       */
/*                                                    */
/******************************************************/
/* The native format is a set of six quadtrees, one each for Arctic, Antarctic,
 * Benin, Galápagos, Howland, and Bengal. Each leaf square is a set of six
 * values for the constant, linear, and quadratic components.
 */
#include "xyz.h"
#include "ellipsoid.h"

struct vball // so called because a sphere so marked looks like a volleyball
{
  int face;
  double x,y;
};

vball encodedir(xyz dir);
xyz decodedir(vball code);

class geoquad
{
public:
  union
  {
    geoquad *sub[4];
    int und[6];
  };
  bool subdivided();
  geoquad();
  ~geoquad();
  void subdivide();
  double undulation(double x,double y);
};
