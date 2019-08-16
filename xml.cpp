/******************************************************/
/*                                                    */
/* xml.cpp - parse XML                                */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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
/* This is a quick&dirty module I'm hacking up to parse the XML in KML files.
 * The files are less than 200 kB, small enough to read into RAM and
 * parse there.
 */
#include "xml.h"

EntityReference entityTable[]=
{
  {"Tab",9},
  {"NewLine",10},
  {"excl",33},
  {"quot",34},
  {"QUOT",34},
  {"num",35},
  {"dollar",36},
  {"percnt",37},
  {"amp",38},
  {"AMP",38},
  {"apos",39},
  {"lpar",40},
  {"rpar",41},
  {"ast",42},
  {"midast",42},
  {"plus",43},
  {"comma",44},
  {"period",46},
  {"sol",47},
  {"colon",58},
  {"semi",59},
  {"lt",60},
  {"LT",60},
  {"equals",61},
  {"gt",62},
  {"GT",62},
  {"quest",63},
  {"commat",64},
  {"lsqb",91},
  {"lbrack",91},
  {"bsol",92},
  {"rsqb",93},
  {"rbrack",93},
  {"Hat",94},
  {"lowbar",95},
  {"grave",96},
  {"DiacriticalGrave",96},
  {"lcub",123},
  {"lbrace",123},
  {"verbar",124},
  {"vert",124},
  {"VerticalLine",124},
  {"rcub",125},
  {"rbrace",125},
  {"nbsp",128},
  {"NonBreakingSpace",128}
};
