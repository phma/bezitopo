/******************************************************/
/*                                                    */
/* vball.h - volleyball coordinates                   */
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
#ifndef VBALL_H
#define VBALL_H
#include "xyz.h"
#define EARTHRAD 6371e3
#define EARTHRADSQ 4.0589641e13

struct vball // so called because a sphere so marked looks like a volleyball
{
  int face;
  double x,y;
  vball();
  vball(int f,xy p);
  xy getxy();
  double diag();
};

vball encodedir(xyz dir);
xyz decodedir(vball code);
#endif
