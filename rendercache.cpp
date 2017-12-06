/******************************************************/
/*                                                    */
/* rendercache.cpp - cache of renderings of drawobjs  */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#include "rendercache.h"
using namespace std;

void RenderCache::clear()
{
  renderMap.clear();
}

void RenderCache::clearPresent()
{
  map<drawobj *,RenderItem>::iterator i;
  int j,nsteps;
  for (i=renderMap.begin();i!=renderMap.end();i++)
    i->second.present=false;
  nsteps=rng.ucrandom();
  for (j=0;j<nsteps;j++)
    subrand.scalar(1);
}

void RenderCache::deleteAbsent()
{
  map<drawobj *,RenderItem>::iterator i;
  int j;
  vector<drawobj *> delenda;
  for (i=renderMap.begin();i!=renderMap.end();i++)
    if (!i->second.present)
      delenda.push_back(i->first);
  for (j=0;j<delenda.size();j++)
    renderMap.erase(delenda[j]);
}

bool RenderCache::shouldRerender(double oldScale,double newScale)
{
  double ratio=oldScale/newScale;
  double prob=ratio*exp(1-ratio);
  return prob<1 && subrand.scalar(1)>=prob;
}

void RenderCache::checkInObject(drawobj *obj,double pixelScale,int layr,int colr,int thik,int ltype)
{
  unsigned objHash;
  if (!renderMap.count(obj))
    renderMap[obj].pixelScale=INFINITY;
  renderMap[obj].colr=colr;
  renderMap[obj].thik=thik;
  renderMap[obj].ltype=ltype;
  renderMap[obj].present=true;
  objHash=obj->hash(); // Computing the hash of a large polyspiral takes 1/20 as much time as rendering it.
  if (shouldRerender(renderMap[obj].pixelScale,pixelScale) ||
      objHash!=renderMap[obj].hash)
  {
    renderMap[obj].rendering=obj->render3d(pixelScale,layr,colr,thik,ltype);
    renderMap[obj].pixelScale=pixelScale;
    renderMap[obj].hash=objHash;
  }
}

      