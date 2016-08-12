/******************************************************/
/*                                                    */
/* closure.cpp - closure and ratio of precision       */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
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
      if (chpos>input.length())
	chpos=input.length();
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
