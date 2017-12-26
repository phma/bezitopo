/******************************************************/
/*                                                    */
/* except.cpp - exceptions                            */
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
#include "except.h"

char exceptionNames[N_EXCEPTIONS][24]=
{
  "noerror",
  "notri",
  "samepnts",
  "flattri",
  "badheader",
  "baddata",
  "matrixmismatch",
  "singularmatrix",
  "unsetgeoid",
  "unsetsource",
  "badunits",
  "badnumber",
  "badbreaklineend",
  "breaklinescross",
  "badbreaklineformat",
  "fileerror"
};

BeziExcept::BeziExcept(const BeziExcept &a)
{
  exceptNumber=a.exceptNumber;
  pointNumber[0]=a.pointNumber[0];
  pointNumber[1]=a.pointNumber[1];
}

BeziExcept::BeziExcept(int num)
{
  exceptNumber=num;
  pointNumber[0]=pointNumber[1]=0;
}
