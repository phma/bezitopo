/******************************************************/
/*                                                    */
/* manysum.h - add many numbers                       */
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
#include <map>
#include <cmath>
/* Adds together many numbers (like millions) accurately.
 * Each number is put in a bucket numbered by its exponent returned by frexp().
 * If the sum of the number and what's in the bucket is too big to fit
 * in the bucket, it is put into the next bucket up. If the sum is too small,
 * it is not put into the next bucket down, as it has insignificant bits
 * at the low end. This can happen only when adding numbers of opposite sign,
 * which does not happen when computing volumes, as positive and negative
 * volumes are added separately so that compaction of dirt can be computed.
 */

class manysum
{
private:
  std::map<int,double> bucket;
  static int cnt;
public:
  void clear();
  void prune();
  double total();
  void dump();
  manysum& operator+=(double x);
  manysum& operator-=(double x);
};
