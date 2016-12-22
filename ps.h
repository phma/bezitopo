/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013,2014,2015,2016 Pierre Abbat.
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
#ifndef PS_H
#define PS_H
#include <string>
#include <iostream>
#include <map>
#include "bezier3d.h"
class document;

struct papersize
{
  int width,height; // in micrometers
};
extern std::map<string,papersize> papersizes;

class PostScript
{
protected:
  std::ostream *psfile;
  int pages;
  bool indocument,inpage;
  double scale; // paper size is in millimeters, but model space is in meters
  int orientation,pageorientation;
  double oldr,oldg,oldb;
  xy paper,modelcenter;
  document *doc;
public:
  PostScript();
  ~PostScript();
  void setpaper(papersize pap,int ori);
  double aspectRatio();
  void open(std::string psfname);
  void prolog();
  void startpage();
  void endpage();
  void trailer();
  void close();
  void setDoc(document &docu);
  double xscale(double x);
  double yscale(double y);
  void setcolor(double r,double g,double b);
  void setscale(double minx,double miny,double maxx,double maxy,int ori=0);
  void dot(xy pnt,std::string comment="");
  void circle(xy pnt,double radius);
  void line(edge lin,int num,bool colorfibaster,bool directed=false);
  void line2p(xy pnt1,xy pnt2);
  void spline(bezier3d spl);
  void widen(double factor);
  void write(xy pnt,std::string text);
  void comment(string text);
};

#endif
