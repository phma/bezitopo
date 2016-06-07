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
#include <iostream>
#include <iomanip>
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

void matrix::setidentity()
{
  int i;
  if (rows!=columns)
    throw matrixmismatch;
  memset(entry,0,rows*columns*sizeof(double));
  for (i=0;i<rows;i++)
    (*this)[i][i]=1;
}

void matrix::dump()
{
  int i,j,wid=10,prec=3;
  cout<<scientific;
  for (i=0;i<rows;i++)
  {
    for (j=0;j<columns;j++)
      cout<<setw(wid)<<setprecision(prec)<<(*this)[i][j];
    cout<<endl;
  }
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

rowsult matrix::rowop(matrix &b,int row0,int row1)
/* Does 0 or more of the elementary row operations:
 * 0: swap row0 and row1
 * 1: divide row0 by the number in the pivot column
 * 2: subtract row0 multiplied by the number in the
 *    pivot column of row1 from row1.
 * Bits 0, 1, or 2 of flags are set to tell what it did.
 * The pivot of row 0 is returned as detfactor, negated if it swapped rows.
 * If b is *this, it is ignored. If not, its rows are swapped, divided,
 * and subtracted along with this's rows.
 */
{
  rowsult ret;
  int i;
  double *temp,*rw0,*rw1,*rwb0,*rwb1;
  double slope,minslope=INFINITY;
  i=columns;
  if (b.columns>i)
    i=b.columns;
  temp=new double[i];
  rw0=(*this)[row0];
  rw1=(*this)[row1];
  if (this==&b)
    rwb0=rwb1=nullptr;
  else
  {
    rwb0=b[row0];
    rwb1=b[row1];
  }
  ret.pivot=-1;
  slope=ret.flags=0;
  for (i=0;i<columns;i++)
    if (rw0[i]!=0 && rw1[i]!=0)
    {
      if (fabs(rw0[i])>fabs(rw1[i]) || row0>=row1)
      {
	slope=fabs(rw1[i]/rw0[i]);
	ret.flags&=~8;
      }
      else
      {
	slope=fabs(rw0[i]/rw1[i]);
	ret.flags|=8;
      }
      if (slope<minslope)
      {
	minslope=slope;
	ret.flags=(ret.flags>>3)*9;
	ret.pivot=i;
      }
    }
  ret.flags&=1;
  if (ret.flags)
  {
    memcpy(temp,rw0,sizeof(double)*columns);
    memcpy(rw0,rw1,sizeof(double)*columns);
    memcpy(rw1,temp,sizeof(double)*columns);
    if (rwb0)
    {
      memcpy(temp,rwb0,sizeof(double)*b.columns);
      memcpy(rwb0,rwb1,sizeof(double)*b.columns);
      memcpy(rwb1,temp,sizeof(double)*b.columns);
    }
  }
  if (ret.pivot<0)
    ret.detfactor=0;
  else
    ret.detfactor=rw0[ret.pivot];
  if (ret.detfactor!=0 && ret.detfactor!=1)
  {
    for (i=0;i<columns;i++)
      rw0[i]/=ret.detfactor;
    for (i=0;rwb0 && i<b.columns;i++)
      rwb0[i]/=ret.detfactor;
    ret.flags+=2;
  }
  if (ret.pivot>=0)
    slope=rw1[ret.pivot];
  if (slope!=0 && row0!=row1)
  {
    for (i=0;i<columns;i++)
      rw1[i]-=rw0[i]*slope;
    for (i=0;rwb0 && i<b.columns;i++)
      rwb1[i]-=rwb0[i]*slope;
    ret.flags+=4;
  }
  if (ret.flags&1)
    ret.detfactor=-ret.detfactor;
  return ret;
}

void matrix::gausselim(matrix &b)
{
  int i,j;
  //dump();
  for (i=0;i<rows;i++)
  {
    for (j=0;j<rows;j++)
      rowop(b,i,j);
    //cout<<endl;
    //dump();
  }
  for (i=rows-1;i>=0;i--)
  {
    for (j=0;j<i;j++)
      rowop(b,i,j);
    //cout<<endl;
    //dump();
  }
  //cout<<endl;
  //b.dump();
}

matrix invert(matrix m)
{
  matrix x(m),ret(m);
  ret.setidentity();
  x.gausselim(ret);
  if (x.getrows()>0 && x[x.getrows()-1][x.getrows()-1]==0)
    ret[0][0]=NAN;
  return ret;
}
