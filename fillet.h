/******************************************************/
/*                                                    */
/* fillet.h - circular and spiral fillets             */
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

#include "spiral.h"

/* In the horizontal plane, a segment takes four numbers to define, an arc
 * takes five, and a spiralarc takes six. The vertical curve above it takes
 * two more numbers to define if it is linear, three if quadratic, and four
 * if cubic. A vertical curve in an alignment, not necessarily beginning or
 * ending with a horizontal curve, takes four numbers if linear, five if
 * quadratic, and six if cubic.
 *
 * Constraints on a curve are equivalent to fixing numbers:
 * Endpoint lies on a curve: 1
 * Curve passes through point: 1
 * Endpoint is at the end of a curve: 2
 * Endpoint is tangent on a curve: 2
 * Endpoint is tangent at the end of a curve: 3
 * Endpoint osculates a curve: 3
 * Endpoint osculates the end of a curve: 4
 *
 * Circular fillet between two curves:
 *   required: 5
 *   specify radius (1), tangent to both curves (2+2): 5
 *   specify point it passes through (1), tangent to both curves (2+2): 5
 * Arc and two spiralarc fillet:
 *   required: 6+5+6=17
 *   specify curvature (1), two clothances (usually equal) (2)
 *   endpoint osculates a curve (3+3)
 *   endpoint osculates endpoint (4+4): 17
 *   specify radius (1), two spiral lengths (usually equal) (2)
 *   endpoint osculates a curve (3+3)
 *   endpoint osculates endpoint (4+4): 17
 * One spiralarc fillet between two circles which do not intersect and
 * are not concentric:
 *   required: 6
 *   endpoint osculates a curve (3+3): 6
 * Two spiralarc fillet at a specified point:
 *   required: 6+6=12
 *   specify common endpoint (2)
 *   endpoint osculates a curve (3+3)
 *   endpoint osculates endpoint (4): 12
 */
