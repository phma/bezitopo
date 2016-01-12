/******************************************************/
/*                                                    */
/* drawobj.cpp - drawing object base class            */
/*                                                    */
/******************************************************/

#include <cmath>
#include "drawobj.h"
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

void drawobj::writeXml(ofstream &ofile)
{
  ofile<<"<DrawingObject />";
}
