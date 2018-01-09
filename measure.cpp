/******************************************************/
/*                                                    */
/* measure.cpp - measuring units                      */
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
/* The unit of length can be the foot, chain, or meter. Independently
 * of this, the foot can be selected from the international foot,
 * the US survey foot, or maybe the Indian survey foot. Selecting
 * one of these feet affects the following units:
 * Length: foot, chain, mile.
 * Area: square foot, acre.
 * Volume: cubic foot, cubic yard, acre-foot. The gallon is unaffected.
 * The scale (see ps.cpp) depends on whether the unit is the foot,
 * but does not depend on which foot is in use. 1"=30' is 1:360, even
 * if the foot is the survey foot (it is not 1 international inch
 * = 1 survey foot).
 */

#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "measure.h"
#include "angle.h"
#include "except.h"
#include "ldecimal.h"
using namespace std;

struct cf
{
  int unitp;
  double factor;
};

cf cfactors[]=
{
  FOOT,		0.3048,			// ft (any of three)
  CHAIN,	20.1168,		// ch (any of three)
  MILE,		1609.344,		// mi (any of three)
  0,		1,			// unknown unit
  MILLIMETER,	0.001,			// mm
  MICROMETER,	0.000001,		// μm
  KILOMETER,	1000,			// km
  METER,	1,			// m
  SQUAREMETER,	1,			// m²
  SQUAREFOOT,	0.09290304,		// ft²
  HECTARE,	10000,			// ha
  ACRE,		4046.8564224,		// ac
  GRAM,		0.001,			// g
  KILOGRAM,	1.0,			// kg
  POUND,	0.45359237,		// lb
  MILLIPOUND,	0.00045359237,		// pound per thousand
  HOUR,		3600,			// hour
  KGPERL,	1000.0,			// kg/l
  LBPERIN3,	27679.90471020312,	// lb/in3
  PERKG,	1.0,			// $/kg
  PERPOUND,	2.2046226218487759,	// $/lb
  PERMETER,	1.0,			// $/m
  PERFOOT,	3.2808398950131233,	// $/ft
  PERLITER,	1000,			// /L
  EACH,		1,			// $/piece
  PERHUNDRED,	0.01,			// $/hundred pieces
  PERTHOUSAND,	0.001,			// $/thousand pieces
  PERLOT,	1,			// $/lot
  PERSET,	1,			// $/set
  ITEM,		1,
  THOUSAND,	1000,
  LOT,		1,
  SET,		1,
  PERMONTH,	0.38026486208173715e-6,
  PERYEAR,	31.688738506811427e-9,
  PERHOUR,	0.00027777777777777777,	// $/hour
  MILLILITER,	0.000001,
  IN3,		0.000016387064,
};
#define nunits (sizeof(cfactors)/sizeof(struct cf))
struct symbol
{
  int unitp;
  char symb[12];
};

symbol symbols[]=
/* The first symbol is the canonical one, if there is one. */
{
  MILLIMETER,	"mm",
  MICROMETER,	"µm", //0000b5 00006d
  MICROMETER,	"μm", //0003bc 00006d
  MICROMETER,	"um",
  KILOMETER,	"km",
  MILE,		"mi",
  FOOT,		"ft",
  FOOT,		"'",
  METER,	"m",
  SQUAREMETER,	"m²",
  SQUAREFOOT,	"ft²",
  HECTARE,	"ha",
  ACRE,		"ac",
  GRAM,		"g",
  KILOGRAM,	"kg",
  POUND,	"lb",
  KGPERL,	"kg/l",
  KGPERL,	"kg/L",
  KGPERL,	"g/ml",
  KGPERL,	"g/mL",
  LBPERIN3,     "lb/in³",
  MILLILITER,	"ml",
  IN3,		"in³",
};
#define nsymbols (sizeof(symbols)/sizeof(struct symbol))

int length_unit=METER;
double length_factor=1;

int fequal(double a, double b)
/* Returns true if they are equal within reasonable precision. */
{return (fabs(a-b)<fabs(a+b)*1e-13);
 }

