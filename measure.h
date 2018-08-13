/******************************************************/
/*                                                    */
/* measure.h - measuring units                        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2015-2018 Pierre Abbat.
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
#include <cstdint>
#include "zoom.h"
#include "xyz.h"

/* These are unit codes. Codes pertaining to angles are defined in angle.h.
 * This file has two codes for the foot: one for the international foot, and
 * one whose conversion factor can be switched.
 * 
 * This module follows http://bezitopo.org/~phma/Metric/unit-codes.html, except
 * for having some units with variable conversion factors. All such units
 * are non-metric units based on the foot and have codes of the form
 * 0x****f8**0000. They may or may not be equal to other unit codes' conversion
 * factors.
 */
#define METER 0x000100400000
#define MILLIMETER 0x0001003d0000
#define MICROMETER 0x0001003c0000
#define KILOMETER 0x000100430000
#define MEGAMETER 0x000100440000
#define FOOT 0x0001f8000000
#define INTFOOT 0x0001effa0000
#define CHAIN 0x0001f8010000
#define MILE 0x0001f8020000
#define SQUAREMETER 0x002700400000
#define SQUAREFOOT 0x0027f8000000
#define HECTARE 0x002700c20000
#define ACRE 0x0027f8010000
#define GRAM 0x000200400000
#define KILOGRAM 0x000200430000
#define POUND 0x0002effd0000
#define HOUR 0x0003effe0000
#define DEGREE 0x0020effe0000
#define ARCMINUTE 0x0020efff0000
#define ARCSECOND 0x002001400000
#define GON 0x002000c00000
#define RADIAN 0x002000400000
#define DEGREE_B 0x0038effe0000
#define ARCMINUTE_B 0x0038efff0000
#define ARCSECOND_B 0x003801400000
#define GON_B 0x003800c00000
#define RADIAN_B 0x003800400000
/* These are physical quantity codes.
 * ANGLE is used when the angle is in radians (latitude and longitude, except
 * in geolattices); ANGLE_B is used when it's in fixed-point binary (bearings,
 * angles turned with a total station, bounds of geolattices, and areas of
 * the earth). Areas of the earth should really be 0x0021..., but the angle
 * excess of a spherical polygon is equal to its area.
 */
#define LENGTH 0x000100000000
#define MASS 0x000200000000
#define AREA 0x002700000000
#define VOLUME 0x002800000000
#define ANGLE 0x002000000000
#define ANGLE_B 0x003800000000
/* These are precision codes. DEC3 = 3 digits after the decimal point.
 * FIXLARGER means that it's in fixed point with larger units, e.g.
 * ARCSECOND+FIXLARGER+DEC3 means degrees, minutes, and seconds with
 * three decimal places.
 */
#define DEC0 0x0280
#define DEC1 0x0281
#define DEC2 0x0282
#define DEC3 0x0283
#define DEC4 0x0284
#define DEC5 0x0285
#define DEC6 0x0286
#define DECIMAL 0x02bf
#define HALF 0x0101
#define QUARTER 0x0102
#define EIGHTH 0x0103
#define SIXTEENTH 0x0104
#define THIRTYSECOND 0x0105
#define BINARY 0x01ff
#define SEXAG0 0x360
#define SEXAG1 0x361
#define SEXAG2 0x362
#define SEXAGESIMAL 0x037f
#define FIXLARGER 0x2000
#define INTERNATIONAL 0
#define USSURVEY 1
#define INSURVEY 2


inline int64_t physicalQuantity(int64_t unitp)
{
  return unitp&0xffff00000000;
}

inline int64_t physicalUnit(int64_t unitp)
{
  return unitp&0xffffffff0000;
}

inline bool compatibleUnits(int64_t unitp1,int64_t unitp2)
{
  return physicalQuantity(unitp1)==physicalQuantity(unitp2);
}

inline bool sameUnit(int64_t unitp1,int64_t unitp2)
{
  return physicalUnit(unitp1)==physicalUnit(unitp2);
}

struct BasePrecision
{
  int notation; // 0=fixed; 1=scientific; 2=with larger units; 3=engineering
  int base;
  int power; // -1 means use precisionMagnitude
};

BasePrecision basePrecision(int64_t unitp);
double precision(int64_t unitp);

struct Measurement
{
  double magnitude; // always in a coherent SI unit regardless of what unit is
  int64_t unit;
};

class Measure
{
public:
  Measure();
  void setFoot(int which);
  int getFoot();
  void addUnit(int64_t unit);
  void removeUnit(int64_t unit);
  void clearUnits();
  void localize(bool loc);
  void setMetric();
  void setCustomary();
  void setDefaultUnit(int64_t quantity,double magnitude);
  void setDefaultPrecision(int64_t quantity,double magnitude);
  int64_t findUnit(int64_t quantity,double magnitude=0);
  int findPrecision(int64_t unit,double magnitude=0);
  double toCoherent(double measurement,int64_t unit,double unitMagnitude=0);
  double fromCoherent(double measurement,int64_t unit,double unitMagnitude=0);
  std::string formatMeasurement(double measurement,int64_t unit,double unitMagnitude=0,double precisionMagnitude=0);
  std::string formatMeasurementUnit(double measurement,int64_t unit,double unitMagnitude=0,double precisionMagnitude=0);
  Measurement parseMeasurement(std::string measStr,int64_t quantity);
  xy parseXy(std::string xystr);
  void writeXml(std::ostream &ofile);
private:
  int whichFoot;
  bool localized;
  std::map<int64_t,double> conversionFactors;
  std::vector<int64_t> availableUnits;
  std::map<int64_t,double> defaultUnit,defaultPrecision;
};
#endif
