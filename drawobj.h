/******************************************************/
/*                                                    */
/* drawobj.h - drawing object base class              */
/*                                                    */
/******************************************************/
/* Copyright 2015-2019 Pierre Abbat.
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
#include <cmath>
#include "xyz.h"
#include "bezier3d.h"
#include "roscat.h"

std::string xmlEscape(std::string str);
unsigned memHash(void *mem,unsigned len,unsigned previous=0);

/* Properties of drawing objects
 * Integer properties start at 0x0000, double at 0x8000, xy at 0x4000,
 * xyz at 0xc000, and string at 0x2000.
 * If an integer property is an angle, asking for its value as a double should
 * be valid and return the angle in radians.
 */
#define PROP_DELTA 0x0000
#define PROP_DELTA2 0x0001
#define PROP_LENGTH 0x8000
#define PROP_RADIUS 0x8001
#define PROP_CURVATURE 0x8002
#define PROP_CLOTHANCE 0x8003
#define PROP_START_TANGENT 0x8004
#define PROP_END_TANGENT 0x8005
#define PROP_THROW 0x8006
#define PROP_CENTER 0x4000
#define PROP_MIDPOINT_XY 0x4001
#define PROP_POINT_INTERSECTION 0x4002
#define PROP_MIDPOINT_XYZ 0xc000

class bsph
{
public:
  xyz center;
  double radius;
};

class bcir
{
public:
  xy center;
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
  unsigned short color;
  short width;
  unsigned short linetype;
  bool filled;
  bezier3d path;
  drawingElement();
  drawingElement(bezier3d b3d);
  drawingElement(bezier3d b3d,int c,int w,int lt);
};

class drawobj
/* Derived classes:
 * point
 * segment
 *   arc
 *   spiralarc
 * polyline
 *   polyarc
 *     polyspiral
 * alignment
 * Circle
 */
{
public:
  virtual bsph boundsphere();
  virtual bool hittest(hline hitline);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
  {
  }
  virtual void roscat(xy tfrom,int ro,double sca,xy tto); // rotate, scale, translate
  virtual unsigned hash();
  virtual double dirbound(int angle,double boundsofar=INFINITY);
  virtual std::vector<drawingElement> render3d(double precision,int layer,int color,int width,int linetype);
  /* render3d is normally called with layer=-1 and color, width, and linetype
   * set to actual values, or layer set to an actual value and color, width,
   * and linetype set to SAMEfoo. The latter won't happen until I implement
   * layers.
   */
  virtual bool hasProperty(int prop);
  virtual int getIntProperty(int prop);
  virtual double getDoubleProperty(int prop);
  virtual xy getXyProperty(int prop);
  virtual xyz getXyzProperty(int prop);
  virtual std::string getStringProperty(int prop);
  virtual void writeXml(std::ofstream &ofile);
};

#endif
