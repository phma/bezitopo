/******************************************************/
/*                                                    */
/* linetype.h - dash patterns                         */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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

/* Dash patterns are encoded in base 5 as up to three dash-space pairs.
 * The dash length is from 0 to 4; the space length is from 1 to 5. Dashes
 * are bigger than the number indicates because of the cap style.
 * Decimal Hex Base5
 *  00000 0000 0000000 (0,1,0,1,0,1)
 *  00001 0001 0000001 (1,1,0,1,0,1)
 *  00025 0019 0000100 (0,1,1,1,0,1)
 *  15624 3d08 0444444 (4,5,4,5,4,5)
 *  15625 3d09 1000000 (0,1,0,1)
 *  16249 3f79 1004444 (4,5,4,5)
 *  16250 3f7a 1010000 (0,1)
 *  16274 3f92 1010044 (4,5)
 * This pattern is repeated a total of four times, In each, the dash and space
 * lengths are multiplied by a power of φ.
 * 00000-16274 0000-3f92 1/φ
 * 16275-32549 3f93-7f25 1
 * 32550-48824 7f26-beb8 φ
 * 48825-65099 beb9-fe4b φ+1.
 * 65100 means no line; 65101 means solid line. 65535 means the layer's
 * linetype (unless the layer's linetype is set to 65535, which is nonsense).
 * Values in between are reserved for lines with symbols or letters in them.
 * The length of dashes is multiplied by the line width (see QPen).
 */
#include <QPen>
#define NOLINE 65100
#define SOLIDLINE 65101
#define SAMELINETYPE 65535

void setLineType(QPen &pen,unsigned short lineType);
