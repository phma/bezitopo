/******************************************************/
/*                                                    */
/* objlist.h - list of drawing objects                */
/*                                                    */
/******************************************************/
/* Copyright 2015,2018 Pierre Abbat.
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

#ifndef OBJLIST_H
#define OBJLIST_H
#include <map>
#include <vector>
#include "drawobj.h"

class document;

class objrec
/* If this object is a dimension, it refers to some other object, such as a
 * segment or arc. references contains the handle of the segment or arc.
 * When the segment or arc is changed, the program finds the dimension and
 * tells it to update.
 */
{
public:
  drawobj *obj;
  unsigned short layr,ltype,colr,thik;
  std::vector<int> references; // handles of objects that this object refers to
  objrec();
  ~objrec();
  unsigned short getlinetype(document *doc);
  unsigned short getcolor(document *doc);
  unsigned short getthickness(document *doc);
};

class ObjectList
{
private:
  std::map<int,objrec> forward;
  std::map<drawobj *,int> reverse;
  int curlayer;
public:
  void setCurrentLayer(int layer);
  int getCurrentLayer();
  int size();
  int insert(drawobj *obj);
  void erase(drawobj *obj); // These do not check for references,
  void erase(int handle);   // so they can leave dangling references.
  objrec operator[](int handle);
  int findHandle(drawobj *obj);
  std::vector<int> referrers(int handle);
};

#endif
