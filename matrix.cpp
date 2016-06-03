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
#include "manysum.h"
#include "random.h"

#define BYTERMS 104.51076499576490995918
/* square root of 10922.5, which is the root-mean-square of a random byte
 * doubled and offset to center
 */

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

matrix matrix::operator+(matrix& b)
{
  if (rows!=b.rows || columns!=b.columns)
    throw matrixmismatch;
  matrix ret(*this);
  int i;
  for (i=0;i<rows*columns;i++)
    ret.entry[i]+=b.entry[i];
  return ret;
}

matrix matrix::operator-(matrix& b)
{
  if (rows!=b.rows || columns!=b.columns)
    throw matrixmismatch;
  matrix ret(*this);
  int i;
  for (i=0;i<rows*columns;i++)
    ret.entry[i]-=b.entry[i];
  return ret;
}

matrix matrix::operator*(matrix &b)
{
  if (columns!=b.rows)
    throw matrixmismatch;
  matrix ret(rows,b.columns);
  int h,i,j,k;
  double *sum;
  sum=new double[columns];
  for (i=0;i<rows;i++)
    for (j=0;j<b.columns;j++)
    {
      for (k=0;k<columns;k++)
	sum[k]=(*this)[i][k]*b[k][j];
      for (h=1;h<columns;h*=2)
	for (k=0;k+h<columns;k+=2*h)
	  sum[k]+=sum[k+h];
      ret[i][j]=sum[0];
    }
  delete[] sum;
  return ret;
}

double matrix::trace()
{
  if (columns!=rows)
    throw matrixmismatch;
  manysum ret;
  int i;
  for (i=0;i<rows;i++)
    ret+=(*this)[i][i];
  return ret.total();
}

void matrix::randomize_c()
{
  int i;
  for (i=0;i<rows*columns;i++)
    entry[i]=(rng.ucrandom()*2-255)/BYTERMS;
}