int is_int(double a)
{return fequal(a,rint(a));
 }

double cfactor(int unitp)
{int i;
 for (i=0;i<nunits;i++)
     if (sameUnit(unitp,cfactors[i].unitp))
        return cfactors[i].factor;
 //fprintf(stdout,"Conversion factor for %x missing!\n",unitp);
 return NAN;
 }

char *symbol(int unitp)
{int i;
 for (i=0;i<nsymbols;i++)
     if (sameUnit(unitp,symbols[i].unitp))
        return symbols[i].symb;
 return "unk";
 }

char *trim(char *str)
/* Removes spaces from both ends of a string in place. */
{char *pos,*spos;
 for (pos=spos=str;*pos;pos++)
     if (!isspace(*pos))
        spos=pos+1;
 *spos=0;
 for (pos=str;isspace(*pos);pos++);
 memmove(str,pos,strlen(pos)+1);
 return(str);
 }

char *collapse(char *str)
/* Collapses multiple spaces into single spaces. */
{char *pos,*spos;
 for (pos=spos=str;*pos;pos++)
     if (!isspace(*pos) || !isspace(pos[1]))
        *spos++=*pos;
 *spos=0;
 return(str);
 }

double precision(int unitp)
/* Returns the precision (>=1) of unitp. Base codes 3-f are returned as 0
   and must be handled specially by the formatter.
   00 1
   01 10
   02 100
   ...
   0f 1000000000000000
   10 1
   11 2
   12 4
   ...
   1f 32768
   20 1
   21 60
   22 3600
   22 216000
   23 10
   24 600   e.g. 63°26.1′
   25 36000
   ...
   2f 216000000
   30-3f Used for mixed units.
   40-ff Undefined.
   */
{
  double base,p;
  int exp,basecode,i;
  exp=unitp&0xf;
  basecode=unitp&0xf0;
  switch (basecode)
  {
    case 0:
      base=10;
      break;
    case 16:
      base=2;
      break;
    case 32:
      base=60;
      break;
    default:
      base=0;
  }
  if (base==60)
  {
    for (p=1,i=0;i<(exp&3);i++)
      p*=base;
    for (i=0;i<(exp&12);i+=4)
      p*=10;
  }
  else
    for (p=1,i=0;i<exp;i++)
      p*=base;
  return p;
}

int moreprecise(int unitp1,int unitp2)
/* Given two unitp codes, returns the more precise. If one of them has no
   conversion factor, or they are of different quantities, it still returns
   one of them, but which one may not make sense. */
{double factor1,factor2;
 factor1=cfactor(unitp1)/precision(unitp1);
 factor2=cfactor(unitp2)/precision(unitp2);
 if (factor1<factor2)
    return unitp1;
 else
    return unitp2;
 }

void trim(string &str)
{
  size_t pos;
  pos=str.find_first_not_of(' ');
  if (pos<=str.length())
    str.erase(0,pos);
  pos=str.find_last_not_of(' ');
  if (pos<=str.length())
    str.erase(pos+1);
}

int parseSymbol(string unitStr)
{
  int i,ret=0;
  for (i=0;i<nsymbols;i++)
    if (unitStr==symbols[i].symb)
      ret=symbols[i].unitp;
  return ret;
}

Measure::Measure()
{
  int i;
  for (i=0;i<sizeof(cfactors)/sizeof(cf);i++)
    conversionFactors[cfactors[i].unitp]=cfactors[i].factor;
  whichFoot=INTERNATIONAL;
  localized=false;
}

void Measure::setFoot(int which)
{
  switch (which)
  {
    case (INTERNATIONAL):
      conversionFactors[FOOT]=0.3048;
      break;
    case (USSURVEY):
      conversionFactors[FOOT]=12e2/3937;
      break;
    case (INSURVEY):
      conversionFactors[FOOT]=0.3047996;
  }
  conversionFactors[CHAIN]=conversionFactors[FOOT]*66;
  conversionFactors[MILE]=conversionFactors[CHAIN]*80;
  conversionFactors[SQUAREFOOT]=sqr(conversionFactors[FOOT]);
  conversionFactors[ACRE]=conversionFactors[SQUAREFOOT]*66*660;
  whichFoot=which;
}

