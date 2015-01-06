/******************************************************/
/*                                                    */
/* closure.cpp - closure and ratio of precision       */
/*                                                    */
/******************************************************/

#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include "closure.h"
#include "point.h"
#include "angle.h"
using namespace std;

void closure_i()
{
  xy displacement,vector;
  double perimeter;
  size_t chpos;
  int bearing,unitp;
  double distance;
  char *distcpy=NULL;
  string input,bearingstr,distancestr;
  do
  {
    cout<<"cl> ";
    cout.flush();
    getline(cin,input);
    chpos=input.find_last_of("0123456789");
    if (chpos>0 && chpos!=string::npos)
    {
      chpos=input.find_last_of(' '); // split the string at the last space before the last digit
      bearingstr=input.substr(0,chpos);
      distancestr=input.substr(chpos);
      bearing=parsebearing(bearingstr,DEGREE);
      distcpy=(char *)realloc(distcpy,distancestr.length()+1);
      strcpy(distcpy,distancestr.c_str());
      distance=parse_meas(distcpy,METER,&unitp);
      vector=cossin(bearing)*distance;
      displacement+=vector;
      cout<<displacement.east()<<' '<<displacement.north()<<endl;
    }
  }
  while (input.length());
  free(distcpy);
}
