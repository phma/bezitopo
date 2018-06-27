/******************************************************/
/*                                                    */
/* objlist.cpp - list of drawing objects              */
/*                                                    */
/******************************************************/
/* Copyright 2015,2018 Pierre Abbat.
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

#include "objlist.h"
#include "document.h"
#include "penwidth.h"
#include "layer.h"
#include "color.h"

using namespace std;

unsigned short objrec::getlinetype(document *doc)
{
  if (ltype==SAMELINETYPE)
    return doc->layers[layr].ltype;
  else
    return ltype;
}

unsigned short objrec::getcolor(document *doc)
{
  if (colr==SAMECOLOR)
    return doc->layers[layr].colr;
  else
    return colr;
}

unsigned short objrec::getthickness(document *doc)
{
  if (thik>200) // see penwidth.h
    return doc->layers[layr].thik;
  else
    return thik;
}

