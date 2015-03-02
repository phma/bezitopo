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
  cout<<'<'<<args<<'>'<<endl;
  if (args=="int'l")
    setfoot(INTERNATIONAL);
  else if (args=="US")
    setfoot(USSURVEY);
  else if (args=="Indian")
    setfoot(INSURVEY);
  else
    cout<<"I don't recognize that foot"<<endl;
}
