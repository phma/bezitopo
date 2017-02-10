/******************************************************/
/*                                                    */
/* spolygon.cpp - spherical polygons                  */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#include "spolygon.h"

/* To compute the area of a boundary (which is needed only for testing; area
 * can be computed easier by asking the geoquads), add up the spherical
 * deflection angles and subtract 2π. To compute the deflection angle ABC:
 * • AXB=A×B; BXC=B×C;
 * • Normalize AXB, BXC, and B;
 * • (AXB×BXC)·B is the sine of the angle; AXB·BXC is the cosine.
 * 
 * There is no difference between a polygon traversed clockwise, thus having
 * a negative area, and a polygon traversed counterclockwise and containing
 * almost all the earth. Areas are thus expressed as 32-bit integers like
 * angles. One ulp is about 119 dunams.
 */
