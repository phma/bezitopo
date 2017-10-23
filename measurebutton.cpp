/******************************************************/
/*                                                    */
/* measurebutton.cpp - buttons for measuring          */
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
#include <iostream>
#include "measurebutton.h"

using namespace std;

MeasureButton::MeasureButton(QObject *parent,int lunit,int whichFoot):QAction(parent)
/* lunit is:
 * 0 if this button selects which foot is in effect;
 * FOOT if this button selects feet;
 * METER if this button selects meters.
 */
{
  setCheckable(true);
  myUnit=lunit;
  myFoot=whichFoot;
}

void MeasureButton::setMeasure(Measure newMeasure)
{
  if (myUnit)
    setChecked(myUnit==newMeasure.findUnit(LENGTH,0.552));
  else
  {
    setEnabled(METER!=newMeasure.findUnit(LENGTH,0.552));
    setChecked(myFoot==newMeasure.getFoot());
  }
}