int Measure::getFoot()
{
  return whichFoot;
}

void Measure::addUnit(int unit)
{
  int i;
  bool found=false;
  for (i=0;i<availableUnits.size();i++)
    if (sameUnit(availableUnits[i],unit))
    {
      availableUnits[i]=unit;
      found=true;
    }
  if (!found)
    availableUnits.push_back(unit);
}

void Measure::removeUnit(int unit)
{
  int i;
  int found=-1;
  for (i=0;i<availableUnits.size();i++)
    if (sameUnit(availableUnits[i],unit))
      found=i;
  if (found+1)
  {
    if (found+1<availableUnits.size())
      swap(availableUnits[i],availableUnits.back());
    availableUnits.resize(availableUnits.size()-1);
  }
}

void Measure::clearUnits()
{
  availableUnits.clear();
}

void Measure::localize(bool loc)
/* If loc is true, and your locale says that commas are used as decimal points,
 * it will convert measurements using commas and expect commas when parsing.
 * When reading and writing data files, turn it off. Turn on only when dealing
 * with the user.
 */
{
  localized=loc;
}

void Measure::setMetric()
{
  clearUnits();
  addUnit(MILLIMETER);
  addUnit(METER);
  addUnit(KILOMETER);
  addUnit(SQUAREMETER);
  addUnit(HECTARE);
  addUnit(GRAM);
  addUnit(KILOGRAM);
}

void Measure::setCustomary()
{
  clearUnits();
  addUnit(FOOT);
  addUnit(CHAIN);
  addUnit(MILE);
  addUnit(SQUAREFOOT);
  addUnit(ACRE);
  addUnit(POUND);
}

void Measure::setDefaultUnit(int quantity,double magnitude)
{
  defaultUnit[quantity&0xffff0000]=magnitude;
}

void Measure::setDefaultPrecision(int quantity,double magnitude)
{
  defaultPrecision[quantity&0xffff0000]=magnitude;
}

int Measure::findUnit(int quantity,double magnitude)
/* Finds the available unit of quantity closest to magnitude.
 * E.g. if magnitude is 0.552, quantity is LENGTH, and available units
 * are the meter with all prefixes, returns METER.
 * If magnitude is 0.552, quantity is LENGTH, and available units
 * include INCH, FOOT, ROD, and MILE, returns FOOT. But if you make
 * the yard available, returns YARD, which is closer to 0.552 m.
 * Ignores units of mass, time, and anything other than length.
 */
{
  int i,closeUnit=0;
  double unitRatio,maxUnitRatio=-1;
  if (magnitude<=0)
    magnitude=defaultUnit[quantity&0xffff0000];
  for (i=0;i<availableUnits.size();i++)
    if (compatibleUnits(availableUnits[i],quantity))
    {
      unitRatio=magnitude/conversionFactors[availableUnits[i]&0xffffff00];
      if (unitRatio>1)
        unitRatio=1/unitRatio;
      if (unitRatio>maxUnitRatio)
      {
        maxUnitRatio=unitRatio;
        closeUnit=availableUnits[i];
      }
    }
  return closeUnit;
}

int Measure::findPrecision(int unit,double magnitude)
/* This returns a number of decimal places. If sexagesimal or binary places
 * are needed, I'll do them later.
 */
{
  double factor;
  int ret;
  if ((unit&0xff00)==0)
    unit=findUnit(unit);
  if (magnitude<=0)
    magnitude=defaultPrecision[unit&0xffff0000];
  factor=conversionFactors[unit];
  if (factor<=0 || std::isnan(factor) || std::isinf(factor))
    factor=1;
  ret=rint(log10(factor/magnitude));
  if (ret<0)
    ret=0;
  return ret;
}

double Measure::toCoherent(double measurement,int unit,double unitMagnitude)
{
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  return measurement*conversionFactors[unit];
}

