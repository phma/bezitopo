/******************************************************/
/*                                                    */
/* penwidth.h - pen width                             */
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
#define SAMEWIDTH 0x5555
#define ZEROWIDTH (-0x5555)

/* Pen widths are stored in a short in a logarithmic scale. 0 represents
 * a width of 1, 12 a width of 2, etc. What a width of 1 means depends on
 * the device. Add an offset to adjust the width to the device.
 * 
 * SAMEWIDTH means that the width will be set to that of the layer. Any value
 * greater than 200 (which represents a ridiculously large width of 4.77M) is
 * taken to be SAMEWIDTH; any value less than -200 is taken to be ZEROWIDTH.
 */

double penWidthRatio(int penWidthCode);
