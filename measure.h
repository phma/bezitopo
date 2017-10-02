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
#define INTFOOT 0x0001ea00
#define FOOT 0x0001f000
#define INTCHAIN 0x0001f100
#define CHAIN 0x0001f200
#define MILE 0x0001f300
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

#define compatible_units(unitp1,unitp2) (((unitp1)&0xffff0000)==((unitp2)&0xffff0000))
#define same_unit(unitp1,unitp2) (((unitp1)&0xffffff00)==((unitp2)&0xffffff00))

#define badunits 4
// error thrown by conversion routine

int is_exact(double measurement, unsigned int unitp,int is_toler);
/* Checks whether the given measurement is exact in that unit.
   If the measurement is a tolerance and is divisible by 127, returns false;
   this means that a tolerance in inches is being checked in millimeters. */
char *format_meas(double measurement, unsigned int unitp);
char *format_meas_unit(double measurement, unsigned int unitp);
double parse_meas(char *meas, int unitp,int *found_unit);
/* Given a string representing a measurement, in the unit unitp unless specified otherwise,
   returns its value in the program's internal unit. */
char *trim(char *str);
/* Removes spaces from both ends of a string in place. */
void setfoot(int f);
/* Switches between international and survey feet. */
void set_length_unit(int unitp);
double precision(int unitp);
/* Converts measurements to and from coherent SI units (internal units)
 * without formatting to a precision. Used when writing coordinate files
 * that may be in feet or meters.
 */
double from_coherent(double measurement,int unitp);
double to_coherent(double measurement,int unitp);
double from_coherent_length(double measurement);
double to_coherent_length(double measurement);
double parse_length(const char *meas);
double parse_length(std::string meas);
std::string format_length(double measurement);
std::string format_length_unit(double measurement);

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
  void setDefaultUnit(int quantity,double magnitude);
  void setDefaultPrecision(int quantity,double magnitude);
  int findUnit(int quantity,double magnitude=0);
  int findPrecision(int unit,double magnitude=0);
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
