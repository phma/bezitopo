/******************************************************/
/*                                                    */
/* icommon.cpp - common interactive routines          */
/*                                                    */
/******************************************************/

#include <iostream>
#include "icommon.h"
#include "measure.h"
using namespace std;

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

