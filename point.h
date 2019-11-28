/******************************************************/
/*                                                    */
/* point.h - classes for points and gradients         */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014-2019 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
class pointlist;
class document;

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
/* No render3d method. A point is drawn as three concentric circles when
 * there are no edges and you're viewing a pointlist.
 */
{
public:
  using xyz::roscat;
  using xyz::_roscat;
  //xy pagepos; //used when dumping a lozenge in PostScript
  point();
  point(double e,double n,double h,std::string desc);
  point(xy pnt,double h,std::string desc);
  point(xyz pnt,std::string desc);
  point(const point &rhs);
  //~point();
  const point& operator=(const point &rhs);
  //void setedge(point *oend);
  //void dump(document doc);
  virtual bool hasProperty(int prop);
  virtual void writeXml(std::ofstream &ofile,pointlist &pl);
  friend class edge;
  friend void maketin(std::string filename);
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
  std::string note;
  edge *line; // a line incident on this point in the TIN. Used to arrange the lines in order around their endpoints.
  edge *edg(triangle *tri);
  // tri.a->edg(tri) is the side opposite tri.b
public:
  xy newgradient,gradient,oldgradient; // may be redone later, but for now a point has one gradient, and type 0 breaklines merely force edges to be in the TIN.
  int valence();
  std::vector<edge *> incidentEdges();
  edge *isNeighbor(point *pnt);
  void insertEdge(edge *edg);
};
#endif
