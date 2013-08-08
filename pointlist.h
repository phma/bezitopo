#ifndef POINTLIST_H
#define POINTLIST_H

#include <map>
#include <string>
#include <vector>
#include "point.h"
#include "tin.h"

typedef std::map<int,point> ptlist;
typedef std::map<point*,int> revptlist;
class pointlist
{
public:
  ptlist points;
  revptlist revpoints;
  std::vector<edge> edgelist;
  void addpoint(int numb,point pnt,bool overwrite=false);
  void clear();
  // the following methods are in tin.cpp
private:
  void dumpedges();
  void dumpedges_ps(bool colorfibaster);
public:
  void maketin(string filename="",bool colorfibaster=false);
  void makegrad(double corr);
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
