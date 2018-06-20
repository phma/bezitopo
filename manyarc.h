/******************************************************/
/*                                                    */
/* manyarc.h - approximate spiral with many arcs      */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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

#include "polyline.h"

double manyArcTrimFunc(double p,double n);
double manyArcTrimDeriv(double p,double n);
double manyArcTrim(unsigned n);
std::vector<segment> manyQuad(segment cubic,int narcs);
double meanSquareDistance(polyarc apx,spiralarc a);
polyarc manyArcUnadjusted(spiralarc a,int narcs);
polyarc manyArc(spiralarc a,int narcs);
