/******************************************************/
/*                                                    */
/* geoidboundary.cpp - geoid boundaries               */
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
#include "geoid.h"
#include "geoidboundary.h"

char vballcompare[8][8]=
{
  {00,77,77,77,77,77,77,77},
  {77,66,12,21,14,36,77,77},
  {77,21,66,12,36,77,14,77},
  {77,12,21,66,77,14,36,77},
  {77,41,63,77,66,45,54,77},
  {77,63,77,41,54,66,45,77},
  {77,77,41,63,45,54,66,77},
  {77,77,77,77,77,77,77,77}
};

bool operator==(const vball &a,const vball &b)
{
  int edgetype=vballcompare[a.face][b.face];
  bool ret=false;
  switch (edgetype)
  {
    case 00:
      ret=true;
      break;
    case 12:
      ret=a.x==1 && a.y==b.x && b.y==1;
      break;
    case 21:
      ret=a.y==1 && a.x==b.y && b.x==1;
      break;
    case 14:
      ret=a.y==-1 && a.x==-b.y && b.x==1;
      break;
    case 41:
      ret=a.x==1 && a.y==-b.x && b.y==-1;
      break;
    case 36:
      ret=a.x==-1 && a.y==b.x && b.y==-1;
      break;
    case 63:
      ret=a.y==-1 && a.x==b.y && b.x==-1;
      break;
    case 45:
      ret=a.y==1 && a.x==-b.y && b.x==-1;
      break;
    case 54:
      ret=a.x==-1 && a.y==-b.x && b.y==1;
      break;
    case 66:
      ret=a.x==b.x && a.y==b.y;
      break;
  }
  return ret;
}

bool sameEdge(const vball &a,const vball &b)
{
  int edgetype=vballcompare[a.face][b.face];
  bool ret=false;
  switch (edgetype)
  {
    case 00:
      ret=true;
      break;
    case 12:
      ret=a.x==1 && b.y==1;
      break;
    case 21:
      ret=a.y==1 && b.x==1;
      break;
    case 14:
      ret=a.y==-1 && b.x==1;
      break;
    case 41:
      ret=a.x==1 && b.y==-1;
      break;
    case 36:
      ret=a.x==-1 && b.y==-1;
      break;
    case 63:
      ret=a.y==-1 && b.x==-1;
      break;
    case 45:
      ret=a.y==1 && b.x==-1;
      break;
    case 54:
      ret=a.x==-1 && b.y==1;
      break;
    case 66:
      ret=a.x==b.x || a.y==b.y;
      break;
  }
  return ret;
}
