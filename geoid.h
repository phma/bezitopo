/******************************************************/
/*                                                    */
/* geoid.h - geoidal undulation                       */
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
/* The native format is a set of six quadtrees, one each for Arctic, Antarctic,
 * Benin, Galápagos, Howland, and Bengal. Each leaf square is a set of six
 * values for the constant, linear, and quadratic components.
 */
#ifndef GEOID_H
#define GEOID_H
#include <vector>
#include <array>
#include <cstring>
#include "xyz.h"
#include "ellipsoid.h"
#include "vball.h"
#include "geoidboundary.h"

#define BOL_EARTH 0
#define BOL_UNDULATION 0
#define BOL_VARLENGTH 1

class gboundary;

unsigned byteswap(unsigned n);

struct cylinterval
/* A rectangle on a cylindrical map. Used when excerpting
 * a geoid file into a geolattice.
 */
{
  int nbd,ebd,sbd,wbd;
  double area();
  void setfull();
  void setempty();
  void round(int latfineness,int lonfineness);
  latlong corner(int n);
};

class geoquad
{
public:
  union
  {
    geoquad *sub[4];
    int und[6];
  };
  xy center;
  float scale; // always a power of 2
  int face;
#ifdef NUMSGEOID
  std::vector<xy> nans,nums;
#endif
  bool subdivided() const;
  bool isnan();
  geoquad();
  ~geoquad();
  geoquad(const geoquad& b);
  geoquad& operator=(geoquad b);
  vball vcenter() const;
  int splitLevel() const;
  void clear();
  void subdivide();
  void filldepth(int depth);
  bool in(xy pnt) const; // does not check whether it's on the right face
  bool in(vball pnt) const; // does check
  double undulation(double x,double y);
  xyz centeronearth();
  double length(); // length, width, and apxarea are accurate only for small squares
  double width(); // and ignore the orientation of the square relative to the
  double apxarea(); // displacement from the center of the face.
  double angarea();
  double area();
#ifdef NUMSGEOID
  int isfull(); // -1 if empty, 0 if partly full or unknown, 1 if full
#endif
  std::array<unsigned,2> hash();
  std::vector<cylinterval> boundrects();
  std::vector<double> areas();
  std::array<vball,4> bounds() const;
  gboundary gbounds();
  void writeBinary(std::ostream &ofile,int nesting=0);
  void readBinary(std::istream &ifile,int nesting=-1);
  void dump(std::ostream &ofile,int nesting=0);
  std::array<int,6> undrange();
  std::array<int,5> undhisto();
};

class cubemap
{
public:
  geoquad faces[6]; // note off-by-one: faces[0] is face 1, the Benin face
  double scale; // vertical scale, e.g. 1 means 1/65536 m. always a power of 2
  std::array<unsigned,2> hash();
  cubemap();
  ~cubemap();
  void clear();
  double undulation(int lat,int lon);
  double undulation(latlong ll);
  double undulation(xyz dir);
  std::vector<cylinterval> boundrects();
  std::vector<double> areas();
  cylinterval boundrect();
  gboundary gbounds();
  void writeBinary(std::ostream &ofile);
  void readBinary(std::istream &ifile);
  void dump(std::ostream &ofile);
  std::array<int,6> undrange();
  std::array<int,5> undhisto();
};

struct geoheader
{
  std::array<unsigned,2> hash,origHash;
  /* If this is an original boldatni file or one converted from another format,
   * hash and origHash are equal. If this is an excerpt of a boldatni file,
   * hash is set to the hash of the excerpt, while origHash is set to the
   * hash of the original file it was excerpted from. This is so that field
   * software using excerpts can tag the file as using geoid separations from
   * the large file.
   */
  bool excerpted;
  int planet;
  int dataType;
  int encoding;
  int ncomponents;
  int xComponentBits;
  /* Bitmask indicating which components are x-coordinates of xy-pairs. In a
   * file of gravity tilt, you would set ncomponents to 2 and xComponentBits
   * to 1 (bit 0 set), meaning that component 0 is x and component 1 is y.
   */
  int logScale;
  double tolerance;
  double sublimit;
  double spacing;
  std::vector<std::string> namesFormats;
  void writeBinary(std::ostream &ofile);
  void readBinary(std::istream &ifile);
};

cylinterval combine(cylinterval a,cylinterval b);
cylinterval intersect(cylinterval a,cylinterval b);
int gap(cylinterval a,cylinterval b);
bool westof(cylinterval a,cylinterval b);
cylinterval combine(std::vector<cylinterval> cyls);

extern cubemap cube;
#endif
