/******************************************************/
/*                                                    */
/* smooth5.cpp - 5-smooth numbers                     */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include "smooth5.h"
#include "angle.h"

bool smooth5(unsigned n)
/* Used for deciding the number of divisions of a circle in a lat-long grid.
 * This is in practice always a 5-smooth number, such as 21600.
 * 21600 (1 arc minute) is between 20736 and 21870.
 * 1440 (1/4 degree, 1 time minute) is between 1350 and 1458.
 * The finest division that may be specified is 0.4 second of arc. The
 * coarsest that may not be used is 0.3955 second (180°/1638400), which is
 * too close to 0.3951 (180°/1640250). Between those two, the criterion is
 * 660 (0.3983 second).
 * Also used for encoding the STL split number of an edge.
 */
{
  if (n==0)
    n=7; // 0 is not smooth, as it is divisible by all primes. It also cannot be the number of divisions.
  while ((n%2)==0)
    n/=2;
  while ((n%3)==0)
    n/=3;
  while ((n%5)==0)
    n/=5;
  return n==1;
}

unsigned nearestSmooth(unsigned n)
{
  unsigned ub,lb;
  if (n>4271484375)
    n=4271484375;
  if (n==0)
    n=1;
  ub=lb=n;
  while (!smooth5(ub))
    ub++;
  while (!smooth5(lb))
    lb--;
  if (sqr(n)>(double)ub*lb)
    n=ub;
  else
    n=lb;
  return n;
}
