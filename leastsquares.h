/******************************************************/
/*                                                    */
/* leastsquares.h - least-squares adjustment          */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include "quaternion.h"
#include "xyz.h"

struct Pole
{
  xyz displacement;
  double height;
}

class PuppetBar
/* So called because the lines from a total station to targets shot, together
 * with the poles the targets are on, resemble a marionette control bar
 * with strings dangling from it. As the PuppetBar is rotated and tilted,
 * the rods remain vertical.
 */
{
public:
  xyz unmeasuredBacksight; // Set to NaN if backsight was measured.
  int kind; // total station or GPS, possibly other kinds
  Quaternion orientation;
  vector<Pole> pole;
};
