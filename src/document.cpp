/******************************************************/
/*                                                    */
/* document.cpp - main document class                 */
/*                                                    */
/******************************************************/
/* Copyright 2015-2020,2024 Pierre Abbat.
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
#include "globals.h"
#include "pnezd.h"
#include "document.h"
#include "except.h"
#include "penwidth.h"
#include "color.h"
#include "linetype.h"
using namespace std;

void document::makepointlist(int n)
{
  if (pl.size()<n+1)
    pl.resize(n+1);
}

void document::copytopopoints(int dst,int src)
/* Uses the criteria in the destination. If the destination doesn't exist,
 * creates it, with no criteria. But if the source doesn't exist, it throws.
 * If you want points included unless they match a criterion, begin the
 * criteria file with "0,0,,1".
 */
{
  ptlist::iterator i;
  if (dst==src || src<0 || src>=pl.size())
    throw BeziExcept(unsetSource);
  makepointlist(dst);
  pl[dst].clear();
  int j;
  bool include;
  for (i=pl[src].points.begin();i!=pl[src].points.end();i++)
  {
    include=false;
    for (j=0;j<pl[dst].crit.size();j++)
      if (pl[dst].crit[j].match(i->second,i->first))
	include=pl[dst].crit[j].istopo;
    if (include)
      pl[dst].addpoint(i->first,i->second);
  }
}

int document::readpnezd(string fname,bool overwrite)
{
  Measure mscopy=ms; // Make an unlocalized copy of ms so that the file
  mscopy.localize(false); // will be read with periods for decimal points.
  makepointlist(0);
  return ::readpnezd(this,fname,mscopy,overwrite);
}

int document::writepnezd(string fname)
{
  Measure mscopy=ms;
  mscopy.localize(false);
  return ::writepnezd(this,fname,mscopy);
}

int document::readpenzd(string fname,bool overwrite)
{
  Measure mscopy=ms;
  mscopy.localize(false);
  makepointlist(0);
  return ::readpenzd(this,fname,mscopy,overwrite);
}

int document::writepenzd(string fname)
{
  Measure mscopy=ms;
  mscopy.localize(false);
  return ::writepenzd(this,fname,mscopy);
}

int document::readzoom(string fname,bool overwrite)
{
  Measure mscopy=ms;
  mscopy.localize(false);
  makepointlist(0);
  return ::readzoom(this,fname,mscopy,overwrite);
}

int document::writezoom(string fname)
{
  Measure mscopy=ms;
  mscopy.localize(false);
  return ::writezoom(this,fname,mscopy);
}

void document::addobject(drawobj *obj)
// The drawobj must be created with new; it will be destroyed with delete.
{
  if (!layers.isLayer(modelSpace.getCurrentLayer()))
  {
    modelSpace.setCurrentLayer(0);
    if (!layers.isLayer(0))
      layers.newLayer(0);
  }
  modelSpace.insert(obj);
}

void document::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<Bezitopo>";
  ms.writeXml(ofile);
  for (i=0;i<pl.size();i++)
    pl[i].writeXml(ofile);
  ofile<<"</Bezitopo>"<<endl;
}

void document::changeOffset (xyz newOffset)
/* Changes the offset of the document, moving everything so that its coordinates
 * added to the offset do not change. Everything, that is, except the quad index,
 * which will have to be recreated.
 */
{
  int i;
  for (i=0;i<pl.size();i++)
    pl[i].roscat(newOffset,0,1,offset); // FIXME: roscat takes xy;
  for (i=0;i<modelSpace.size();i++)
    modelSpace[i].obj->roscat(newOffset,0,1,offset); // the z has to be adjusted elsewise.
  offset=newOffset;
}
