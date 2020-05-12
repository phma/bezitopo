/******************************************************/
/*                                                    */
/* objlist.cpp - list of drawing objects              */
/*                                                    */
/******************************************************/
/* Copyright 2015,2018,2020 Pierre Abbat.
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

#include <cassert>
#include "objlist.h"
#include "document.h"
#include "penwidth.h"
#include "layer.h"
#include "color.h"

using namespace std;

objrec::objrec()
{
  obj=nullptr;
}

objrec::~objrec()
{
  if (obj)
    free(obj);
}

unsigned short objrec::getlinetype(document *doc)
{
  if (ltype==SAMELINETYPE)
    return doc->layers[layr].ltype;
  else
    return ltype;
}

unsigned short objrec::getcolor(document *doc)
{
  if (colr==SAMECOLOR)
    return doc->layers[layr].colr;
  else
    return colr;
}

unsigned short objrec::getthickness(document *doc)
{
  if (thik>200) // see penwidth.h
    return doc->layers[layr].thik;
  else
    return thik;
}

void ObjectList::setCurrentLayer(int layer)
{
  curlayer=layer;
}

int ObjectList::getCurrentLayer()
{
  return curlayer;
}

int ObjectList::size()
{
  assert(forward.size()==reverse.size());
  return forward.size();
}

int ObjectList::insert(drawobj *obj)
/* The drawobj must be created as some subclass of drawobj with new.
 * The ObjectList takes responsibility for destroying the drawobj.
 * Returns the object handle. If the drawobj is already in the ObjectList,
 * returns the existing handle.
 */
{
  if (reverse.count(obj))
    return reverse[obj];
  else
  {
    objrec newobjrec;
    int newhandle;
    newobjrec.obj=obj;
    newobjrec.layr=curlayer;
    newobjrec.ltype=SAMELINETYPE;
    newobjrec.colr=SAMECOLOR;
    newobjrec.thik=SAMEWIDTH;
    if (reverse.size())
      newhandle=forward.rbegin()->first+1;
    else
      newhandle=0;
    forward[newhandle]=newobjrec;
    reverse[obj]=newhandle;
    return newhandle;
  }
}

void ObjectList::erase(drawobj *obj)
{
  if (reverse.count(obj))
  {
    int handle=reverse[obj];
    reverse.erase(obj);
    forward.erase(handle);
  }
}

void ObjectList::erase(int handle)
{
  if (forward.count(handle))
  {
    objrec *rec=&forward[handle];
    reverse.erase(rec->obj);
    forward.erase(handle);
  }
}

objrec ObjectList::operator[](int handle)
/* forward[handle] would insert a new objrec into forward.
 * This returns an empty objrec, but does not insert it.
 */
{
  objrec ret;
  if (forward.count(handle))
    ret=forward[handle];
  else
    ret.obj=nullptr;
  return ret;
}

int ObjectList::findHandle(drawobj *obj)
{
  if (reverse.count(obj))
    return reverse[obj];
  else
    return -1;
}

vector<int> ObjectList::allHandles()
{
  vector<int> ret;
  map<int,objrec>::iterator i;
  for (i=forward.begin();i!=forward.end();++i)
    ret.push_back(i->first);
  return ret;
}

vector<int> ObjectList::referrers(int handle)
{
  map<int,objrec>::iterator i;
  int j;
  vector<int> ret;
  for (i=forward.begin();i!=forward.end();++i)
    for (j=0;j<i->second.references.size();j++)
      if (i->second.references[j]==handle)
	ret.push_back(i->first);
  return ret;
}
