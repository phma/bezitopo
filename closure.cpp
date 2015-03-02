/******************************************************/
/*                                                    */
/* closure.cpp - closure and ratio of precision       */
/*                                                    */
/******************************************************/

#include <cstring>
#include <iostream>
#include <cstdlib>
#include "closure.h"
#include "point.h"
#include "cogo.h"
#include "angle.h"
using namespace std;

void closure_i(string args)
{
  xy displacement,vector,origin(0,0);
  double perimeter,area,misclosure;
  size_t chpos;
  int bearing,unitp;
  double distance;
  char *distcpy=NULL;
  string input,bearingstr,distancestr;
  perimeter=area=0;
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
      distance=parse_length(distcpy);
      vector=cossin(bearing)*distance;
      area+=area3(origin,displacement,displacement+vector);
      perimeter+=vector.length();
      displacement+=vector;
      cout<<displacement.east()<<' '<<displacement.north()<<' '<<bintoangle(atan2i(displacement),DEGREE+SEXAG1)<<' '<<displacement.length()<<endl;
    }
  }
  while (input.length());
  cout<<"Misclosure: "<<format_meas_unit(displacement.length(),METER+DEC3)<<endl;
  cout<<"Perimeter: "<<perimeter<<endl;
  cout<<"Area: "<<area<<endl;
  cout<<"Ratio of precision: 1:"<<perimeter/displacement.length()<<endl;
  free(distcpy);
}
