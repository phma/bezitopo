/******************************************************/
/*                                                    */
/* roadint.h - road intersections                     */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include "polyline.h"

/* A road intersection is placed at the intersection of two or more alignments.
 * Its boundary consists of alternating line segments, which have cross-sections
 * fitted to those of the alignments, and circular arcs concave outward.
 * The region is star-shaped about a point, normally the intersection of all
 * the roads. The surface is computed by some combination of filleting
 * the alignments and connecting them to the center point.
 */
