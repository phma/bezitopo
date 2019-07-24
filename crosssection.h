/******************************************************/
/*                                                    */
/* crosssection.h - cross sections of roadways        */
/*                                                    */
/******************************************************/
/* Copyright 2013,2019 Pierre Abbat.
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

/* There are two kinds of cross section: TIN cross sections, which
 * are taken of a TIN and design cross sections, which specify the
 * design of a road.
 * A design cross section is taken at a station and consists of a list
 * of triples (index,offset,elevation), where index is a dimensionless
 * number and offset and elevation are in meters. By convention,
 * index=0 where offset=0, which is the centerline. index is used to
 * connect corresponding points of one cross section to the next.
 * elevation is relative to the alignment.
 */
#include <map>
#include "xyz.h"

struct xsitem
{
  double offset,elevation;
};

class BaseCrossSection
{
public:
  void setVerticalOffset(double o);
  double getVerticalOffset();
  virtual double elevation(double o)=0;
  virtual double lowElevation(double o)=0;
  virtual double highElevation(double o)=0;
protected:
  xy midpoint;
  double verticalOffset;
  int direction;
};

class DesignCrossSection: public BaseCrossSection
{
  std::map<double,xsitem> data;
};
