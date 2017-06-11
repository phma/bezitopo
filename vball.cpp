/******************************************************/
/*                                                    */
/* vball.cpp - volleyball coordinates                 */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#include <cmath>
#include <cfloat>
#include "vball.h"

signed char adjFaceY[6][2][3]=
{ // new face, new x value, sign change of x becoming y
  {{3,1,1},{4,1,-1}},
  {{1,1,1},{6,1,-1}},
  {{2,1,1},{5,1,-1}},
  {{5,-1,-1},{2,-1,1}},
  {{6,-1,-1},{1,-1,1}},
  {{4,-1,-1},{3,-1,1}}
}, adjFaceX[6][2][3]=
{ // new face, new y value, sign change of y becoming x
  {{2,1,1},{5,-1,1}},
  {{3,1,1},{4,-1,1}},
  {{1,1,1},{6,-1,1}},
  {{1,-1,-1},{6,1,-1}},
  {{3,-1,-1},{4,1,-1}},
  {{2,-1,-1},{5,1,-1}}
};

vball::vball()
{
  face=0;
  x=y=0;
}

vball::vball(int f,xy p)
{
  face=f;
  x=p.getx();
  y=p.gety();
}

xy vball::getxy()
{
  return xy(x,y);
}

double vball::diag()
/* Used in geoidboundary as a fake distance, because all distances involved
 * are orthogonal.
 */
{
  return x+y;
}

void vball::switchFace()
/* If the vball is on an edge, switches to the other face on that edge.
 * If it is at a corner, switches to one of the other two faces,
 * cycling through the three faces on the corner.
 */
{
  double save;
  if (face>0 && face<7 && fabs(y)==1)
  {
    save=y;
    y=x*adjFaceY[face-1][save<0][2];
    x=adjFaceY[face-1][save<0][1];
    face=adjFaceY[face-1][save<0][0];
  }
  else if (face>0 && face<7 && fabs(x)==1)
  {
    save=x;
    x=y*adjFaceX[face-1][save<0][2];
    y=adjFaceX[face-1][save<0][1];
    face=adjFaceX[face-1][save<0][0];
  }
}

vball encodedir(xyz dir)
{
  vball ret;
  double absx,absy,absz;
  absx=fabs(dir.getx());
  absy=fabs(dir.gety());
  absz=fabs(dir.getz());
  if (absx==0 && absy==0 && absz==0)
  {
    ret.face=0;
    ret.x=ret.y=0;
  }
  else if (std::isnan(absx) || std::isnan(absy) || std::isnan(absz) ||
           ((std::isinf(absx)+std::isinf(absy)+std::isinf(absz))>1))
  {
    ret.face=7;
    ret.x=ret.y=NAN;
  }
  else
  {
    if (absx>=absy && absx>=absz)
    {
      ret.face=1;
      ret.x=dir.gety()/absx;
      ret.y=dir.getz()/dir.getx();
      if (dir.getx()<0)
	ret.face=6;
    }
    if (absy>=absz && absy>=absx)
    {
      ret.face=2;
      ret.x=dir.getz()/absy;
      ret.y=dir.getx()/dir.gety();
      if (dir.gety()<0)
	ret.face=5;
    }
    if (absz>=absx && absz>=absy)
    {
      ret.face=3;
      ret.x=dir.getx()/absz;
      ret.y=dir.gety()/dir.getz();
      if (dir.getz()<0)
	ret.face=4;
    }
  }
  return ret;
}

xyz decodedir(vball code)
{
  xyz ret;
  switch (code.face&7)
  {
    case 0:
      ret=xyz(0,0,0);
      break;
    case 1:
      ret=xyz(1,code.x,code.y);
      break;
    case 2:
      ret=xyz(code.y,1,code.x);
      break;
    case 3:
      ret=xyz(code.x,code.y,1);
      break;
    case 4:
      ret=xyz(code.x,-code.y,-1);
      break;
    case 5:
      ret=xyz(-code.y,-1,code.x);
      break;
    case 6:
      ret=xyz(-1,code.x,-code.y);
      break;
    case 7:
      ret=xyz(NAN,NAN,NAN);
      break;
  }
  if ((code.face&7)%7)
    ret=ret*(EARTHRAD/ret.length());
  return ret;
}
