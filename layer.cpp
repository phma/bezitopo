/******************************************************/
/*                                                    */
/* layer.cpp - drawing layers                         */
/*                                                    */
/******************************************************/
/* Copyright 2015,2019 Pierre Abbat.
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

#include "layer.h"

using namespace std;

int LayerList::newLayer(string na)
{
  int i;
  string numstr;
  for (i=0;layerMap.count(i);i++);
  numstr=to_string(i);
  while (na.length()==0 || layerMap.count(findLayer(na)))
    na+=numstr;
  layerMap[i].name=na;
  layerMap[i].colr=BLACK;
  layerMap[i].thik=ZEROWIDTH;
  layerMap[i].ltype=SOLIDLINE;
  layerMap[i].visible=true;
  return i;
}

int LayerList::findLayer(std::string na)
/* If there is a layer by that name, returns its number, which is normally nonnegative.
 * If there isn't, returns a negative number of no layer, normally -1.
 */
{
  map<int,layer>::iterator it;
  int i;
  for (i=-1;layerMap.count(i);i--);
  for (it=layerMap.begin();it!=layerMap.end();it++)
    if (it->second.name==na)
    {
      i=it->first;
      break;
    }
  return i;
}

size_t LayerList::size()
{
  return layerMap.size();
}

bool LayerList::setName(int l,std::string na)
/* Returns true if successful. Returns false if layer l does not exist
 * or the name is already in use. If you try to rename a layer to the same name,
 * returns false, because the name is in use.
 */
{
  int i=findLayer(na);
  if (na.length()==0 || layerMap.count(l)==0 || layerMap.count(i)>0)
    return false;
  else
  {
    layerMap[l].name=na;
    return true;
  }
}

bool LayerList::setColor(int l,int color)
/* Returns false if the layer does not exist or you try to set it to SAMECOLOR.
 * Anything greater than WHITE is considered to be SAMECOLOR.
 */
{
  if (color<=WHITE && color>=0 && layerMap.count(l))
  {
    layerMap[l].colr=color;
    return true;
  }
  else
    return false;
}

bool LayerList::setWidth(int l,int width)
{
  if (width<=200 && width>=-32768 && layerMap.count(l))
  {
    layerMap[l].thik=width;
    return true;
  }
  else
    return false;
}

bool LayerList::setLinetype(int l,int linetype)
{
  if (linetype<SAMELINETYPE && linetype>=0 && layerMap.count(l))
  {
    layerMap[l].ltype=linetype;
    return true;
  }
  else
    return false;
}

void LayerList::applyLayer(int l,int &color,int &width,int &linetype)
{
  if (layerMap.count(l))
  {
    if (color>WHITE)
      color=layerMap[l].colr;
    if (width>200)
      width=layerMap[l].thik;
    if (linetype==SAMELINETYPE)
      linetype=layerMap[l].ltype;
  }
}
