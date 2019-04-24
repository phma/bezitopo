/******************************************************/
/*                                                    */
/* sourcegeoid.h - geoidal undulation source data     */
/*                                                    */
/******************************************************/
/* Copyright 2015-2018 Pierre Abbat.
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
/* http://www.hypack.com/new/portals/1/PDF/sb/07_10/New Geoid Model for France and Information about Geoid File Formats.pdf
 * describes some geoid file formats. These are latitude-longitude grids.
 * I don't know what formats are used for polar geoid files.
 */
#ifndef SOURCEGEOID_H
#define SOURCEGEOID_H
#include <vector>
#include <string>
#include <array>
#include "angle.h"
#include "geoid.h"
#include "matrix.h"

#define HASHPRIME 729683249
// Used for hashing 256-bit patterns of which samples in a geoquad are valid.
#define ENDIAN_BIG 192
#define ENDIAN_NATIVE 24
#define ENDIAN_LITTLE 3

struct usngsheader
{
  double south,west,latspace,longspace;
  int nlat,nlong,dtype;
};

struct carlsongsfheader
{
  double south,west,north,east;
  int nlong,nlat;
};

struct usngatxtheader
{
  double south,west,north,east,latspace,longspace;
};

class geolattice
{
  /* nbd must be greater than sbd; both must be in [-DEG90,DEG90].
   * ebd-sbd must be positive. Neither has to be in [-DEG180,DEG180].
   * undula is 4-byte integers with 0x10000 meaning 1 meter. 0x80000000 means NaN.
   * size of undula is (width+1)*(height+1) - note fencepost!
   */
public:
  int nbd,ebd,sbd,wbd; // fixed-point binary - 18 mm is good enough for geoid work
  int width,height;
  std::vector<int> undula,eslope,nslope; // starts at southwest corner, heads east
  double elev(int lat,int lon);
  double elev(xyz dir);
  void setslopes();
  void resize(size_t dataSize=~(size_t)0);
  void setundula();
  void setbound(cylinterval bound);
  void setheader(usngsheader &hdr);
  void cvtheader(usngsheader &hdr);
  void setheader(carlsongsfheader &hdr,size_t dataSize);
  void cvtheader(carlsongsfheader &hdr);
  void setheader(usngatxtheader &hdr,size_t dataSize);
  void cvtheader(usngatxtheader &hdr);
  void settest();
  void dump();
  void setfineness(int latfineness,int lonfineness);
  int getLatFineness();
  int getLonFineness();
  cylinterval boundrect();
};

struct smallcircle
// Used to express "within 200 km of Philadelphia" and the like.
{
  xyz center;
  double cosrad;
  int radius; // angle
  void setradius(int r);
  smallcircle complement();
  double farin(xyz pt);
  bool in(xyz pt);
  cylinterval boundrect();
};

struct geoid
{
  geoheader *ghdr;
  cubemap *cmap;
  geolattice *glat;
  geoid();
  ~geoid();
  geoid(const geoid &b);
  double elev(int lat,int lon);
  double elev(xyz dir);
  int getLatFineness();
  int getLonFineness();
  cylinterval boundrect();
};

struct geoformat
{
  /* cmd is the argument to -f on the command line; ext is the file extension.
   * They are often the same, but ".bin" is used for many types of binary
   * files, so that format has cmd set to "ngs".
   */
  std::string cmd,ext,desc;
  int (*readfunc)(geoid&,std::string);
  void (*writefunc)(geoid&,std::string);
};

void setEndian(int n);
std::string readword(std::istream &file);
double readdouble(std::istream &file);
/* The read<geoidformat> functions return:
 * 0 if the file could not be opened for reading
 * 1 if the file could be opened, but is not of that format
 * 2 if they succeed.
 */
int readusngsbin(geolattice &geo,std::string filename);
int readusngsbin(geoid &geo,std::string filename);
int readcarlsongsf(geolattice &geo,std::string filename);
int readcarlsongsf(geoid &geo,std::string filename);
int readusngatxt(geoid &geo,std::string filename);
int readusngabin(geoid &geo,std::string filename);
int readboldatni(geoid &geo,std::string filename);
void writeusngsbin(geolattice &geo,std::string filename);
void writeusngsbin(geoid &geo,std::string filename);
void writecarlsongsf(geolattice &geo,std::string filename);
void writecarlsongsf(geoid &geo,std::string filename);
void writeusngatxt(geolattice &geo,std::string filename);
void writeusngatxt(geoid &geo,std::string filename);
void writeboldatni(geoid &geo,std::string filename);
std::vector<xyz> gcscint(xyz gc,smallcircle sc);
extern std::vector<geoid> geo;
extern std::vector<smallcircle> excerptcircles;
extern cylinterval excerptinterval;
double avgelev(xyz dir);
bool allBoldatni();
geoquadMatch bolMatch(geoquad &quad);
double qscale(int i,int qsz);
std::array<double,6> correction(geoquad &quad,double qpoints[][16],int qsz);
double maxerror(geoquad &quad,double qpoints[][16],int qsz);
/* qsz is the number of points on the side of the square used for
 * sampling the geoid for converting to a geoquad. It must be
 * in [4,16]. It can't be 3 because 9/2<6.
 */
int quadhash(double qpoints[][16],int qsz);
matrix autocorr(double qpoints[][16],int qsz);
void dump256(double qpoints[][16],int qsz);
bool overlap(smallcircle sc,const geoquad &gq);
#endif
