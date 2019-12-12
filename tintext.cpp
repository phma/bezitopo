/******************************************************/
/*                                                    */
/* tintext.cpp - input TIN in AquaVeo text format     */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */
// https://www.xmswiki.com/wiki/TIN_Files

#include <fstream>
#include "tintext.h"
#include "binio.h"
#include "textfile.h"
#include "ldecimal.h"
using namespace std;

bool readTinText(string inputFile,pointlist &pl,double unit)
{
  ifstream file(inputFile);
  TextFile tfile(file);
  int i,n,tag,ptnum,corners[3];
  double x,y,z;
  bool good=false,cont=true;
  triangle *tri;
}
