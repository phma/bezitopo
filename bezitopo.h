/******************************************************/
/*                                                    */
/*                                                    */
/*                                                    */
/******************************************************/

#ifndef BEZITOPO_H
#define BEZITOPO_H
// #define NDEBUG
#define _WITH_GETLINE
#include <cassert>
#include <map>
#include <cstdio>
#include "point.h"

typedef std::map<int,point> ptlist;
extern ptlist points;
extern std::map<point*,int> revpoints;
extern FILE *randfil;

void addpoint(int numb,point pnt,bool overwrite=false);
void dumppoints();

#endif
