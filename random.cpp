/******************************************************/
/*                                                    */
/* random.cpp - random numbers                        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014 Pierre Abbat.
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

#include <cstdio>
#include "random.h"

randm::randm()
{
  randfil=fopen("/dev/urandom","rb");
}

randm::~randm()
{
  fclose(randfil);
}

unsigned short randm::usrandom()
{
  unsigned short n;
  fread(&n,1,2,randfil);
  return n;
}

unsigned char randm::ucrandom()
{
  unsigned char n;
  fread(&n,1,1,randfil);
  return n;
}

randm rng;