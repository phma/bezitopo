/******************************************************/
/*                                                    */
/* closure.cpp - closure and ratio of precision       */
/*                                                    */
/******************************************************/
/* Copyright 2015-2018 Pierre Abbat.
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
#include "globals.h"
#include "except.h"
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

void setpt_cl(string args)
/* If spstatus==0, sets both startpoint and endpoint to args;
 * If spstatus>0, sets endpoint to args and sets spstatus to 2.
 * spstatus is set to 1 when a call is traversed.
 */
{
  xy point;
  point=parsexy(args);
  if (point.isfinite())
  {
    if (spstatus)
    {
      endpoint=point;
      spstatus=2;
      cout<<"Endpoint: "<<endpoint.getx()<<','<<endpoint.gety()<<endl;
    }
    else
    {
      endpoint=startpoint=point;
      cout<<"Startpoint: "<<startpoint.getx()<<','<<startpoint.gety()<<endl;
    }
  }
  else
    cout<<"Could not parse \""<<args<<"\" as a 2D point"<<endl;
}

void closure_i(string args)
{
  xy displacement,vector;
  double perimeter,area,misclosure;
  size_t chpos;
  int bearing,unitp,i,cmd;
  double distance;
  string input,bearingstr,distancestr,cmdstr,argstr;
  bool validbearing,validdistance;
  endpoint=startpoint=xy(0,0);
  spstatus=0;
  clcommands.clear();
  clcommands.push_back(command("p",setpt_cl,"Set the start or end point: e,n"));
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
    if (cmd<0 && chpos>0 && chpos!=string::npos)
    {
      chpos=input.find_last_of(' ',chpos); // split the string at the last space before the last digit
      if (chpos>input.length())
	chpos=input.length();
      bearingstr=input.substr(0,chpos);
      distancestr=input.substr(chpos);
      validbearing=validdistance=true;
      try
      {
	bearing=parsebearing(bearingstr,DEGREE);
      }
      catch(BeziExcept e)
      {
	validbearing=false;
      }
      try
      {
        distance=doc.ms.parseMeasurement(distancestr,LENGTH).magnitude;
      }
      catch(...)
      {
        validdistance=false;
      }
    }
    else
      validbearing=validdistance=false;
    if (cmd>=0)
      clcommands[cmd].fun(argstr);
    else
    {
      if (validbearing && validdistance)
      {
	vector=cossin(bearing)*distance;
	area+=area3(xy(0,0),displacement,displacement+vector);
	perimeter+=vector.length();
	displacement+=vector;
	cout<<(startpoint+displacement).east()<<' '<<(startpoint+displacement).north()<<' '<<bintoangle(atan2i(displacement),DEGREE+SEXAG1)<<' '<<displacement.length()<<endl;
	if (!spstatus)
	  spstatus++;
      }
      else if (input.length())
      {
        if (!validbearing)
          cout<<"Could not parse \""<<bearingstr<<"\" as a bearing"<<endl;
        if (!validdistance)
          cout<<"Could not parse \""<<distancestr<<"\" as a distance"<<endl;
      }
    }
  }
  while (input.length());
  misclosure=(startpoint+displacement-endpoint).length();
  cout<<"Misclosure: "<<doc.ms.formatMeasurementUnit(misclosure,LENGTH)<<endl;
  cout<<"Perimeter: "<<doc.ms.formatMeasurementUnit(perimeter,LENGTH)<<endl;
  cout<<"Area: "<<doc.ms.formatMeasurementUnit(area,AREA)<<endl;
  cout<<"Ratio of precision: 1:"<<perimeter/misclosure<<endl;
}
