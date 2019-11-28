/******************************************************/
/*                                                    */
/* color.h - drawing colors                           */
/*                                                    */
/******************************************************/
/* Copyright 2015,2017 Pierre Abbat.
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
/* Colors are stored in two formats: short and int.
 * A short denotes one of 64000 colors (40³) or any of up to 1536 special values
 * (like same as the layer). An int denotes any of 16777216 colors.
 * Colors are as shown on screen, with a black background.
 * When printing, one can do any of the following:
 * • Print as is, wasting lots of black ink.
 * • Exchange white and black, leaving everything else the same.
 * • Invert all colors.
 * • Invert light and dark, leaving hue alone. This is difficult.
 * This operation is done on the 24-bit color.
 */
#include <QPen>

#define SAMECOLOR 65535
#define BLACK 0
#define BLUE 39
#define GREEN 1560
#define CYAN 1599
#define RED 62400
#define MAGENTA 62439
#define YELLOW 63960
#define WHITE 63999

int colorint(unsigned short colorshort);
unsigned short colorshort(int colorint);
int printingcolor(int color,int op);
void setColor(QPen &pen,unsigned short color);
