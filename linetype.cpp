/******************************************************/
/*                                                    */
/* linetype.cpp - dash patterns                       */
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

#include <iostream>
#include "linetype.h"
#include "relprime.h"
using namespace std;

void setLineType(QPen &pen,unsigned short lineType)
{
  QVector<qreal> dashes;
  int q,r,npairs,i;
  double mult;
  q=lineType/16275;
  r=lineType%16275;
  npairs=(r<15625)+(r<16250)+1;
  if (q==4)
    switch (r)
    {
      case NOLINE:
        pen.setStyle(Qt::NoPen);
        break;
      case SOLIDLINE:
        pen.setStyle(Qt::SolidLine);
        break;
      case SAMELINETYPE:
        cerr<<"Shouldn't happen: setting pen to SAMELINETYPE"<<endl;
        break;
    }
  else
  {
    switch (q)
    {
      case 0:
        mult=M_1PHI;
        break;
      case 1:
        mult=1;
        break;
      case 2:
        mult=1+M_1PHI;
        break;
      case 3:
        mult=2+M_1PHI;
        break;
    }
    for (i=0;i<npairs;i++)
    {
      dashes<<mult*(r%5);
      r/=5;
      dashes<<mult*(r%5+1);
      r/=5;
    }
    pen.setDashPattern(dashes);
  }
}
