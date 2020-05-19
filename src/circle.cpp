/******************************************************/
/*                                                    */
/* circle.cpp - circles, including lines              */
/*                                                    */
/******************************************************/
/* Copyright 2017-2020 Pierre Abbat.
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

#include "circle.h"
#include "angle.h"
#include "cogo.h"
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

xy Circle::center() const
{
  return mid+cossin(bear+DEG90)/cur;
}

xyz Circle::station(double along) const
{
  double angalong=cur*along;
  double rad;
  if (cur)
    rad=sin(angalong/2)*2/cur;
  else
    rad=along;
  return xyz(mid+cossin(rbear+angalong/2)*rad,0);
}

int Circle::bearing(double along) const
{
  return bear+radtobin(cur*along);
}

double Circle::curvature() const
{
  return cur;
}

double Circle::radius() const
{
  return 1/cur;
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

double Circle::dirbound(int angle,double boundsofar)
{
  int ang;
  double l;
  if (cur)
  {
    ang=foldangle(angle-bear-DEG90*sign(cur));
    l=bintorad(ang)/cur;
    return xy(station(l)).dirbound(angle);
  }
  else
    return INFINITY;
}

double Circle::visibleLength(double precision)
/* Returns either the negative of the whole length of the circle or 32768*precision,
 * which is less than half the length. This is enough to make sure that all
 * of the circle or line that is visible on an A0 sheet is printed if the midpoint
 * is in view and it's approximated to within 0.1 mm on the paper.
 */
{
  if (cur*precision*16384>1)
    return -length();
  else
    return precision*32768;
}

bezier3d Circle::approx3d(double precision)
{
  double visLength=visibleLength(precision);
  double error=precision*2;
  double piecestart,pieceend;
  int npieces,i;
  bezier3d ret;
  npieces=floor(fabs(visLength*cur*2));
  if (npieces<1)
    npieces=1;
  while (true)
  {
    error=bez3destimate(station(0),bearing(0),visLength/npieces,
			bearing(visLength/npieces),station(visLength/npieces));
    if (error<=precision)
      break;
    npieces=ceil(npieces*sqrt(sqrt(error/precision)));
  }
  for (i=0;i<npieces;i++)
  {
    piecestart=(i-npieces/2.)/npieces*fabs(visLength);
    pieceend=(i+1-npieces/2.)/npieces*fabs(visLength);
    ret+=bezier3d(station(piecestart),bearing(piecestart),0,0,
		  bearing(pieceend),station(pieceend));
  }
  if (visLength<0)
    ret.close();
  return ret;
}

vector<drawingElement> Circle::render3d(double precision,int layer,int color,int width,int linetype)
{
  vector<drawingElement> ret;
  ret.push_back(drawingElement(approx3d(precision),color,width,linetype));
  return ret;
}

bool Circle::hasProperty(int prop)
{
  return (prop>=PROP_LENGTH && prop<=PROP_CURVATURE) ||
         (prop>=PROP_CENTER && prop<=PROP_MIDPOINT_XY);
}

double Circle::getDoubleProperty(int prop)
{
  double ret=NAN;
  switch (prop)
  {
    case PROP_LENGTH:
      ret=length();
      break;
    case PROP_RADIUS:
      ret=radius();
      break;
    case PROP_CURVATURE:
      ret=curvature();
      break;
  }
  return ret;
}

xy Circle::getXyProperty(int prop)
{
  xy ret=nanxy;
  switch (prop)
  {
    case PROP_CENTER:
      ret=center();
      break;
    case PROP_MIDPOINT_XY:
      ret=mid;
      break;
  }
  return ret;
}

void Circle::writeXml(ofstream &ofile)
{
  ofile<<"<circle bearing=\""<<bear<<"\" curvature=\""<<ldecimal(cur)<<"\">";
  mid.writeXml(ofile);
  ofile<<"</circle>\n";
}
