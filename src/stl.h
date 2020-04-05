/******************************************************/
/*                                                    */
/* stl.h - stereolithography (3D printing) export     */
/*                                                    */
/******************************************************/
/* Copyright 2013,2017,2019 Pierre Abbat.
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

#include <array>
#include <vector>
#include "point.h"
#include "config.h"

extern std::vector<int> stltable; // used in bezier.cpp
void initStlTable();
std::array<int,3> adjustStlSplit(std::array<int,3> stlSplit,std::array<int,3> stlMin);

struct stltriangle
{
  xyz normal,a,b,c;
  std::string attributes;
  stltriangle();
  stltriangle(xyz A,xyz B,xyz C);
};
