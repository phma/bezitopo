/******************************************************/
/*                                                    */
/* ptin.h - PerfectTIN files                          */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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

#ifndef PTIN_H
#define PTIN_H
#include <string>
#include "manysum.h"
#include "pointlist.h"

#define PT_UNKNOWN_HEADER_FORMAT -1
#define PT_NOT_PTIN_FILE -2
#define PT_COUNT_MISMATCH -3
#define PT_INVALID_POINT_NUMBER -4
#define PT_BACKWARD_TRIANGLE -5
#define PT_OUT_OF_RANGE -6
#define PT_EOF -7
#define PT_EDGE_MISMATCH -8
#define PT_DOT_OUTSIDE -9
#define PT_ZCHECK_FAIL -10
/* Unknown header format: file was written by a newer version of PerfectTIN.
 * Not ptin file: file is not a PerfectTIN file.
 * Count mismatch: file is not a PerfectTIN file.
 * Any other negative tolRatio value: file is corrupt.
 * tolRatio>0 but tolerance is NaN: file was incompletely written.
 */

struct PtinHeader
{
  PtinHeader();
  time_t conversionTime;
  double tolerance; // NaN means file wasn't finished being written
  int tolRatio; // negative means an error
  int numPoints;
  int numConvexHull;
  int numTriangles;
};

class CoordCheck
{
private:
  size_t count;
  double stage0[14][8192],stage1[27][8192],stage2[40][8192],
         stage3[53][8192],stage4[64][4096];
public:
  void clear();
  CoordCheck& operator<<(double val);
  double operator[](int n);
  size_t getCount()
  {
    return count;
  }
};

xyz readPoint(std::istream &file);
PtinHeader readPtinHeader(std::istream &inputFile);
PtinHeader readPtinHeader(std::string inputFile);
PtinHeader readPtin(std::string inputFile,pointlist &pl);
#endif
