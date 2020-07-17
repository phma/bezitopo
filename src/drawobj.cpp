/******************************************************/
/*                                                    */
/* drawobj.cpp - drawing object base class            */
/*                                                    */
/******************************************************/
/* Copyright 2015-2020 Pierre Abbat.
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

#include <cmath>
#include "drawobj.h"
#include "penwidth.h"
#include "color.h"
#include "linetype.h"
#include "geoid.h"
using namespace std;

string xmlEscape(string str)
{
  string ret;
  size_t pos;
  do
  {
    pos=str.find_first_of("\"&'<>");
    ret+=str.substr(0,pos);
    if (pos!=string::npos)
      switch (str[pos])
      {
	case '"':
	  ret+="&quot;";
	  break;
	case '&':
	  ret+="&amp;";
	  break;
	case '\'':
	  ret+="&apos;";
	  break;
	case '<':
	  ret+="&lt;";
	  break;
	case '>':
	  ret+="&gt;";
	  break;
      }
    str.erase(0,pos+(pos!=string::npos));
  } while (str.length());
  return ret;
}

drawingElement::drawingElement()
{
  color=SAMECOLOR;
  width=SAMEWIDTH;
  linetype=SAMELINETYPE;
  filled=false;
}

drawingElement::drawingElement(bezier3d b3d)
{
  color=SAMECOLOR;
  width=SAMEWIDTH;
  linetype=SAMELINETYPE;
  filled=false;
  path=b3d;
}

drawingElement::drawingElement(bezier3d b3d,int c,int w,int lt)
{
  color=c;
  width=w;
  linetype=lt;
  filled=false;
  path=b3d;
}

unsigned memHash(void *mem,unsigned len,unsigned previous)
/* This is used to tell whether a drawing object has changed
 * and must therefore be regenerated.
 */
{
  unsigned i;
  unsigned *umem=reinterpret_cast<unsigned *>(mem);
  unsigned ulen=len/sizeof(unsigned);
  for (i=0;i<ulen;i++)
    previous=byteswap(previous*59049+umem[i]);
  return previous;
}

int drawobj::type()
{
  return OBJ_DRAWOBJ;
}

bsph drawobj::boundsphere()
{
  bsph ret;
  ret.center=xyz(NAN,NAN,NAN);
  ret.radius=0;
  return ret;
}

bool drawobj::hittest(hline hitline)
{
  return false;
}

void drawobj::roscat(xy tfrom,int ro,double sca,xy tto)
/* Rotate, scale, and translate. Two xys are provided to accurately rotate
 * and scale something around a point, simultaneously rotate and translate, etc.
 * Does not affect the z scale. Always calls _roscat to avoid repeatedly
 * computing the sine and cosine of ro, except in the base drawobj class,
 * which has no location. 
 */
{
}

unsigned drawobj::hash()
{
  return 0;
}

double drawobj::dirbound(int angle,double boundsofar)
{
  return INFINITY;
}

bezier3d drawobj::approx3d(double precision)
{
  return bezier3d();
}

vector<drawingElement> drawobj::render3d(double precision,int layer,int color,int width,int linetype)
{
  return vector<drawingElement>();
}

/* hasProperty, getDoubleProperty, getStringProperty, etc. will be used to make
 * one object refer to another, such as a dimension label which displays
 * the length of a line. hasProperty tells whether the object has the property.
 * 
 * An object can be of a class which has a property without having the property,
 * e.g. an open polyline does not have the area property even though a closed
 * polyline does. Getting a nonexistent double property returns NaN. Getting
 * an existent property can return NaN, e.g. a spiralarc with clothance=0
 * and curvature and length making it a full circle has tangent length NaN.
 * 
 * The property number determines whether it is a double or string property.
 * Calling getDoubleProperty(prop) where prop is a string property returns NaN,
 * even if the object has the property.
 */

bool drawobj::hasProperty(int prop)
{
  return false;
}

int drawobj::getIntProperty(int prop)
{
  return 0;
}

double drawobj::getDoubleProperty(int prop)
{
  return NAN;
}

xy drawobj::getXyProperty(int prop)
{
  return nanxy;
}

xyz drawobj::getXyzProperty(int prop)
{
  return nanxyz;
}

string drawobj::getStringProperty(int prop)
{
  return "";
}

void drawobj::writeXml(ofstream &ofile)
{
  ofile<<"<DrawingObject />";
}
