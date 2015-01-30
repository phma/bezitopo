#ifndef POINTLIST_H
#define POINTLIST_H

#include <map>
#include <string>
#include <vector>
#include "point.h"
#include "tin.h"
#include "bezier.h"
#include "qindex.h"

typedef std::map<int,point> ptlist;
typedef std::map<point*,int> revptlist;
class pointlist
{
public:
  ptlist points;
  revptlist revpoints;
  std::map<int,edge> edges;
  std::map<int,triangle> triangles;
  qindex qinx;
  void addpoint(int numb,point pnt,bool overwrite=false);
  void clear();
  void setgradient(bool flat=false);
  void findcriticalpts();
  // the following methods are in tin.cpp
private:
  void dumpedges();
  void dumpedges_ps(bool colorfibaster);
  void dumpnext_ps();
public:
  void maketin(string filename="",bool colorfibaster=false);
  void makegrad(double corr);
  void maketriangles();
  void makeqindex();
  double elevation(xy location);
  double dirbound(int angle);
};
struct criterion
{
  string str;
  bool istopo;
};

typedef std::vector<criterion> criteria;

extern pointlist surveypoints,topopoints;

void copytopopoints(criteria crit);
#endif
