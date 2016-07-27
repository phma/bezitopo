/******************************************************/
/*                                                    */
/* sourcegeoid.h - geoidal undulation source data     */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#include <vector>
#include <string>
#include <array>
#include "angle.h"
#include "geoid.h"
#include "matrix.h"

#define HASHPRIME 729683249
// Used for hashing 256-bit patterns of which samples in a geoquad are valid.

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
  void setheader(usngsheader &hdr);
  void setheader(carlsongsfheader &hdr);
  void settest();
  void dump();
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
};

struct geoformat
{
  /* cmd is the argument to -f on the command line; ext is the file extension.
   * They are often the same, but ".bin" is used for many types of binary
   * files, so that format has cmd set to "ngs".
   * readfunc will need changing when I add boldatni to the list of formats.
   */
  std::string cmd,ext,desc;
  int (*readfunc)(geoid&,std::string);
};

bool smooth5(unsigned n);
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
std::vector<xyz> gcscint(xyz gc,smallcircle sc);
extern std::vector<geoid> geo;
double avgelev(xyz dir);
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
