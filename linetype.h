/******************************************************/
/*                                                    */
/* linetype.h - dash patterns and line widths         */
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

/* Dash patterns are encoded as a string of bytes, where:
 * 0xff 1024 mm
 * 0x41 1.00 mm
 * 0x00 0.093 mm
 * 0x01 0.097 mm
 * 0x02 0.100 mm
 * 0x54 2.0 mm
 * Line widths are encoded as shorts on a logarithmic scale,
 * with the special value 0 meaning 0.
 */
