/******************************************************/
/*                                                    */
/* binio.h - binary input/output                      */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016,2018-2020 Pierre Abbat.
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
#include <fstream>
#include <string>

#define FP_IEEE 754
/* Used in the header of transmer.dat.
 * Other values are:
 * 1876 (0x754): IEEE decimal
 * 1508: Fake double precision (e.g. 1508,128 means two 64-bit numbers encode
 * one number with greater precision, but no greater range, called double-double)
 * 387: 387,80 is the ten-byte float (long double) endemic to Intel-compatible
 * processors, named for the floating-point coprocessor
 */

std::streamsize fileSize(std::istream &file);
void writebeshort(std::ostream &file,short i);
void writeleshort(std::ostream &file,short i);
short readbeshort(std::istream &file);
short readleshort(std::istream &file);
void writebeint(std::ostream &file,int i);
void writeleint(std::ostream &file,int i);
int readbeint(std::istream &file);
int readleint(std::istream &file);
void writebelong(std::ostream &file,long long i);
void writelelong(std::ostream &file,long long i);
long long readbelong(std::istream &file);
long long readlelong(std::istream &file);
void writebefloat(std::ostream &file,float f);
void writelefloat(std::ostream &file,float f);
float readbefloat(std::istream &file);
float readlefloat(std::istream &file);
void writebedouble(std::ostream &file,double f);
void writeledouble(std::ostream &file,double f);
double readbedouble(std::istream &file);
double readledouble(std::istream &file);
void writegeint(std::ostream &file,int i); // for Bezitopo's geoid files
int readgeint(std::istream &file);
void writeustring(std::ostream &file,std::string s);
std::string readustring(std::istream &file);

