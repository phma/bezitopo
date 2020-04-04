/******************************************************/
/*                                                    */
/* except.h - exceptions                              */
/*                                                    */
/******************************************************/
/* Copyright 2017-2018 Pierre Abbat.
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
#include <QException>
#include <QString>
#include <QCoreApplication>

#define notri 1
// less than 3 points, no triangle
#define samepnts 2
// two points are at same xy coordinates when making a TIN
#define flattri 3
// all the points are in a line
#define badheader 4
// when reading a geoid file
#define baddata 5
// when reading a geoid file
#define matrixmismatch 6
// operation on matrices is impossible because the sizes don't match
#define singularmatrix 7
// not actually used - for inverting a singular matrix
#define unsetgeoid 8
// trying to write a geoid file when the pointer in the geoid structure is null
#define unsetsource 9
// trying to copy topo points from a nonexistent pointlist
#define badunits 10
// unrecognized unit symbol in a measurement
#define badnumber 11
// malformatted (empty) number in a measurement
#define badbreaklineend 12
// a breakline endpoint number is not the number of any point
#define breaklinescross 13
// two breakline segments intersect
#define badbreaklineformat 14
// malformatted breakline text representation
#define fileerror 15
// any error reading or writing a file
#define stationoutofrange 16
// along is out of range in a station function
#define badabsorient 17
// insufficient points to compute absolute orientation
#define N_EXCEPTIONS 18

class BeziExcept: public QException
{
  Q_DECLARE_TR_FUNCTIONS(BeziExcept)
public:
  BeziExcept(const BeziExcept &a);
  BeziExcept(int num);
  int pointNumber[2];
  void raise() const
  {
    throw *this;
  }
  BeziExcept *clone() const
  {
    return new BeziExcept(*this);
  }
  void clear();
  const QString &message();
  int getNumber()
  {
    return exceptNumber;
  }
private:
  int exceptNumber;
};

const QString &translateException(int n);
void initTranslateException();

extern BeziExcept noTriangle,samePoints,flatTriangle;
extern BeziExcept badHeader,badData,matrixMismatch;
extern BeziExcept singularMatrix,unsetGeoid;
extern BeziExcept unsetSource,badUnits,badNumber;
extern BeziExcept badBreaklineEnd,breaklinesCross;
extern BeziExcept badBreaklineFormat,fileError;
extern BeziExcept stationOutOfRange,badAbsOrient;
