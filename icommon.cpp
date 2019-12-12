/******************************************************/
/*                                                    */
/* icommon.cpp - common interactive routines          */
/*                                                    */
/******************************************************/
/* Copyright 2015-2019 Pierre Abbat.
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

#include <iostream>
#include "firstarg.h"
#include "icommon.h"
#include "measure.h"
#include "globals.h"

using namespace std;

bool subcont;

void setfoot_i(string args)
{
  args=trim(args);
  if (args=="int'l")
    doc.ms.setFoot(INTERNATIONAL);
  else if (args=="US")
    doc.ms.setFoot(USSURVEY);
  else if (args=="Indian")
    doc.ms.setFoot(INSURVEY);
  else
    cout<<"I don't recognize that foot"<<endl;
}

void setlengthunit_i(string args)
{
  args=trim(args);
  if (args=="m")
  {
    doc.ms.setMetric();
    doc.ms.setDefaultUnit(LENGTH,0.552); // geometric mean of meter and foot
    doc.ms.setDefaultPrecision(LENGTH,1.746e-3); // g.m. of 1 mm and 0.01 ft
  }
  else if (args=="ft")
  {
    doc.ms.setCustomary();
    doc.ms.setDefaultUnit(LENGTH,0.552);
    doc.ms.setDefaultPrecision(LENGTH,1.746e-3);
  }
  else if (args=="ch")
  {
    doc.ms.setCustomary();
    doc.ms.setDefaultUnit(LENGTH,14.18);
    doc.ms.setDefaultPrecision(LENGTH,0.1418);
  }
  else
    cout<<"I don't recognize that length unit"<<endl;
}

arangle parsearangle(string angstr,int unitp)
/* 13, angle to the right, relative
 * -13, angle to the left, relative
 * d13, deflection angle, same as 193, relative
 * n13e, bearing, absolute
 */
{
  arangle ret;
  int def=0;
  if (tolower(angstr[0])=='d' || !isalpha(angstr[0]))
  {
    ret.rel=true;
    if (tolower(angstr[0])=='d')
    {
      angstr.erase(0,1);
      def=DEG180;
    }
    ret.ang=-parsesignedangle(angstr,unitp);
  }
  else
  {
    ret.rel=false;
    ret.ang=parsebearing(angstr,unitp);
  }
  return ret;
}

xy parsexy(string xystr)
{
  return doc.ms.parseXy(xystr);
}

void subexit(string args)
{
  subcont=false;
}
