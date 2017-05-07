/******************************************************/
/*                                                    */
/* geoidboundary.h - geoid boundaries                 */
/*                                                    */
/******************************************************/
/* Copyright 2016,2017 Pierre Abbat.
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
#ifndef GEOIDBOUNDARY_H
#define GEOIDBOUNDARY_H
#include <vector>
#include "geoid.h"
#include "polyline.h"

bool operator==(const vball &a,const vball &b);
bool sameEdge(const vball &a,const vball &b);

struct vsegment
{
  vball start,end;
  vball midpoint();
};

int splitLevel(double coord);
int splitLevel(vball v);
int splitLevel(vsegment v);

class g1boundary
{
private:
  std::vector<vball> bdy;
  bool inner; // used in kml
public:
  bool isempty();
  bool isInner();
  void setInner(bool i);
  int size();
  void clear();
  void push_back(vball v);
  vball operator[](int n);
  vsegment seg(int n);
  std::vector<int> segmentsAtLevel(int l);
  std::vector<int> nullSegments();
  void positionSegment(int n);
  void splice(g1boundary &b);
  void split(int n,g1boundary &b);
  void splice(int m,g1boundary &b,int n);
  void split(int m,int n,g1boundary &b);
  void deleteCollinear();
  std::vector<xyz> surfaceCorners();
  std::vector<xyz> surfaceMidpoints();
  double perimeter(bool midpt=false);
  int area();
  double cubeArea();
  friend bool operator==(const g1boundary l,const g1boundary r);
};

bool overlap(vsegment a,vsegment b);

class gboundary
{
private:
  std::vector<g1boundary> bdy;
  std::vector<polyarc> flatBdy; // for kml
  std::vector<int> areaSign; // for kml
  int segNum;
public:
  void push_back(g1boundary g1);
  g1boundary operator[](int n);
  polyarc getFlatBdy(int n);
  int size() const;
  int totalSegments();
  vsegment seg(int n);
  vsegment someSeg();
  xyz nearPoint();
  void clear();
  void setInner(int n,bool i);
  void consolidate(int l);
  void splitoff(int l);
  void deleteCollinear();
  void deleteNullSegments();
  void deleteEmpty();
  void erase(int n);
  double perimeter(bool midpt=false);
  int area();
  double cubeArea();
  friend gboundary operator+(const gboundary &l,const gboundary &r);
  void flattenBdy(); // for kml
  unsigned int in(xyz pnt);
  unsigned int in(latlong pnt);
  unsigned int in(vball pnt);
};
#endif
