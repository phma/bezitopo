/******************************************************/
/*                                                    */
/* dxf.cpp - Drawing Exchange Format                  */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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

#include "dxf.h"
#include "binio.h"
using namespace std;

TagRange tagTable[]=
{
  {0,128}, // string
  {10,88}, // xyz
  {40,72}, // double
  {60,2}, // 2-byte short
  {80,0}, // invalid
  {90,4}, // 4-byte int
  {100,128},
  {101,0},
  {102,128},
  {103,0},
  {105,132}, // hex string
  {106,0},
  {110,72}, // 110-139 are three doubles that go together. 140-149 is a scalar.
  {150,0},
  {160,8}, // 8-byte long
  {170,2},
  {180,0},
  {210,72},
  {240,0},
  {270,2},
  {290,1}, // bool
  {300,128},
  {310,129}, // hex string binary chunk
  {320,132}, // hex string handle
  {370,2},
  {390,132},
  {400,2},
  {410,128},
  {420,4},
  {430,128},
  {440,4},
  {450,8},
  {460,72},
  {470,128},
  {480,132},
  {482,0},
  {999,128},
  {1010,0}
};

int tagFormat(int tag)
{
  int lo=0,hi=sizeof(tagTable)/sizeof(tagTable[0]),mid;
  while (hi-lo>1)
  {
    mid=(lo+hi)/2;
    if (tagTable[mid].tag>tag)
      hi=mid;
    else
      lo=mid;
  }
  if (tagTable[lo].tag>tag)
    return 0;
  else
    return tagTable[lo].format;
}

GroupCode::GroupCode()
{
  tag=-1;
}

GroupCode::GroupCode(int tag0)
{
  tag=tag0;
  switch (tagFormat(tag))
  {
    case 1:
      flag=false;
      break;
    case 2:
    case 4:
    case 8:
    case 132:
      integer=0;
      break;
    case 72:
      real=0;
      break;
    case 88:
      new (&pnt) xyz();
      break;
    case 128:
    case 129:
      new (&str) string();
      break;
  }
}

GroupCode::GroupCode(const GroupCode &b)
{
  tag=b.tag;
  switch (tagFormat(tag))
  {
    case 1:
      flag=b.flag;
      break;
    case 2:
    case 4:
    case 8:
    case 132:
      integer=b.integer;
      break;
    case 72:
      real=b.real;
      break;
    case 88:
      new (&pnt) xyz(b.pnt);
      break;
    case 128:
    case 129:
      new (&str) string(b.str);
      break;
  }
}

GroupCode& GroupCode::operator=(const GroupCode &b)
{
  if (tagFormat(tag)==88 && tagFormat(b.tag)!=88)
    pnt.~xyz();
  if ((tagFormat(tag)&-2)==128 && (tagFormat(b.tag)&-2)!=128)
    str.~string();
  if (tagFormat(tag)!=88 && tagFormat(b.tag)==88)
    new (&pnt) xyz();
  if ((tagFormat(tag)&-2)!=128 && (tagFormat(b.tag)&-2)==128)
    new (&str) string();
  tag=b.tag;
  switch (tagFormat(tag))
  {
    case 1:
      flag=b.flag;
      break;
    case 2:
    case 4:
    case 8:
    case 132:
      integer=b.integer;
      break;
    case 72:
      real=b.real;
      break;
    case 88:
      pnt=b.pnt;
      break;
    case 128:
    case 129:
      str=b.str;
      break;
  }
}

GroupCode::~GroupCode()
{
  switch (tagFormat(tag))
  {
    case 88:
      pnt.~xyz();
      break;
    case 128:
    case 129:
      str.~string();
      break;
  }
}


