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
protected:
  double elevation;
  std::vector<xy> endpoints;
  std::vector<double> lengths;
public:
  polyline();
  bool isopen();
  segment getsegment(int i);
  virtual void insert(xy newpoint,int pos=-1);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double length();
  virtual double area();
};

class polyarc: public polyline
{
protected:
  std::vector<int> deltas;
public:
  arc getarc(int i);
  virtual void insert(xy newpoint,int pos=-1);
  void setdelta(int i,int delta);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  //virtual double length();
  virtual double area();
};

class alignment
{
public:
  std::vector<xyz> endpoints;
  std::vector<int> deltas;
  std::vector<double> lengths;
};
