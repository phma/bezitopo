/******************************************************/
/*                                                    */
/* pointlist.h - list of points                       */
/*                                                    */
/******************************************************/
/* Copyright 2012-2017 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef POINTLIST_H
#define POINTLIST_H

#include <map>
#include <string>
#include <vector>
#include <array>
#include "point.h"
#include "tin.h"
#include "bezier.h"
#include "qindex.h"
#include "polyline.h"
#include "contour.h"
#include "breakline.h"

typedef std::map<int,point> ptlist;
typedef std::map<point*,int> revptlist;

class criterion
{
public:
  criterion();
  bool match(point &pnt,int num);
  std::string str;
  int lo,hi; // point number range
  double elo,ehi; // elevation range
  bool istopo;
};

typedef std::vector<criterion> criteria;
/* The last criterion that matches the point determines whether the point
 * is included in the topo. If none matches, it is not included.
 */

class pointlist
{
private:
  std::vector<segment> break0;
public:
  ptlist points;
  revptlist revpoints;
  std::map<int,edge> edges;
  std::map<int,triangle> triangles;
  /* edges and triangles are logically arrays from 0 to size()-1, but are
   * implemented as maps, because they have pointers to each other, and points
   * point to edges, and the pointers would be messed up by moving memory
   * when a vector is resized.
   */
  std::vector<polyspiral> contours;
  criteria crit;
  ContourInterval contourInterval;
  std::vector<Breakline0> type0Breaklines;
  std::vector<std::vector<xy> > type1Breaklines;
  qindex qinx;
  void addpoint(int numb,point pnt,bool overwrite=false);
  void clear();
  int size();
  void clearmarks();
  void clearTin();
  bool checkTinConsistency();
  int readCriteria(std::string fname);
  void setgradient(bool flat=false);
  void findedgecriticalpts();
  void findcriticalpts();
  void addperimeter();
  void removeperimeter();
  triangle *findt(xy pnt,bool clip=false);
  bool join2break0();
  void joinBreaklines();
  void edgesToBreaklines();
  void stringToBreakline(std::string line); // for import, with hyphens
  std::string hitTestString(triangleHit hit);
  std::string hitTestPointString(xy pnt,double radius);
  virtual void writeXml(std::ofstream &ofile);
  // the following methods are in tin.cpp
private:
  void dumpedges();
  void dumpnext_ps(PostScript &ps);
public:
  void dumpedges_ps(PostScript &ps,bool colorfibaster);
  void splitBreaklines();
  int checkBreak0(edge &e);
  bool shouldFlip(edge &e);
  bool tryStartPoint(PostScript &ps,xy &startpnt);
  int flipPass(PostScript &ps,bool colorfibaster);
  void maketin(std::string filename="",bool colorfibaster=false);
  void makegrad(double corr);
  void maketriangles();
  void makeqindex();
  void updateqindex();
  double elevation(xy location);
  double dirbound(int angle);
  std::array<double,2> lohi();
  virtual void roscat(xy tfrom,int ro,double sca,xy tto); // rotate, scale, translate
};

#endif
