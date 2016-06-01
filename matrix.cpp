/******************************************************/
/*                                                    */
/* matrix.cpp - matrices                              */
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

#include <cassert>
#include <cstring>
#include <utility>
#include "matrix.h"

using namespace std;

matrix::matrix()
{
  rows=columns=0;
  entry=nullptr;
}

matrix::matrix(unsigned r,unsigned c)
{
  rows=r;
  columns=c;
  entry=new double[r*c];
}

matrix::matrix(const matrix &b)
{
  rows=b.rows;
  columns=b.columns;
  entry=new double[rows*columns];
  memcpy(entry,b.entry,sizeof(double)*rows*columns);
}

matrix::~matrix()
{
  delete[] entry;
}

matrix &matrix::operator=(const matrix &b)
{
  if (this!=&b)
  {
    matrix c(b);
    swap(rows,c.rows);
    swap(columns,c.columns);
    swap(entry,c.entry);
  }
}

double *matrix::operator[](unsigned row)
{
  assert(row<rows);
  return entry+columns*row;
}
