/******************************************************/
/*                                                    */
/* qindex.h - quad index to tin                       */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013 Pierre Abbat.
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

#ifndef QINDEX_H
#define QINDEX_H
#include <vector>
#include "bezier.h"
#include "point.h"
class qindex
{
public:
  double x,y,side;
  union
  {
    qindex *sub[4]; // Either all four subs are set,
    triangle *tri;  // or tri alone is set, or they're all NULL.
  };
  triangle *findt(xy pnt,bool clip=false);
  int quarter(xy pnt,bool clip=false);
  xy middle();
  void sizefit(std::vector<xy> pnts);
  void split(std::vector<xy> pnts);
  void clear();
  void draw(bool root=true);
  std::vector<qindex*> traverse(int dir=0);
  void settri(triangle *starttri);
  qindex();
  ~qindex();
  int size(); // This returns the total number of nodes, which is 4n+1. The number of leaves is 3n+1.
};
#endif
