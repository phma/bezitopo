/******************************************************/
/*                                                    */
/* contour.h - generates contours                     */
/*                                                    */
/******************************************************/
/* Copyright 2012,2015,2016 Pierre Abbat.
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

#include <vector>
#include "tin.h"
#include "polyline.h"
#define CCHALONG 0.30754991027012474516361707317
// This is sqrt(4/27) of the way from 0.5 to 0. See clampcubic.

class pointlist;

std::vector<uintptr_t> contstarts(pointlist &pts,double elev);
polyline trace(uintptr_t edgep,double elev);
polyline intrace(triangle *tri,double elev);
bool ismarked(uintptr_t ep);
void roughcontours(pointlist &pl,double conterval);
void smoothcontours(pointlist &pl,double conterval,bool log=false);
void checkedgediscrepancies(pointlist &pl);
