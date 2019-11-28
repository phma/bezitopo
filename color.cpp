/******************************************************/
/*                                                    */
/* color.cpp - drawing colors                         */
/*                                                    */
/******************************************************/
/* Copyright 2015,2017 Pierre Abbat.
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

#include <cassert>
#include <iostream>
#include "color.h"
using namespace std;

unsigned char tab40[]=
{
    0,  7, 13, 20, 26, 33, 39, 46, 52, 59, 65, 72, 78, 85, 92, 98,105,111,118,124,
  131,137,144,150,157,163,170,177,183,190,196,203,209,216,222,229,235,242,248,255
};

char tab256[]=
{
   0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
   2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5,
   5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
   7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9,10,
  10,10,10,10,10,11,11,11,11,11,11,11,12,12,12,12,
  12,12,13,13,13,13,13,13,13,14,14,14,14,14,14,15,
  15,15,15,15,15,15,16,16,16,16,16,16,17,17,17,17,
  17,17,17,18,18,18,18,18,18,19,19,19,19,19,19,19,
  20,20,20,20,20,20,20,21,21,21,21,21,21,22,22,22,
  22,22,22,22,23,23,23,23,23,23,24,24,24,24,24,24,
  24,25,25,25,25,25,25,26,26,26,26,26,26,26,27,27,
  27,27,27,27,28,28,28,28,28,28,28,29,29,29,29,29,
  29,30,30,30,30,30,30,30,31,31,31,31,31,31,32,32,
  32,32,32,32,32,33,33,33,33,33,33,33,34,34,34,34,
  34,34,35,35,35,35,35,35,35,36,36,36,36,36,36,37,
  37,37,37,37,37,37,38,38,38,38,38,38,39,39,39,39
};

/* Sextants:0 through 5
 * Axis parts: 16 through 21
 * Center: 32
 * Invalid: 64
 */
char sexttab[]=
{
  64,64,00, 64,64,20, 02,21,01,
  64,64,19, 64,32,64, 16,64,64,
  04,18,05, 17,64,64, 03,64,64
};

int colorint(unsigned short colorshort)
{
  int r,g,b,ret;
  if (colorshort<64000)
  {
    b=colorshort%40;
    r=colorshort/1600;
    g=(colorshort-r*1600)/40;
    r=tab40[r];
    g=tab40[g];
    b=tab40[b];
    ret=(r<<16)|(g<<8)|b;
  }
  else
    ret=colorshort+0x7fff0000;
  return ret;
}

unsigned short colorshort(int colorint)
{
  int r,g,b;
  unsigned short ret;
  if (colorint<16777216)
  {
    r=(colorint&0xff0000)>>16;
    g=(colorint&0xff00)>>8;
    b=colorint&0xff;
    r=tab256[r];
    g=tab256[g];
    b=tab256[b];
    ret=(r*1600)+(g*40)+b;
  }
  else
    ret=colorint&0xffff;
  return ret;
}

/* Flipping the hue:
 * *  *
 * 
 *  *    *
 * * *  * *
 * 
 *   r      c
 *  y m    b g
 * g c b  m r y
 * 
 *    *        *
 *   y m      c c
 *  y * m    m * y
 * * c c *  * m y *
 * 
 *     r          c
 *    r r        b g
 *   y * m      b * g
 *  g * * b    b * * g
 * g g c b b  m r r r y
 * 
 *      r            c
 *     r r          c c
 *    y * m        b * g
 *   y y m m      b b g g
 *  g * c * b    m * r * y
 * g g c c b b  m m r r y y
 * 
 *       r              c
 *      r r            c c
 *     y * m          c * c
 *    y y m m        b b y y
 *   y y * m m      m b * g y
 *  g * c c * b    m * r r * y
 * g g c c c b b  m m m r y y y
 * 
 *        r
 *       r r
 *      r r r
 *     y y m m
 *    y y | m m
 *   g y / \ m b
 *  g g c c c b b
 * g g g c c b b b
 * 
 *        17
 *          
 *      3 | 4
 *  16         18
 *      \ | /  
 *   2   / \   5
 *    /   |   \  
 * 21  1     0   19
 *        20
 */
