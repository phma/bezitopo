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
#include <vector>
#include "icommon.h"
#include "closure.h"
#include "point.h"
#include "cogo.h"
#include "angle.h"
using namespace std;
vector<command> clcommands;
xy startpoint,endpoint;
int spstatus;

void help_cl(string args)
{
  int i;
  for (i=0;i<clcommands.size();i++)
  {
    cout<<clcommands[i].word<<"  "<<clcommands[i].desc<<endl;
  }
  cout<<"bearing distance (e.g. n30-20e 234.5): add call to traverse"<<endl;
  cout<<"Blank line exits."<<endl;
}

void closure_i(string args)
{
  xy displacement,vector;
  double perimeter,area,misclosure;
  size_t chpos;
  int bearing,unitp,i,cmd;
  double distance;
  char *distcpy=NULL;
  string input,bearingstr,distancestr,cmdstr,argstr;
  bool validbearing;
  endpoint=startpoint=xy(0,0);
  clcommands.clear();
  //clcommands.push_back(command("p",setpt_cl,"Set the start or end point: e,n"));
  //clcommands.push_back(command("u",undo_cl,"Undo"));
  clcommands.push_back(command("help",help_cl,"List commands"));
  perimeter=area=0;
  do
  {
    cout<<"cl> ";
    cout.flush();
    getline(cin,input);
    bearingstr=distancestr=cmdstr=argstr="";
    chpos=input.find(' ');
    if (chpos>0 && chpos!=string::npos)
    {
      cmdstr=input.substr(0,chpos);
      argstr=input.substr(chpos);
    }
    else
    {
      cmdstr=input;
      argstr="";
    }
    for (cmd=-1,i=0;i<clcommands.size();i++)
      if (clcommands[i].word==cmdstr)
	cmd=i;
    chpos=input.find_last_of("0123456789");
    if (chpos>0 && chpos!=string::npos)
    {
      chpos=input.find_last_of(' ',chpos); // split the string at the last space before the last digit
      if (chpos>input.length())
	chpos=input.length();
      bearingstr=input.substr(0,chpos);
      distancestr=input.substr(chpos);
      validbearing=true;
      try
      {
	bearing=parsebearing(bearingstr,DEGREE);
      }
      catch(int e)
      {
	validbearing=false;
      }
      distcpy=(char *)realloc(distcpy,distancestr.length()+1);
      strcpy(distcpy,distancestr.c_str());
      distance=parse_length(distcpy);
    }
    if (cmd>=0)
      clcommands[cmd].fun(argstr);
    else
    {
      if (validbearing)
      {
	vector=cossin(bearing)*distance;
	area+=area3(startpoint,displacement,displacement+vector);
	perimeter+=vector.length();
	displacement+=vector;
	cout<<displacement.east()<<' '<<displacement.north()<<' '<<bintoangle(atan2i(displacement),DEGREE+SEXAG1)<<' '<<displacement.length()<<endl;
      }
      else
	cout<<"Could not parse \""<<bearingstr<<"\" as a bearing"<<endl;
    }
  }
  while (input.length());
  cout<<"Misclosure: "<<format_meas_unit((displacement-endpoint).length(),METER+DEC3)<<endl;
  cout<<"Perimeter: "<<perimeter<<endl;
  cout<<"Area: "<<area<<endl;
  cout<<"Ratio of precision: 1:"<<perimeter/displacement.length()<<endl;
  free(distcpy);
}
