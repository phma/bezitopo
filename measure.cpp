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
#include "manysum.h"
using namespace std;

bool isnumeric(int ch,int i)
{
  return (ch>='0' && ch<='9') || ch=='.' || (i==0 && (ch=='-' || ch=='+'));
}

struct cf
{
  int64_t unitp;
  double factor;
};

cf cfactors[]=
{
  FOOT,			0.3048,			// ft (any of three)
  CHAIN,		20.1168,		// ch (any of three)
  MILE,			1609.344,		// mi (any of three)
  0,			1,			// unknown unit
  MILLIMETER,		0.001,			// mm
  MICROMETER,		0.000001,		// μm
  KILOMETER,		1000,			// km
  METER,		1,			// m
  SQUAREMETER,		1,			// m²
  SQUAREFOOT,		0.09290304,		// ft²
  HECTARE,		10000,			// ha
  ACRE,			4046.8564224,		// ac
  GRAM,			0.001,			// g
  KILOGRAM,		1.0,			// kg
  POUND,		0.45359237,		// lb
  DIOPTER,		1,
  MILLIDIOPTER,		0.001,
  SQUAREDIOPTER,	1,
  SQUAREMILLIDIOPTER,	1e-6,
  HOUR,			3600,			// hour
  RADIAN,		1,
  DEGREE,		M_PI/180,
  GON,			M_PI/200,
  ARCMINUTE,		M_PI/10800,
  ARCSECOND,		M_PI/648000,
  RADIAN_B,		1073741824./M_PI,
  DEGREE_B,		1073741824./180,
  GON_B,		1073741824./200,
  ARCMINUTE_B,		1073741824./10800,
  ARCSECOND_B,		1073741824./648000,
};
#define nunits (sizeof(cfactors)/sizeof(struct cf))
struct symbol
{
  int64_t unitp;
  char symb[12];
};

symbol symbols[]=
/* The first symbol is the canonical one, if there is one. */
{
  MILLIMETER,		"mm",
  MICROMETER,		"µm", //0000b5 00006d
  MICROMETER,		"μm", //0003bc 00006d
  MICROMETER,		"um",
  KILOMETER,		"km",
  MILE,			"mi",
  FOOT,			"ft",
  FOOT,			"'",
  METER,		"m",
  SQUAREMETER,		"m²",
  SQUAREFOOT,		"ft²",
  HECTARE,		"ha",
  ACRE,			"ac",
  GRAM,			"g",
  KILOGRAM,		"kg",
  POUND,		"lb",
  DIOPTER,		"dpt",
  MILLIDIOPTER,		"mdpt",
  SQUAREDIOPTER,	"dpt²",
  SQUAREMILLIDIOPTER,	"mdpt²",
  RADIAN,		"rad",
  DEGREE,		"°",
  GON,			"gon",
  ARCMINUTE,		"′",
  ARCSECOND,		"″",
  RADIAN_B,		"rad",
  DEGREE_B,		"°",
  GON_B,		"gon",
  ARCMINUTE_B,		"′",
  ARCSECOND_B,		"″",
};
#define nsymbols (sizeof(symbols)/sizeof(struct symbol))

int64_t length_unit=METER;
double length_factor=1;

int fequal(double a, double b)
/* Returns true if they are equal within reasonable precision. */
{return (fabs(a-b)<fabs(a+b)*1e-13);
 }

int is_int(double a)
{
  return fequal(a,rint(a));
}

double cfactor(int64_t unitp)
{
  int i;
  for (i=0;i<nunits;i++)
    if (sameUnit(unitp,cfactors[i].unitp))
      return cfactors[i].factor;
  //fprintf(stdout,"Conversion factor for %x missing!\n",unitp);
  return NAN;
}