double Measure::fromCoherent(double measurement,int unit,double unitMagnitude)
{
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  return measurement/conversionFactors[unit];
}

string Measure::formatMeasurement(double measurement,int unit,double unitMagnitude,double precisionMagnitude)
{
  int prec,len;
  double m;
  char *pLcNumeric;
  string saveLcNumeric;
  vector<char> format,output;
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  prec=findPrecision(unit,precisionMagnitude);
  pLcNumeric=setlocale(LC_NUMERIC,nullptr);
  if (pLcNumeric)
    saveLcNumeric=pLcNumeric;
  setlocale(LC_NUMERIC,"C");
  format.resize(8);
  len=snprintf(&format[0],format.size(),"%%.%df",prec);
  if (len+1>format.size())
  {
    format.resize(len+1);
    len=snprintf(&format[0],format.size(),"%%.%df",prec);
  }
  m=measurement/conversionFactors[unit];
  if (localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  output.resize(8);
  len=snprintf(&output[0],output.size(),&format[0],m);
  if (len+1>output.size())
  {
    output.resize(len+1);
    len=snprintf(&output[0],output.size(),&format[0],m);
  }
  if (!localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  return string(&output[0]);
}

string Measure::formatMeasurementUnit(double measurement,int unit,double unitMagnitude,double precisionMagnitude)
{
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  return formatMeasurement(measurement,unit,unitMagnitude,precisionMagnitude)+' '+symbol(unit);
}

Measurement Measure::parseMeasurement(string measStr,int quantity)
/* quantity is a code for physical quantity (e.g. LENGTH) or 0, which means
 * accept any unit. If quantity is nonzero and no unit symbol is supplied,
 * it assumes whatever available unit of that quantity is closest to that
 * quantity's magnitude (see setDefaultUnit). If quantity is zero, a unit
 * symbol must be supplied. If quantity is 0 and there is no unit symbol, or
 * quantity is nonzero and the unit symbol doesn't match it, throws badUnits.
 * If the number is missing, throws badNumber.
 */
{
  char *pLcNumeric;
  string saveLcNumeric,unitStr;
  double valueInUnit;
  size_t endOfNumber;
  Measurement ret;
  pLcNumeric=setlocale(LC_NUMERIC,nullptr);
  if (pLcNumeric)
    saveLcNumeric=pLcNumeric;
  if (!localized)
    setlocale(LC_NUMERIC,"C");
  try
  {
    valueInUnit=stod(measStr,&endOfNumber); // TODO later: handle 12+3/8 when needed
  }
  catch (...)
  {
    throw badNumber;
  }
  unitStr=measStr.substr(endOfNumber);
  trim(unitStr);
  if (unitStr.length())
    ret.unit=parseSymbol(unitStr);
  else
    ret.unit=findUnit(quantity);
  if (!localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  if (ret.unit==0 || (quantity>0 && !compatibleUnits(ret.unit,quantity)))
    throw badUnits;
  ret.magnitude=valueInUnit*conversionFactors[ret.unit];
  return ret;
}

void Measure::writeXml(ostream &ofile)
{
  int i;
  map<int,double>::iterator j;
  ofile<<"<Measure foot="<<whichFoot;
  if (localized)
    ofile<<" localized";
  ofile<<"><availableUnits>"; // No need to output conversion factors; they're implied by foot.
  for (i=0;i<availableUnits.size();i++)
  {
    if (i)
      ofile<<' ';
    ofile<<availableUnits[i];
  }
  ofile<<"</availableUnits><defaultUnit>";
  for (i=0,j=defaultUnit.begin();j!=defaultUnit.end();++i,++j)
  {
    if (i)
      ofile<<' ';
    ofile<<j->first<<':'<<ldecimal(j->second);
  }
  ofile<<"</defaultUnit><defaultPrecision>";
  for (i=0,j=defaultPrecision.begin();j!=defaultPrecision.end();++i,++j)
  {
    if (i)
      ofile<<' ';
    ofile<<j->first<<':'<<ldecimal(j->second);
  }
  ofile<<"</defaultPrecision></Measure>"<<endl;
}
