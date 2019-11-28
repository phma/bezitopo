/******************************************************/
/*                                                    */
/* halton.h - Halton subrandom point generator        */
/*                                                    */
/******************************************************/
/* Copyright 2014-2016,2019 Pierre Abbat.
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

/* This class generates Halton sequences. The object has a 64-bit
 * number counting from 0 to 2^32*3^20-1. The number's bits and trits
 * are reversed, and it produces one of three kinds of output:
 * • x and y coordinates, one made from the bits and the other from the trits;
 * • a latlong, the latitude made from the trits (with slight influence
 *   from the bits) and the longitude from the bits; or
 * • a single number.
 * The x and y coordinates, if the sequence is counted to its full length,
 * are dense enough to cover Egypt with 15 points per square millimeter.
 * They are used for numerical integration to find volumes.
 * The latlongs are dense enough to cover the entire earth with 29364
 * points per square meter.
 * The latlong is used for computing error histograms of converted geoids.
 * The single number is used to select which subsquare when picking
 * a random point within a boundary.
 */

#ifndef HALTON_H
#define HALTON_H
#include <vector>
#include "point.h"
#include "angle.h"
#include "latlong.h"

extern unsigned short btreversetable[62208];
void initbtreverse();
std::vector<unsigned short> splithalton(unsigned long long n);
unsigned long long btreverselong(unsigned long long n);

class halton
{
private:
  unsigned long long n;
  halton& operator++();
public:
  halton();
  xy _pnt();
  xy pnt();
  latlong _onearth();
  latlong onearth();
  double _scalar(double x);
  double scalar(double x); // this can return x because of roundoff
};

#endif