const char *symbol(int64_t unitp)
{
  int i;
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

unsigned short basecodes[][2]=
{
  { 2,0x100},
  { 6,0x200},
  { 8,0x240},
  {10,0x280},
  {12,0x2c0},
  {16,0x300},
  {20,0x340},
  {60,0x360},
  { 0,0x380}
};
#define nbasecodes (sizeof(basecodes)/sizeof(basecodes[0]))

BasePrecision basePrecision(int64_t unitp)
{
  BasePrecision ret;
  int exp,basecode,i;
  ret.notation=(unitp&0xf000)>>12;
  basecode=unitp&0xfff;
  if (basecode==0)
    basecode=DECIMAL;
  for (i=0;i<nbasecodes;i++)
  {
    if (basecodes[i][1]<=basecode)
    {
      ret.base=basecodes[i][0];
      ret.power=basecode-basecodes[i][1];
    }
    if (basecodes[i][1]==basecode+1)
      ret.power=-1;
  }
  return ret;
}

double precision(int64_t unitp)
/* Returns the precision (>=1) of unitp.
   280 1
   281 10
   282 100
   ...
   28f 1000000000000000
   100 1
   101 2
   102 4
   ...
   10f 32768
   360 1
   361 60
   362 3600
   362 216000
   */
{
  double base,p;
  int exp,basecode,i;
  basecode=unitp&0xfff; // Nybble 0xf000 indicates whether to use bigger units
  for (i=0;i<nbasecodes;i++)
    if (basecodes[i][1]<=basecode)
    {
      base=basecodes[i][0];
      exp=basecode-basecodes[i][1];
    }
  for (p=1,i=0;i<exp;i++)
    p*=base;
  return p;
}

int64_t moreprecise(int64_t unitp1,int64_t unitp2)
/* Given two unitp codes, returns the more precise. If one of them has no
   conversion factor, or they are of different quantities, it still returns
   one of them, but which one may not make sense. */
{
  double factor1,factor2;
  factor1=cfactor(unitp1)/precision(unitp1);
  factor2=cfactor(unitp2)/precision(unitp2);
  if (factor1<factor2)
    return unitp1;
  else
    return unitp2;
}

vector<int> largerUnitFactors(int64_t unitp)
{
  vector<int> ret;
  unitp=physicalUnit(unitp);
  switch (unitp)
  {
    case ARCSECOND:
    case ARCSECOND_B:
      ret.push_back(60);
    case ARCMINUTE:
    case ARCMINUTE_B:
      ret.push_back(60);
      break;
  }
  return ret;
}

vector<string> largerUnitSymbols(int64_t unitp)
{
  vector<string> ret;
  unitp=physicalUnit(unitp);
  switch (unitp)
  {
    case ARCSECOND:
    case ARCSECOND_B:
      ret.push_back("′");
    case ARCMINUTE:
    case ARCMINUTE_B:
      ret.push_back("°");
      break;
  }
  return ret;
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

int64_t parseSymbol(string unitStr,int64_t quantity)
{
  int i;
  int64_t ret=0;
  for (i=0;i<nsymbols;i++)
    if (unitStr==symbols[i].symb && !compatibleUnits(ret,quantity))
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

void Measure::addUnit(int64_t unit)
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

void Measure::removeUnit(int64_t unit)
{
  int i;
  int found=-1;
  for (i=0;i<availableUnits.size();i++)
    if (sameUnit(availableUnits[i],unit))
      found=i;
  if (found+1)
  {
    if (found+1<availableUnits.size())
      swap(availableUnits[found],availableUnits.back());
    availableUnits.resize(availableUnits.size()-1);
  }
}

void Measure::clearUnits(int64_t quantity)
{
  int i;
  vector<int> found;
  for (i=availableUnits.size()-1;i>=0;i--)
    if (quantity==0 || compatibleUnits(availableUnits[i],quantity))
      found.push_back(i);
  for (i=0;i<found.size();i++)
  {
    if (found[i]+1<availableUnits.size())
      swap(availableUnits[found[i]],availableUnits.back());
    availableUnits.resize(availableUnits.size()-1);
  }
  availableUnits.shrink_to_fit();
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
  clearUnits(LENGTH);
  clearUnits(AREA);
  clearUnits(MASS);
  addUnit(MILLIMETER);
  addUnit(METER);
  addUnit(KILOMETER);
  addUnit(SQUAREMETER);
  addUnit(HECTARE);
  addUnit(GRAM);
  addUnit(KILOGRAM);
  addUnit(DIOPTER);
  addUnit(MILLIDIOPTER);
  addUnit(SQUAREDIOPTER);
  addUnit(SQUAREMILLIDIOPTER);
}

void Measure::setCustomary()
{
  clearUnits(LENGTH);
  clearUnits(AREA);
  clearUnits(MASS);
  addUnit(FOOT);
  addUnit(CHAIN);
  addUnit(MILE);
  addUnit(SQUAREFOOT);
  addUnit(ACRE);
  addUnit(POUND);
}

void Measure::setDefaultUnit(int64_t quantity,double magnitude)
{
  defaultUnit[quantity&0xffff00000000]=magnitude;
}

void Measure::setDefaultPrecision(int64_t quantity,double magnitude)
{
  defaultPrecision[quantity&0xffff00000000]=magnitude;
}

int64_t Measure::findUnit(int64_t quantity,double magnitude)
/* Finds the available unit of quantity closest to magnitude.
 * E.g. if magnitude is 0.552, quantity is LENGTH, and available units
 * are the meter with all prefixes, returns METER.
 * If magnitude is 0.552, quantity is LENGTH, and available units
 * include INCH, FOOT, ROD, and MILE, returns FOOT. But if you make
 * the yard available, returns YARD, which is closer to 0.552 m.
 * Ignores units of mass, time, and anything other than length.
 */
{
  int i;
  int64_t closeUnit=0;
  double unitRatio,maxUnitRatio=-1;
  if (magnitude<=0)
    magnitude=defaultUnit[quantity&0xffff00000000];
  for (i=0;i<availableUnits.size();i++)
    if (compatibleUnits(availableUnits[i],quantity))
    {
      unitRatio=magnitude/conversionFactors[availableUnits[i]&0xffffffff0000];
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

int Measure::findPrecision(int64_t unit,double magnitude)
/* This returns a number of decimal places. If sexagesimal or binary places
 * are needed, I'll do them later.
 */
{
  double factor;
  int ret;
  BasePrecision bp;
  if ((unit&0xffff0000)==0)
    unit=findUnit(unit);
  bp=basePrecision(unit);
  if (magnitude<=0)
    magnitude=defaultPrecision[physicalQuantity(unit)];
  factor=conversionFactors[physicalUnit(unit)];
  if (factor<=0 || std::isnan(factor) || std::isinf(factor))
    factor=1;
  if (bp.power<0)
    ret=rint(log10(factor/magnitude));
  else
    ret=bp.power;
  if (ret<0)
    ret=0;
  return ret;
}

double Measure::toCoherent(double measurement,int64_t unit,double unitMagnitude)
{
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  return measurement*conversionFactors[unit];
}

double Measure::fromCoherent(double measurement,int64_t unit,double unitMagnitude)
{
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  return measurement/conversionFactors[unit];
}

string Measure::formatMeasurement(double measurement,int64_t unit,double unitMagnitude,double precisionMagnitude)
{
  int prec,len,i;
  BasePrecision bp;
  vector<double> m;
  vector<int> luf;
  vector<string> lus;
  char *pLcNumeric;
  string saveLcNumeric,ret;
  vector<char> format,output;
  vector<string> formats;
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  bp=basePrecision(unit);
  prec=findPrecision(unit,precisionMagnitude);
  pLcNumeric=setlocale(LC_NUMERIC,nullptr);
  if (pLcNumeric)
    saveLcNumeric=pLcNumeric;
  setlocale(LC_NUMERIC,"C");
  format.resize(8);
  m.push_back(measurement/conversionFactors[physicalUnit(unit)]);
  if (bp.notation==2)
  {
    luf=largerUnitFactors(unit);
    lus=largerUnitSymbols(unit);
    for (i=0;i<luf.size();i++)
    {
      m.push_back(trunc(m[i]/luf[i]));
      m[i]=fabs(m[i]-m[i+1]*luf[i]);
    }
  }
  for (i=0;i<m.size();i++)
  {
    len=snprintf(&format[0],format.size(),"%%0%d.%df",(i<m.size()-1)?2:0,i?0:prec);
    if (len+1>format.size())
    {
      format.resize(len+1);
      len=snprintf(&format[0],format.size(),"%%0%d.%df",(i<m.size()-1)?2:0,i?0:prec);
    }
    formats.push_back(string(&format[0]));
  }
  if (localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  output.resize(8);
  for (i=0;i<m.size();i++)
  {
    len=snprintf(&output[0],output.size(),formats[i].c_str(),m[i]);
    if (len+1>output.size())
    {
      output.resize(len+1);
      len=snprintf(&output[0],output.size(),formats[i].c_str(),m[i]);
    }
    if (i)
      ret=lus[i-1]+ret;
    ret=string(&output[0])+ret;
  }
  if (!localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  return ret;
}

string Measure::formatMeasurementUnit(double measurement,int64_t unit,double unitMagnitude,double precisionMagnitude)
{
  bool space=true;
  string unitSymbol;
  if ((unit&0xffff)==0)
    unit=findUnit(unit,unitMagnitude);
  if (sameUnit(unit,DEGREE) || sameUnit(unit,ARCMINUTE) || sameUnit(unit,ARCSECOND) ||
      sameUnit(unit,DEGREE_B) || sameUnit(unit,ARCMINUTE_B) || sameUnit(unit,ARCSECOND_B))
    space=false;
  unitSymbol=symbol(unit);
  if (space)
    unitSymbol=' '+unitSymbol;
  return formatMeasurement(measurement,unit,unitMagnitude,precisionMagnitude)+unitSymbol;
}

Measurement Measure::parseMeasurement(string measStr,int64_t quantity)
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
  string saveLcNumeric;
  vector<string> numberStr,unitStr;
  vector<double> valueInUnit,conversionFactor,coherentValue;
  vector<int64_t> unit;
  vector<int> luf;
  size_t endOfNumber;
  int i,j,ch,lastch=-1;
  Measurement ret;
  pLcNumeric=setlocale(LC_NUMERIC,nullptr);
  if (pLcNumeric)
    saveLcNumeric=pLcNumeric;
  if (!localized)
    setlocale(LC_NUMERIC,"C");
  for (i=j=0;i<measStr.length();i++)
  {
    ch=(unsigned char)measStr[i];
    if (isnumeric(ch,i))
    {
      if (lastch>=0 && !isnumeric(lastch,i-1))
	j++;
      while (j>=numberStr.size())
	numberStr.push_back("");
      while(j>=unitStr.size())
	unitStr.push_back("");
      numberStr[j]+=(char)ch;
    }
    else
    {
      unitStr[j]+=(char)ch;
    }
    lastch=ch;
  }
  try
  {
    for (i=0;i<numberStr.size();i++)
    {
      valueInUnit.push_back(stod(numberStr[i],&endOfNumber)); // TODO later: handle 12+3/8 when needed
      if (valueInUnit[0]<0 && valueInUnit.back()>0)
	valueInUnit.back()*=-1;
    }
  }
  catch (...)
  {
    throw badNumber;
  }
  /* If the string has multiple unit symbols, like 34°27'18", and they belong
   * to the same physical quantity which does not disagree with quantity, it is
   * valid. If the string has hyphens between numbers, but ends with a unit
   * symbol that has larger unit factors, it is valid. However, if it has
   * hyphens but no unit symbol, it is valid only if findUnit returns a unit
   * that has larger unit factors. So 24-45, when expecting a distance in meters,
   * is invalid, but when expecting an angle in minutes or seconds, is valid.
   */
  for (i=0;i<unitStr.size();i++)
  {
    trim(unitStr[i]);
    if (unitStr[i]=="-")
      unit.push_back(1);
    else if (unitStr[i].length())
      unit.push_back(parseSymbol(unitStr[i],quantity));
    else
      unit.push_back(findUnit(quantity));
  }
  ret.unit=unit.back();
  conversionFactor=valueInUnit;
  for (i=0;i<unit.size();i++)
    if (physicalUnit(unit[i]))
      conversionFactor[i]=conversionFactors[physicalUnit(unit[i])];
    else
      conversionFactor[i]=0;
  luf=largerUnitFactors(ret.unit);
  for (i=conversionFactor.size()-2;i>=0;i--)
    if (conversionFactor[i]==0 && conversionFactor.size()-2-i<luf.size())
      conversionFactor[i]=conversionFactor[i+1]*luf[conversionFactor.size()-2-i];
  if (!localized)
    setlocale(LC_NUMERIC,saveLcNumeric.c_str());
  for (i=0;i<valueInUnit.size();i++)
  {
    coherentValue.push_back(valueInUnit[i]*conversionFactor[i]);
    if (conversionFactor[i]==0 || (unit[i]>1 && !compatibleUnits(ret.unit,unit[i])))
      throw badUnits;
  }
  if (ret.unit==0 || (quantity>0 && !compatibleUnits(ret.unit,quantity)))
    throw badUnits;
  ret.magnitude=pairwisesum(coherentValue);
  return ret;
}

xy Measure::parseXy(string xystr)
{
  size_t pos;
  string xstr,ystr;
  xy ret;
  Measurement xmeas,ymeas;
  pos=xystr.find(',');
  if (pos==string::npos)
    ret=xy(NAN,NAN);
  else
  {
    xstr=xystr.substr(0,pos);
    ystr=xystr.substr(pos+1);
    xmeas=parseMeasurement(xstr,LENGTH);
    ymeas=parseMeasurement(ystr,LENGTH);
    ret=xy(xmeas.magnitude,ymeas.magnitude);
  }
  return ret;
}

void Measure::writeXml(ostream &ofile)
{
  int i;
  map<int64_t,double>::iterator j;
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
