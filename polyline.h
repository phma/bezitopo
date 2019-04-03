/******************************************************/
/*                                                    */
/* polyline.h - polylines                             */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014-2019 Pierre Abbat.
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
 * straight line segments, it is too low. Default value is 180°. Setting it to
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
  std::vector<bcir> boundCircles;
  int stationSegment(double along);
public:
  friend class polyarc;
  friend class polyspiral;
  polyline();
  polyline(double e);
  double getElevation()
  {
    return elevation;
  }
  virtual unsigned hash();
  bool isopen();
  int size();
  segment getsegment(int i);
  xyz getEndpoint(int i);
  xyz getstart();
  xyz getend();
  void dedup();
  virtual bezier3d approx3d(double precision);
  virtual std::vector<drawingElement> render3d(double precision,int layer,int color,int width,int linetype);
  virtual void insert(xy newpoint,int pos=-1);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  double length();
  double getCumLength(int i);
  virtual xyz station(double along);
  virtual double closest(xy topoint,bool offends=false);
  virtual double area();
  virtual double dirbound(int angle,double boundsofar=INFINITY);
  virtual bool hasProperty(int prop);
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
  virtual unsigned hash();
  arc getarc(int i);
  virtual bezier3d approx3d(double precision);
  virtual void insert(xy newpoint,int pos=-1);
  void setdelta(int i,int delta);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  virtual xyz station(double along);
  virtual double closest(xy topoint,bool offends=false);
  virtual double area();
  virtual double dirbound(int angle,double boundsofar=INFINITY);
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
  virtual unsigned hash();
  spiralarc getspiralarc(int i);
  virtual bezier3d approx3d(double precision);
  virtual void insert(xy newpoint,int pos=-1);
  void setbear(int i);
  void setbear(int i,int bear);
  void setspiral(int i);
  void smooth();
  //void setdelta(int i,int delta);
  virtual void setlengths();
  virtual void open();
  virtual void close();
  virtual double in(xy point);
  virtual xyz station(double along);
  virtual double closest(xy topoint,bool offends=false);
  virtual double area();
  virtual double dirbound(int angle,double boundsofar=INFINITY);
  virtual void writeXml(std::ofstream &ofile);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
};

class alignment: public drawobj
/* An alignment consists of a sequence of horizontal curves (all are spirals,
 * possibly with zero clothance or curvature) and a sequence of vertical
 * curves (all with degree 3 or less). The stations along the horizontal
 * and vertical curves (cumLengths and vCumLengths) must start and end at the
 * same number (they don't have to start at 0) but their internal endpoints
 * generally are unrelated.
 * 
 * Normally one plans the horizontal curves first. During this phase, the
 * vertical curve is a single horizontal line at elevation 0. Then one takes
 * cross-sections along the path and designs the vertical curves.
 */
{
protected:
  std::vector<xy> endpoints;
  std::vector<int> deltas,delta2s;
  std::vector<int> bearings; // correspond to endpoints
  std::vector<int> midbearings;
  std::vector<xy> midpoints;
  std::vector<double> clothances,curvatures;
  std::vector<double> hLengths,hCumLengths;
  std::vector<bcir> boundCircles;
  std::vector<double> vLengths,vCumLengths;
  std::vector<double> controlPoints;
  std::vector<double> lengths,cumLengths;
  void setVLength();
  void setlengths();
  int xyStationSegment(double along);
  int zStationSegment(double along);
public:
  alignment();
  void clear();
  double startStation();
  double endStation();
  double length();
  void appendPoint(xy pnt);
  void prependPoint(xy pnt);
  void appendTangentCurve(double startCurvature,double length,double endCurvature);
  void prependTangentCurve(double startCurvature,double length,double endCurvature);
  spiralarc getHorizontalCurve(int i);
  segment getVerticalCurve(int i);
  void setStartStation(double along);
  void setHLengths();
  xy xyStation(double along);
  double zStation(double along);
  xyz station(double along);
  int bearing(double along);
  double slope(double along);
  double curvature(double along);
  double accel(double along);
};

#endif
