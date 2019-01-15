/******************************************************/
/*                                                    */
/* layer.h - drawing layers                           */
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

#ifndef LAYER_H
#define LAYER_H
#include <string>
#include <map>
#include "color.h"
#include "linetype.h"
#include "penwidth.h"

class layer
{
public:
  unsigned short colr,thik,ltype;
  bool visible;
  std::string name;
};

class LayerList
{
public:
  int newLayer(std::string na="");
  int findLayer(std::string na);
  bool setName(int l,std::string na);
  bool setColor(int l,int color);
  bool setWidth(int l,int width);
  bool setLinetype(int l,int linetype);
  void applyLayer(int l,int &color,int &width,int &linetype);
private:
  std::map<int,layer> layerMap;
};
#endif
