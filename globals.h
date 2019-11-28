/******************************************************/
/*                                                    */
/* globals.h - multi-file global variables            */
/*                                                    */
/******************************************************/
/* Copyright 2012,2015,2018 Pierre Abbat.
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

#ifndef GLOBALS_H
#define GLOBALS_H
// #define NDEBUG
#define _WITH_GETLINE
#include <cassert>
#include <map>
#include <cstdio>
#include "point.h"
#include "document.h"
#include "projection.h"

void dumppoints();
extern document doc;
extern ProjectionList allProjections;

#endif
