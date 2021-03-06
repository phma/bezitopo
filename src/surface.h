/******************************************************/
/*                                                    */
/* surface.h - surfaces                               */
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
/* A surface is a function from a region of 2-space to a scalar elevation.
 * Types of surface include:
 * * a hemisphere (for testing the volume computation routine)
 * * a TIN with its Bézier triangles
 * * an alignment with its cross-sections
 * * the nth lowest of m surfaces
 * * a difference between surfaces.
 */
