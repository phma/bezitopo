/******************************************************/
/*                                                    */
/* pnezd.h - file i/o in                              */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2015,2016,2017 Pierre Abbat.
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

#include <vector>
#include <string>
#include "measure.h"

class document;

int readpnezd(document *doc,std::string fname,Measure ms,bool overwrite=false);
int writepnezd(document *doc,std::string fname,Measure ms);
int readpenzd(document *doc,std::string fname,Measure ms,bool overwrite=false);
int writepenzd(document *doc,std::string fname,Measure ms);
