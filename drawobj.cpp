/******************************************************/
/*                                                    */
/* drawobj.cpp - drawing object base class            */
/*                                                    */
/******************************************************/
/* Copyright 2015-2017 Pierre Abbat.
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
#include "drawobj.h"
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

unsigned memHash(unsigned *mem,unsigned len,unsigned previous)
/* This is used to tell whether a drawing object has changed
 * and must therefore be regenerated.
 */
{
  unsigned i;
  for (i=0;i<len;i++)
    previous=byteswap(previous*59049+mem[i]);
  return previous;
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

int drawobj::hash()
{
  return 0;
}

vector<drawingElement> drawobj::render3d(double precision)
{
  return vector<drawingElement>();
}

void drawobj::writeXml(ofstream &ofile)
{
  ofile<<"<DrawingObject />";
}
