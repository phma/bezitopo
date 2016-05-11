/******************************************************/
/*                                                    */
/* vcurve.h - vertical curve arithmetic               */
/*                                                    */
/******************************************************/
/* Copyright 2012 Pierre Abbat.
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

#include <vector>

double vcurve(double a,double b,double c,double d,double x);
double vslope(double a,double b,double c,double d,double x);
double vaccel(double a,double b,double c,double d,double x);
double vlength(double a,double b,double c,double d,double hlength);
std::vector<double> vextrema(double a,double b,double c,double d);
void vsplit(double a,double b,double c,double d,double p,
	    double &b1,double &c1,double &d1a2,double &b2,double &c2);
