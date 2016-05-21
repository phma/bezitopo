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

struct usngsheader
{
  double south,west,latspace,longspace;
  int nlat,nlong,dtype;
};

class geolattice
{
  /* nbd must be greater than sbd; both must be in [-DEG90,DEG90].
   * ebd-sbd must be positive. Neither has to be in [-DEG180,DEG180].
   * undula is 4-byte integers with 0x10000 meaning 1 meter. 0x80000000 means NaN.
   * size of undula is (width+1)*(height+1) - note fencepost!
   */
public:
  int type; // not used yet - will distinguish lat-long grid from whatever is used at the poles
  int nbd,ebd,sbd,wbd; // fixed-point binary - 18 mm is good enough for geoid work
  int width,height;
  std::vector<int> undula,eslope,nslope; // starts at southwest corner, heads east
  double elev(int lat,int lon);
  double elev(xyz dir);
  void setslopes();
  void setheader(usngsheader &hdr);
  void settest();
  void dump();
};

bool smooth5(unsigned n);
int readusngsbin(geolattice &geo,std::string filename);
extern std::vector<geolattice> geo;
double avgelev(xyz dir);
std::array<double,6> correction(geoquad &quad,double qpoints[][16]);
double maxerror(geoquad &quad,double qpoints[][16]);
