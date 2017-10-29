/******************************************************/
/*                                                    */
/* document.cpp - main document class                 */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016,2017 Pierre Abbat.
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
#include "bezitopo.h"
#include "pnezd.h"
#include "document.h"
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
    throw unsetsource;
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
  ::writepnezd(this,fname,mscopy);
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
  ::writepenzd(this,fname,mscopy);
}

void document::addobject(drawobj *obj)
// The drawobj must be created with new; it will be destroyed with delete.
{
  objrec o;
  if (curlayer<0 || curlayer>=layers.size())
  {
    curlayer=0;
    if (layers.size()==0)
    {
      layer l;
      l.colr=WHITE;
      l.visible=true;
      l.name="0";
      layers.push_back(l);
    }
  }
  o.layr=curlayer;
  o.ltype=o.colr=o.thik=SAMECOLOR;
  o.obj=obj;
  objlist.push_back(o);
}

void document::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<Bezitopo>";
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
  for (i=0;i<objlist.size();i++)
    objlist[i].obj->roscat(newOffset,0,1,offset); // the z has to be adjusted elsewise.
  offset=newOffset;
}
