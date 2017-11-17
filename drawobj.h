/******************************************************/
/*                                                    */
/* drawobj.h - drawing object base class              */
/*                                                    */
/******************************************************/
/* Copyright 2015-2017 Pierre Abbat.
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

#ifndef DRAWOBJ_H
#define DRAWOBJ_H
#include <fstream>
#include <string>
#include <vector>
#include "xyz.h"
#include "bezier3d.h"

std::string xmlEscape(std::string str);
unsigned memHash(unsigned *mem,unsigned len,unsigned previous=0);

class bsph
{
public:
  xyz center;
  double radius;
};

class hline
{
public:
  xyz dir,pnt;
};

class drawingElement
{
public:
  short color; // Normally returned as SAMECOLOR, unless it's a component of a block.
  short width;
  bool filled;
  bezier3d path;
  drawingElement();
  drawingElement(bezier3d b3d);
};

class drawobj
{
public:
  virtual bsph boundsphere();
  virtual bool hittest(hline hitline);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
  {
  }
  virtual void roscat(xy tfrom,int ro,double sca,xy tto); // rotate, scale, translate
  virtual int hash();
  virtual std::vector<drawingElement> render3d(double precision);
  virtual void writeXml(std::ofstream &ofile);
};

#endif
