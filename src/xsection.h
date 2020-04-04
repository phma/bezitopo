/******************************************************/
/*                                                    */
/* xsection.h - cross sections                        */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
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

#include <vector>
/* A cross section consists of a central part, which is made of any number
 * of line segments or splines, and two margins, which are slopes extending
 * to infinity. The margins may be empty, single, or double. If they are
 * double, one of them (normally the lower) may start at a point which is
 * not the outermost.
 * 
 * The following operations can be performed on cross sections:
 * • Take a cross section of a TIN surface at a specified point and direction.
 * • Construct a cross section from scratch.
 * • Add a line segment or spline to either side of a cross section.
 * • Add or delete margins.
 * • Interpolate between two cross sections.
 * • Given a cross section with double margins and one with single or empty margins,
 *   extend the one with double margins until it meets the other.
 * • Compute the elevation anywhere along a cross section, including the
 *   margins if and only if they are single.
 */

class xsection
{
private:
  std::vector<double> xs,zs,ctrls;
  int center,lmstart,rmstart;
  double lmargin[2],rmargin[2];
};
