/******************************************************/
/*                                                    */
/* geoid.h - geoidal undulation                       */
/*                                                    */
/******************************************************/
/* The native format is a set of six quadtrees, one each for Arctic, Antarctic,
 * Benin, Galápagos, Howland, and Bengal. Each leaf square is a set of six
 * values for the constant, linear, and quadratic components.
 */
#include <vector>
#include <array>
#include "xyz.h"
#include "ellipsoid.h"

struct vball // so called because a sphere so marked looks like a volleyball
{
  int face;
  double x,y;
  vball();
  vball(int f,xy p);
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
  xy center;
  float scale; // always a power of 2
  int face;
  std::vector<xy> nans,nums;
  bool subdivided();
  geoquad();
  ~geoquad();
  void clear();
  void subdivide();
  bool in(xy pnt); // does not check whether it's on the right face
  bool in(vball pnt); // does check
  double undulation(double x,double y);
  xyz centeronearth();
  double length(); // length, width, and area are accurate only for small squares
  double width(); // and ignore the orientation of the square relative to the
  double area(); // displacement from the center of the face.
  int isfull(); // -1 if empty, 0 if partly full or unknown, 1 if full
  std::array<unsigned,2> hash();
};
