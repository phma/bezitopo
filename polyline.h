/******************************************************/
/*                                                    */
/* polyline.h - polylines                             */
/*                                                    */
/******************************************************/

#include <vector>
#include "point.h"
#include "arc.h"

/* Polylines and alignments are very similar. The differences are:
 * polylines are in a horizontal plane, whereas alignments have vertical curves;
 * polylines consist of arcs, whereas alignments can have spirals (once I figure out how).
 */

class polyline
{
private:
  double elevation;
  std::vector<xy> endpoints;
  std::vector<int> deltas;
  std::vector<double> lengths;
public:
  bool isopen();
  arc getarc(int i);
  void insert(xy newpoint,int pos=-1);
  void setlengths();
  void setdelta(int i,int delta);
  void open();
  void close();
  double length();
  double area();
};

class alignment
{
public:
  std::vector<xyz> endpoints;
  std::vector<int> deltas;
  std::vector<double> lengths;
};
