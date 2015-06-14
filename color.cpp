/******************************************************/
/*                                                    */
/* color.cpp - drawing colors                         */
/*                                                    */
/******************************************************/

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
 */
int fliphue(int color)
{
  int r,g,b,val,ring,sext;
  r=(color&0xff0000)>>16;
  g=(color&0xff00)>>8;
  b=color&0xff;
  val=r+g+b;
  r=3*r-val;
  g=3*g-val;
  b=3*b-val;
  sext=((r>g)-(g>r))*9+((g>b)-(b>g))*3+((b>r)-(r>b));
  sext=sexttab[sext+13];
  cout<<"r="<<r<<" g="<<g<<" b="<<b<<" sext="<<sext<<endl;
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
