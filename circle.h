/******************************************************/
/*                                                    */
/* circle.h - circles, including lines                */
/*                                                    */
/******************************************************/
/* Copyright 2017-2018 Pierre Abbat.
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
#ifndef CIRCLE_H
#define CIRCLE_H
#include "drawobj.h"

class Circle: public drawobj
/* These circles are used for construction:
 * • For a spiralarc, construct the throw, the shortest segment between the
 *   osculating circles.
 * • Given two circles which do not intersect and are not concentric, construct
 *   the spiralarc that osculates both.
 * They are drawing objects, so they can be in a drawing, but they are 2D circles.
 * If you want a circle with elevation, or a circle which is a lot boundary,
 * use two arcs.
 */
{
public:
  Circle();
  Circle(xy c,double r);
  Circle(xy m,int b,double c=0);
  xy center() const;
  xyz station(double along) const;
  int bearing(double along) const;
  double curvature() const;
  double radius() const;
  double length() const;
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
  virtual void roscat(xy tfrom,int ro,double sca,xy tto); // rotate, scale, translate
  virtual unsigned hash();
  virtual double dirbound(int angle,double boundsofar=INFINITY);
  bezier3d approx3d(double precision);
  virtual std::vector<drawingElement> render3d(double precision,int layer,int color,int width,int linetype);
  virtual void writeXml(std::ofstream &ofile);
private:
  xy mid;
  int bear;
  double rbear; // chordbearing in radians
  double cur;
  double visibleLength(double precision);
};
#endif
