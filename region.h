/******************************************************/
/*                                                    */
/* region.h - regions                                 */
/*                                                    */
/******************************************************/
/* Copyright 2014 Pierre Abbat.
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
/* A region is some piece of land surrounded by a boundary.
 * It has these methods:
 * + List points on or inside the boundary.
 * + Tell whether a point is in the region.
 * + Tell whether a line segment intersects the boundary.
 * 
 * These kinds of regions are available:
 * + Circular (for testing). The point list consists of the center.
 * + Polyline.
 * + Intersection (for surfaces that are made by subtracting surfaces).
 * 
 * They are used for delimiting areas over which a volume is to be computed
 * and for delimiting areas within which contours are valid.
 * A region can have a hole in it; for instance, a lot with a building,
 * where the contours passing through the building are omitted.
 */
