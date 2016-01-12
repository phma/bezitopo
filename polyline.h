/******************************************************/
/*                                                    */
/* polyline.h - polylines                             */
/*                                                    */
/******************************************************/
#ifndef POLYLINE_H
#define POLYLINE_H

#include <vector>
#include "point.h"
#include "xyz.h"
#include "arc.h"
#include "bezier3d.h"
#include "spiral.h"

#define BENDLIMIT DEG120

/* Polylines and alignments are very similar. The differences are:
 * polylines are in a horizontal plane, whereas alignments have vertical curves;
 * polylines consist of arcs, whereas alignments can have spirals (once I figure out how).
 */
int midarcdir(xy a,xy b,xy c);

class polyline: public drawobj
{
protected:
  double elevation;
  std::vector<xy> endpoints;
  std::vector<double> lengths;
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
  virtual double length();
  virtual double area();
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
  //virtual double length();
  virtual double area();
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
  //virtual void setlengths();
  virtual void open();
  virtual void close();
  //virtual double length();
  //virtual double area();
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
