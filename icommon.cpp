/******************************************************/
/*                                                    */
/* icommon.cpp - common interactive routines          */
/*                                                    */
/******************************************************/

#include <iostream>
#include "icommon.h"
#include "measure.h"
using namespace std;

bool subcont;

string firstarg(string &args)
{
  size_t pos;
  string ret;
  pos=args.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  args.erase(0,pos);
  pos=args.find(' ');
  ret=args.substr(0,pos);
  args.erase(0,pos);
  pos=args.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  args.erase(0,pos);
  return ret;
}

string trim(string word)
{
  size_t pos;
  pos=word.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  word.erase(0,pos);
  pos=word.find_last_not_of(' ');
  if (pos==string::npos)
    pos=0;
  else
    pos++;
  word.erase(pos,string::npos);
  return word;
}

void setfoot_i(string args)
{
  args=trim(args);
  if (args=="int'l")
    setfoot(INTERNATIONAL);
  else if (args=="US")
    setfoot(USSURVEY);
  else if (args=="Indian")
    setfoot(INSURVEY);
  else
    cout<<"I don't recognize that foot"<<endl;
}

void setlengthunit_i(string args)
{
  args=trim(args);
  if (args=="m")
    set_length_unit(METER+DEC3);
  else if (args=="ft")
    set_length_unit(FOOT+DEC2);
  else if (args=="ch")
    set_length_unit(CHAIN+DEC2);
  else
    cout<<"I don't recognize that length unit"<<endl;
}

arangle parsearangle(string angstr,int unitp)
/* 13, angle to the right, relative
 * -13, angle to the left, relative
 * d13, deflection angle, same as 193, relative
 * n13e, bearing, absolute
 */
{
  arangle ret;
  int def=0;
  if (tolower(angstr[0])=='d' || !isalpha(angstr[0]))
  {
    ret.rel=true;
    if (tolower(angstr[0])=='d')
    {
      angstr.erase(0,1);
      def=DEG180;
    }
    ret.ang=-parsesignedangle(angstr,unitp);
  }
  else
  {
    ret.rel=false;
    ret.ang=parsebearing(angstr,unitp);
  }
  return ret;
}

xy parsexy(string xystr)
{
  size_t pos;
  string xstr,ystr;
  xy ret;
  int dummy;
  pos=xystr.find(',');
  if (pos==string::npos)
    ret=xy(NAN,NAN);
  else
  {
    xstr=xystr.substr(0,pos);
    ystr=xystr.substr(pos+1);
    ret=xy(parse_length(xstr),parse_length(ystr));
  }
  return ret;
}

void subexit(string args)
{
  subcont=false;
}
