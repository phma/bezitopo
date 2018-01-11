/******************************************************/
/*                                                    */
/* penwidth.cpp - pen width                           */
/*                                                    */
/******************************************************/
/* Copyright 2017-2018 Pierre Abbat.
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
#include <cassert>
#include <cmath>
#include "penwidth.h"
#define PENSTEP 1.07991744123616394147

double penWidthRatio(int penWidthCode)
{
  assert(penWidthCode<=200); // SAMEWIDTH should be replaced with an actual width code before calling this function.
  if (penWidthCode<-200)
    return 0;
  else
    return pow(PENSTEP,penWidthCode);
}

void setWidth(QPen &pen,short penWidthCode)
{
  pen.setWidthF(penWidthRatio(penWidthCode));
}
