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

#ifndef CONTOUR_H
#define CONTOUR_H
#include <vector>
#include "polyline.h"
#define CCHALONG 0.30754991027012474516361707317
// This is sqrt(4/27) of the way from 0.5 to 0. See clampcubic.

class pointlist;

class ContourInterval
{
  /* interval is in meters. When interval is in the display unit, they may be:
   * int fine coarse
   *  1    5     4
   *  2    5     5
   *  5    4     5
   *  1    1     5
   *  2    1     5
   *  5    1     4
   * with int multiplied by any power of 10. The coarse interval tells which
   * contours are labeled with elevations, the medium interval tells which
   * contours are completely drawn, and the fine interval, if different from
   * the medium, tells which contours are drawn only on nearly flat ground.
   */
public:
  double interval;
  int fineRatio,coarseRatio;
  double fineInterval()
  {
    return interval;
  };
  double mediumInterval()
  {
    return interval*fineRatio;
  };
  double coarseInterval()
  {
    return interval*fineRatio*coarseRatio;
  };
};

std::vector<uintptr_t> contstarts(pointlist &pts,double elev);
polyline trace(uintptr_t edgep,double elev);
polyline intrace(triangle *tri,double elev);
bool ismarked(uintptr_t ep);
void roughcontours(pointlist &pl,double conterval);
void smoothcontours(pointlist &pl,double conterval,bool log=false);
void checkedgediscrepancies(pointlist &pl);
#endif
