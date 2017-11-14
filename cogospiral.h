/******************************************************/
/*                                                    */
/* cogospiral.h - intersections of spirals            */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
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
/* To find the intersection of two spirals, given two points near it on each:
 * 1. Find the intersection of the lines defined by the points.
 * 2. Interpolate or extrapolate a new point on each curve.
 * 3. Discard the farthest pair of points where one is on one curve and the other
 *    is on the other. If the farthest points are both the new points, or there
 *    are no new points because the lines are parallel, give up.
 * 4. If the closest points are identical, or on both curves the points are so
 *    close together that no point between them can be computed, you're done.
 *    Otherwise return to step 1.
 * To find all the intersections, pick points on both curves as in segment::closest,
 * then pick pairs of points on each curve and run the above algorithm.
 * 
 * As segments and arcs are special cases of spiralarcs, the algorithm works
 * for them too. Intersections of segments and arcs can be computed by formula;
 * intersections involving a spiral must be computed iteratively, since the
 * spiral cannot be written in closed form. Intersections involving an arc
 * of very small delta should be done iteratively, to avoid loss of precision.
 */
#include <vector>
#include "point.h"
#include "spiral.h"

struct alosta
{
  double along;
  xy station;
  int bearing;
  double curvature;
  alosta();
  alosta(double a,xy s);
  alosta(double a,xy s,int b,double c);
};

std::vector<alosta> intersection1(spiralarc a,double a1,double a2,spiralarc b,double b1,double b2,bool extend=false);
std::vector<alosta> intersection1(segment a,double a1,double a2,segment b,double b1,double b2,bool extend=false);
