/******************************************************/
/*                                                    */
/* binio.cpp - binary input/output                    */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#include <cstring>
#include "binio.h"
#include "config.h"

using namespace std;

void endianflip(void *addr,int n)
{
  int i;
  char *addr2;
  addr2=(char *)addr;
  for (i=0;i<n/2;i++)
  {
    addr2[i]^=addr2[n-1-i];
    addr2[n-1-i]^=addr2[i];
    addr2[i]^=addr2[n-1-i];
  }
}

void writebeshort(std::ostream &file,short i)
{
  char buf[2];
  *(short *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,2);
#endif
  file.write(buf,2);
}

void writeleshort(std::ostream &file,short i)
{
  char buf[2];
  *(short *)buf=i;
#ifdef BIGENDIAN
  endianflip(buf,2);
#endif
  file.write(buf,2);
}

short readbeshort(std::istream &file)
{
  char buf[2];
  file.read(buf,2);
#ifndef BIGENDIAN
  endianflip(buf,2);
#endif
  return *(short *)buf;
}

short readleshort(std::istream &file)
{
  char buf[2];
  file.read(buf,2);
#ifdef BIGENDIAN
  endianflip(buf,2);
#endif
  return *(short *)buf;
}

void writebeint(std::ostream &file,int i)
{
  char buf[4];
  *(int *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

void writeleint(std::ostream &file,int i)
{
  char buf[4];
  *(int *)buf=i;
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  file.write(buf,4);
}

int readbeint(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(int *)buf;
}

int readleint(std::istream &file)
{
  char buf[4];
  file.read(buf,4);
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(int *)buf;
}

float readbefloat(std::fstream &file)
{
  char buf[4];
  file.read(buf,4);
#ifndef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(float *)buf;
}

float readlefloat(std::fstream &file)
{
  char buf[4];
  file.read(buf,4);
#ifdef BIGENDIAN
  endianflip(buf,4);
#endif
  return *(float *)buf;
}

void writebedouble(std::ostream &file,double i)
{
  char buf[8];
  *(double *)buf=i;
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  file.write(buf,8);
}

double readbedouble(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifndef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(double *)buf;
}

double readledouble(std::istream &file)
{
  char buf[8];
  file.read(buf,8);
#ifdef BIGENDIAN
  endianflip(buf,8);
#endif
  return *(double *)buf;
}

void writegeint(std::ostream &file,int i)
/* Numbers in Bezitopo's geoid files are in 65536ths of a meter and are less than 110 m
 * (7208960) in absolute value. They are encoded as follows:
 * gg xx xx where gg is 00-7f		00 gg xx xx
 * gg xx xx where gg is 81-ff		ff gg xx xx
 * 80 gg xx xx xx where gg is not 0	gg xx xx xx with an offset
 * 80 00				80 00 00 00, which means NaN
 * Numbers greater than 0x7f800000 are encoded as 80 00. They mean 32640 m, which is
 * higher than the tallest mountain, so they cannot occur.
 */
{
  char buf[8];
  if (i>=(int)0xff810000 && i<=0x7fffff)
  {
    *(int *)buf=i;
#ifndef BIGENDIAN
    endianflip(buf,4);
#endif
    file.write(buf+1,3);
  }
  else if (i==(int)0x80000000 || i>=(int)0x7f800000)
  {
    buf[0]=0x80;
    buf[1]=0;
    file.write(buf,2);
  }
  else
  {
    if (i<(int)0xff810000)
      *(int *)buf=i-0xff810000;
    if (i>0x7fffff)
      *(int *)buf=i+0x800000;
#ifndef BIGENDIAN
    endianflip(buf,4);
#endif
    memmove(buf+1,buf,4);
    buf[0]=0x80;
    file.write(buf,5);
  }
}

int readgeint(std::istream &file)
{
  char buf[8];
  int ret;
  file.read(buf,2);
  if (buf[0]==(char)0x80)
    if (buf[1]==0)
      ret=0x80000000;
    else
    {
      file.read(buf+2,3);
      memmove(buf,buf+1,4);
#ifndef BIGENDIAN
      endianflip(buf,4);
#endif
      ret=*(int *)buf;
      if (ret<0)
	ret-=0x7f0000;
      else
	ret-=0x800000;
    }
  else
  {
    file.read(buf+2,1);
    memmove(buf+1,buf,3);
    buf[0]=(buf[1]&0x80)?0xff:0;
#ifndef BIGENDIAN
    endianflip(buf,4);
#endif
    ret=*(int *)buf;
  }
  return ret;
}

void writeustring(ostream &file,string s)
// FIXME: if s contains a null character, it should be written as c0 a0
{
  file.write(s.data(),s.length());
  file.put(0);
}

string readustring(istream &file)
{
  int ch;
  string ret;
  do
  {
    ch=file.get();
    if (ch>0)
      ret+=(char)ch;
  } while (ch>0);
  return ret;
}
