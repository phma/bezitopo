/******************************************************/
/*                                                    */
/* refinegeoid.h - refine geoid approximation         */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include "geoid.h"
#include "histogram.h"
#include "manysum.h"

extern int avgelev_interrocount,avgelev_refinecount;
extern histogram correctionHist;
extern manysum dataArea,totalArea;

void outProgress();
void interroquad(geoquad &quad,double spacing);
void refine(geoquad &quad,double vscale,double tolerance,double sublimit,double spacing,int qsz,bool allbol);
