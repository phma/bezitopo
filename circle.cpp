/******************************************************/
/*                                                    */
/* circle.cpp - circles, including lines              */
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

#include "circle.h"
#include "angle.h"
#include "ldecimal.h"

using namespace std;

Circle::Circle()
// Constructs the x-axis.
{
  mid=xy(0,0);
  rbear=bear=0;
  cur=0;
}

Circle::Circle(xy c,double r)
{
  bear=atan2i(c);
  mid=c-r*cossin(bear);
  bear-=DEG90;
  cur=1/r;
  rbear=bintorad(bear);
}

Circle::Circle(xy m,int b,double c)
{
  mid=m;
  bear=b;
  cur=c;
  rbear=bintorad(bear);
}

xyz Circle::station(double along) const
{
  double angalong=cur/along;
  double rad;
  if (cur)
    rad=sin(angalong/2)*2/cur;
  else
    rad=along;
  return xyz(mid+cossin(rbear+angalong/2)*rad,0);
}

double Circle::length() const
{
  return 2*M_PI/cur;
}

void Circle::_roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
{
  mid._roscat(tfrom,ro,sca,cis,tto);
  bear+=ro;
  cur/=sca;
}

void Circle::roscat(xy tfrom,int ro,double sca,xy tto)
{
  _roscat(tfrom,ro,sca,cossin(ro)*sca,tto);
}

unsigned Circle::hash()
{
  return memHash(&mid,sizeof(xy),
         memHash(&bear,sizeof(int),
         memHash(&cur,sizeof(double))));
}

void Circle::writeXml(ofstream &ofile)
{
  ofile<<"<circle bearing=\""<<bear<<"\" curvature=\""<<ldecimal(cur)<<"\">";
  mid.writeXml(ofile);
  ofile<<"</circle>\n";
}