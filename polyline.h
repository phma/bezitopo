/******************************************************/
/*                                                    */
/* polyline.h - polylines                             */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014,2015,2016,2017 Pierre Abbat.
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
#ifndef POLYLINE_H
#define POLYLINE_H

#include <vector>
#include "point.h"
#include "xyz.h"
#include "arc.h"
#include "bezier3d.h"
#include "spiral.h"

extern int bendlimit;
/* The maximum angle through which a segment of polyspiral can bend. If the bend
 * is greater, it will be replaced with a straight line segment. If angles in
 * contours have extra bulbs, it is too high; if smooth curves in contours have
 * straight line segments, it is too low. Default value is 120°. Setting it to
 * DEG360 will not work; set it to DEG360-1 instead.
 */

/* Polylines and alignments are very similar. The differences are:
 * polylines are in a horizontal plane, whereas alignments have vertical curves;
 * polylines have derived classes with arcs and spirals, whereas alignments
 * always have complete spiral data.
 */
int midarcdir(xy a,xy b,xy c);

class polyline: public drawobj
{
protected:
  double elevation;
  std::vector<xy> endpoints;
  std::vector<double> lengths,cumLengths;
public:
  friend class polyarc;
  friend class polyspiral;
  polyline();
  polyline(double e);
  double getElevation()
  {
    return elevation;
  }
  bool isopen();
  int size();
  segment getsegment(int i);
  void dedup();
  virtual bezier3d approx3d(double precision);
  virtual void insert(xy newpoint,int pos=-1);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  virtual double length();
  virtual double area();
  virtual double dirbound(int angle);
  virtual void writeXml(std::ofstream &ofile);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
};

class polyarc: public polyline
{
protected:
  std::vector<int> deltas;
public:
  friend class polyspiral;
  polyarc();
  polyarc(double e);
  polyarc(polyline &p);
  arc getarc(int i);
  virtual bezier3d approx3d(double precision);
  virtual void insert(xy newpoint,int pos=-1);
  void setdelta(int i,int delta);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  //virtual double length();
  virtual double area();
  virtual double dirbound(int angle);
  virtual void writeXml(std::ofstream &ofile);
};

class polyspiral: public polyarc
{
protected:
  std::vector<int> bearings; // correspond to endpoints
  std::vector<int> delta2s;
  std::vector<int> midbearings;
  std::vector<xy> midpoints;
  std::vector<double> clothances,curvatures;
  bool curvy;
public:
  polyspiral();
  polyspiral(double e);
  polyspiral(polyline &p);
  spiralarc getspiralarc(int i);
  virtual bezier3d approx3d(double precision);
  virtual void insert(xy newpoint,int pos=-1);
  void setbear(int i);
  void setspiral(int i);
  void smooth();
  //void setdelta(int i,int delta);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  //virtual double length();
  virtual double area();
  virtual double dirbound(int angle);
  virtual void writeXml(std::ofstream &ofile);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
};

class alignment
{
public:
  std::vector<xyz> endpoints;
  std::vector<int> deltas;
  std::vector<double> lengths;
};

#endif
