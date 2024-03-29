/******************************************************/
/*                                                    */
/* fileio.h - file I/O                                */
/*                                                    */
/******************************************************/
/* Copyright 2019-2022 Pierre Abbat.
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
#ifndef FILEIO_H
#define FILEIO_H
#include <string>
#include "manysum.h"
#include "point.h"
#include "stl.h"

#define PT_UNKNOWN_HEADER_FORMAT -1
#define PT_NOT_PTIN_FILE -2
#define PT_COUNT_MISMATCH -3
#define PT_INVALID_POINT_NUMBER -4
#define PT_BACKWARD_TRIANGLE -5
#define PT_INVALID_CONVEX_HULL -6
#define PT_EOF -7
#define PT_EDGE_MISMATCH -8
#define PT_DOT_OUTSIDE -9
#define PT_ZCHECK_FAIL -10
#define PT_CONTOUR_ERROR -11
#define PT_UNKNOWN_GROUP -12
/* Unknown header format: file was written by a newer version of PerfectTIN.
 * Not ptin file: file is not a PerfectTIN file.
 * Count mismatch: file is not a PerfectTIN file.
 * Any other negative tolRatio value: file is corrupt.
 * tolRatio>0 but tolerance is NaN: file was incompletely written.
 */
#define CHECKSUM_BUG 1
#define CHECKSUM_DIV1 65536
#define CHECKSUM_DIV2 536870912

#define GRP_CONTOUR 0
#define GRP_BOUNDARY 1
#define GRPTYPE_POLY 0

extern char hexdig[16];

std::string noExt(std::string fileName);
std::string extension(std::string fileName);
std::string baseName(std::string fileName);
void deleteFile(std::string fileName);
void writeDxf(std::string outputFile,pointlist &pl,bool asc,double outUnit,int flags);
void writeStl(std::string outputFile,pointlist &pl,bool asc,double outUnit,int flags);
int readCloud(std::string &inputFile,double inUnit,int flags);
void writePoint(std::ostream &file,xyz pnt);
xyz readPoint(std::istream &file);
void dumpTriangles(std::string outputFile,std::vector<triangle *> tris);
#endif
