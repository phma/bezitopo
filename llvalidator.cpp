/******************************************************/
/*                                                    */
/* llvalidator.cpp - validator for latlong            */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include <iostream>
#include "llvalidator.h"
#include "latlong.h"
#include "angle.h"
using namespace std;

LlValidator::LlValidator(QObject *parent):QValidator(parent)
{
  /* This string consists of the abbreviations for north, east, south, and west,
   * in that order, repeated in upper and lower case if necessary. The
   * abbreviations may be more than one letter, so they are separated by spaces.
   * If any of the abbreviations is a lowercase 'g', it will be impossible
   * to specify an angle in gons.
   */
  setnesw(tr("N E S W").toStdString());
}

LlValidator::State LlValidator::validate(QString &input,int &pos) const
{
  State ret=Invalid;
  if (partialLatlong(input.toStdString()))
    ret=Intermediate;
  if (parselatlong(input.toStdString(),DEGREE).valid()==2)
    ret=Acceptable;
  if (ret==Invalid)
    cout<<'x';
  if (ret==Intermediate)
    cout<<'i';
  if (ret==Acceptable)
    cout<<'a';
  cout.flush();
  return ret;
}

