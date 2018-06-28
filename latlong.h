/******************************************************/
/*                                                    */
/* latlong.h - latitude-longitude structure           */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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

#ifndef LATLONG_H
#define LATLONG_H

struct latlongelev;

struct latlong
{
  double lat;
  double lon;
  latlong();
  latlong(int ilat,int ilon);
  latlong(double dlat,double dlon); // Arguments are in radians.
  latlong(latlongelev lle);
  int valid(); // 0, 1, or 2
};

void setnesw(std::string neswString);
latlong parselatlong(std::string angstr,int unitp);
std::string formatlatlong(latlong ll,int unitp);

struct latlongelev
{
  double lat;
  double lon;
  double elev;
};

#endif
