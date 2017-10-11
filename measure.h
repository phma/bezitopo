/******************************************************/
/*                                                    */
/* measure.h - measuring units                        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2015,2016,2017 Pierre Abbat.
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

#ifndef MEASURE_H
#define MEASURE_H

#include <string>
#include <map>
#include <vector>
#include "zoom.h"

/* These are unit codes. Codes pertaining to angles are defined in angle.h.
 * This file has two codes for the foot: one for the international foot, and
 * one whose conversion factor can be switched.
 * 
 * This module follows http://bezitopo.org/~phma/Metric/unit-codes.html, except
 * for having some units with variable conversion factors. All such units
 * are non-metric units based on the foot and have codes of the form
 * 0x****f***. They may or may not be equal to other unit codes' conversion
 * factors.
 */
#define METER 0x00011000
#define MILLIMETER 0x00010d00
#define MICROMETER 0x00010c00
#define KILOMETER 0x00011300
#define MEGAMETER 0x00011400
#define FOOT 0x0001ea00
#define INTCHAIN 0x0001f100
#define CHAIN 0x0001f200
#define MILE 0x0001f300
#define SQUAREMETER 0x00271000
#define SQUAREFOOT 0x0027ea00
#define HECTARE 0x00273200
#define ACRE 0x0027e500
#define GRAM 0x00021000
#define KILOGRAM 0x00021300
#define POUND 0x0002ed00
#define MILLIPOUND 0x0002f000
#define HOUR 0x0003ee00
#define KGPERL 0x00801400
#define LBPERIN3 0x0080ef00
#define PERKG 0x00291300
#define PERPOUND 0x0029ed00
#define PERMETER 0x002a1000
#define PERFOOT 0x002aeb00
#define PERLOT 0x40000000
#define ITEM 0x04001000
#define THOUSAND 0x04001300
#define EACH 0x04011000
#define PERHUNDRED 0x04011200
#define PERTHOUSAND 0x04011300
#define PERSET 0x40010000
#define LOT 0x40020000
#define PERLITER 0x00353000
#define SET 0x40030000
#define PERMONTH 0x0036eb00
#define PERYEAR 0x0036ea00
#define PERHOUR 0x0036ee00
#define MILLILITER 0x00282d00
#define IN3 0x0028ec00
// These are physical quantity codes.
#define LENGTH 0x00010000
#define MASS 0x00020000
#define AREA 0x00270000
#define VOLUME 0x00280000
/* These convert as different units depending on the selected measuring system. */
#define MASS1 0x0002fe00
#define MASS2 0x0002fd00
#define LENGTH1 0x0001fe00
#define LENGTH2 0x0001fd00
#define ANGLE 0x00380000
/* These are precision codes. DEC3 = 3 digits after the decimal point. */
#define DEC0 0x00
#define DEC1 0x01
#define DEC2 0x02
#define DEC3 0x03
#define DEC4 0x04
#define DEC5 0x05
#define DEC6 0x06
#define HALF 0x11
#define QUARTER 0x12
#define EIGHTH 0x13
#define SIXTEENTH 0x14
#define THIRTYSECOND 0x15

#define INTERNATIONAL 0
#define USSURVEY 1
#define INSURVEY 2

#define badunits 4
#define badnumber 10
// error thrown by conversion routine

inline int physicalQuantity(int unitp)
{
  return unitp&0xffff0000;
}

inline int physicalUnit(int unitp)
{
  return unitp&0xffffff00;
}

inline bool compatibleUnits(int unitp1,int unitp2)
{
  return physicalQuantity(unitp1)==physicalQuantity(unitp2);
}

inline bool sameUnit(int unitp1,int unitp2)
{
  return physicalUnit(unitp1)==physicalUnit(unitp2);
}

double precision(int unitp);

struct Measurement
{
  double magnitude; // always in a coherent SI unit regardless of what unit is
  int unit;
};

class Measure
{
public:
  Measure();
  void setFoot(int which);
  void addUnit(int unit);
  void removeUnit(int unit);
  void clearUnits();
  void localize(bool loc);
  void setMetric();
  void setCustomary();
  void setDefaultUnit(int quantity,double magnitude);
  void setDefaultPrecision(int quantity,double magnitude);
  int findUnit(int quantity,double magnitude=0);
  int findPrecision(int unit,double magnitude=0);
  double toCoherent(double measurement,int unit,double unitMagnitude=0);
  double fromCoherent(double measurement,int unit,double unitMagnitude=0);
  std::string formatMeasurement(double measurement,int unit,double unitMagnitude=0,double precisionMagnitude=0);
  std::string formatMeasurementUnit(double measurement,int unit,double unitMagnitude=0,double precisionMagnitude=0);
  Measurement parseMeasurement(std::string measStr,int quantity);
private:
  int whichFoot;
  bool localized;
  std::map<int,double> conversionFactors;
  std::vector<int> availableUnits;
  std::map<int,double> defaultUnit,defaultPrecision;
};
#endif
