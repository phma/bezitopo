/******************************************************/
/*                                                    */
/* kml.h - Keyhole Markup Language                    */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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

#include <iostream>
#include <fstream>
#include <map>
#include "sourcegeoid.h"
#include "polyline.h"

double middleOrdinate(latlong ll0,latlong ll1);
void openkml(std::ofstream &file,char *filename);
void closekml(std::ofstream &file);
polyarc flatten(g1boundary g1);

class KmlRegionList
{
  /* The bits in the key of regionMap tell which g1boundaries the region
   * is inside. The value is a point in the region.
   * blankBitCount is the number of bits set in the key of a blank region.
   * Full regions have one more bit set.
   */
public:
  std::map<unsigned int,xyz> regionMap;
  int blankBitCount;
  unsigned int biggestBlankRegion(gboundary& allBdy);
};

gboundary regionBoundary(KmlRegionList& regionList,gboundary& allBdy,unsigned reg);
KmlRegionList kmlRegions(gboundary &gb);
gboundary extractRegion(gboundary &gb);
