/******************************************************/
/*                                                    */
/* point.h - classes for points and gradients         */
/*                                                    */
/******************************************************/

#ifndef POINT_H
#define POINT_H
#include <string>
#include "xyz.h"
#include "drawobj.h"
//#include "tin.h"
// Declaration of xy and xyz is in xyz.h.

class point;
class edge;
class triangle;
class drawobj;
class document;

using std::string;

extern const xy beforestart,afterend;
/* Used to answer segment::nearpnt if the closest point is the start or end
 * and the line isn't normal to the segment/arc/spiralarc
 */

/*class farna: public xy
{
 private:
 void normalize();
 }

farna perp(farna dir);*/

class point: public xyz, public drawobj
{
public:
  using xyz::roscat;
  //xy pagepos; //used when dumping a lozenge in PostScript
  point();
  point(double e,double n,double h,string desc);
  point(xy pnt,double h,string desc);
  point(const point &rhs);
  //~point();
  const point& operator=(const point &rhs);
  //void setedge(point *oend);
  //void dump(document doc);
  friend class edge;
  friend void maketin(string filename);
  friend void rotate(document &doc,int n);
  friend void movesideways(document &doc,double sw);
  friend void moveup(document &doc,double sw);
  friend void enlarge(document &doc,double sw);
  short flags;
  /* 0: a normal point from the input file
   * 1: a point resulting from merging several points
   * 2: an explicitly ignored point
   * 3: a point ignored because it's in a group that was merged
   */
  string note;
  edge *line; // a line incident on this point in the TIN. Used to arrange the lines in order around their endpoints.
  edge *edg(triangle *tri);
  // tri.a->edg(tri) is the side opposite tri.b
  public:
  xy newgradient,gradient,oldgradient; // may be redone later, but for now a point has one gradient, and type 0 breaklines merely force edges to be in the TIN.
  int valence();
};
#endif