int fliphue(int color)
{
  int r,g,b,val,ring,sext,sext1,hilim,lolim,hilim2,lolim2;
  r=(color&0xff0000)>>16;
  g=(color&0xff00)>>8;
  b=color&0xff;
  val=r+g+b;
  r=3*r-val;
  g=3*g-val;
  b=3*b-val;
  sext=((r>g)-(g>r))*9+((g>b)-(b>g))*3+((b>r)-(r>b));
  sext=sexttab[sext+13];
  if (val<255)
    hilim=val*2;
  else
    hilim=765-val;
  if (val<=510)
    lolim=-val;
  else
    lolim=2*(val-765);
  /* If hilim:lolim is even, there's a midpoint in the top:bottom line.
   * If val is a multiple of 3, there is a center point where altitudes intersect.
   * val hilim lolim
   *   0    0     0 one point is midpoint
   *   1    2    -1 three points, each altitude has one point
   *   2    4    -2 six points, each altitude has two points, sides have midpoints
   *   3    6    -3 ten points, each altitude has two points but the midpoint is skipped, so the other point (corner) is fixed
   *   4    8    -4 15 points, each alt has 3 points, corner xch opposite midpoint
   *   5   10    -5 21 points, each alt has 3 points, corner xch midpoint of 1st ring
   *   6   12    -6 28 points, each alt has 4 points but midpoint is skipped
   * ..............
   * 250  500  -250
   * 251  502  -251
   * 252  504  -252
   * 253  506  -253
   * 254  508  -254
   * 255  510  -255
   * 256  509  -256
   * 257  508  -257
   * 258  507  -258
   * 259  506  -259
   * 260  505  -260
   * ..............
   * 509  256  -509
   * 510  255  -510
   * 511  254  -508
   * ..............
   * 763    2    -4
   * 764    1    -2
   * 765    0     0
   */
  hilim2=hilim-3*(hilim&1);
  lolim2=lolim+3*(lolim&1);
  //cout<<"r="<<r<<" g="<<g<<" b="<<b<<" sext="<<sext<<" val="<<val;
  if ((sext&240)==16) // on one altitude
  {
    switch (sext)
    {
      case 17:
      case 20:
	r=(hilim2+lolim2)-r;
	g=b=-r/2;
	break;
      case 18:
      case 21:
	g=(hilim2+lolim2)-g;
	b=r=-g/2;
	break;
      case 16:
      case 19:
	b=(hilim2+lolim2)-b;
	r=g=-b/2;
	break;
    }
    sext1=((r>g)-(g>r))*9+((g>b)-(b>g))*3+((b>r)-(r>b));
    sext1=sexttab[sext1+13];
    if (abs(sext1-sext)!=3 && val%3==0)
      switch (sext)
      {
	case 17:
	  r-=6;
	  g+=3;
	  b+=3;
	  break;
	case 20:
	  r+=6;
	  g-=3;
	  b-=3;
	  break;
	case 18:
	  r+=3;
	  g-=6;
	  b+=3;
	  break;
	case 21:
	  r-=3;
	  g+=6;
	  b-=3;
	  break;
	case 16:
	  r+=3;
	  g+=3;
	  b-=6;
	  break;
	case 19:
	  r-=3;
	  g-=3;
	  b+=6;
	  break;
      }
  }
  //cout<<" r="<<r<<" g="<<g<<" b="<<b<<" sext="<<sext1<<" val="<<val<<endl;
  r=(r+val)/3;
  g=(g+val)/3;
  b=(b+val)/3;
  assert(r>=0 && r<256 && g>=0 && g<256 && b>=0 && b<256);
  color=(r<<16)+(g<<8)+b;
  return color;
}

int printingcolor(int color,int op)
{
  color&=16777215;
  if (op&1)
    if (color==0 || color==16777215)
      color^=16777215;
  if (op&2)
    color^=16777215;
  if (op&4)
    color=fliphue(color);
  return color;
}

void setColor(QPen &pen,unsigned short color)
{
  assert(color<64000);
  pen.setColor(QColor((QRgb)(colorint(color)|0xff000000)));
}
