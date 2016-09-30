/******************************************************/
/*                                                    */
/* mkpoint.cpp - make new points                      */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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

/* The commands are:
 * m: make point. Takes a pair of coordinates.
 * o: occupy point. Takes a point number. The backsight angle is dropped.
 * b: backsight point. Takes a point number. The backsight angle is set to
 *    the direction to that point, unless it's the same point, in which case
 *    it's dropped.
 * f: foresight point. Takes a relative or absolute angle. Moves to the new
 *    point, and backsights the previous point.
 * s: sideshot point. Takes a relative or absolute angle. Makes a new point,
 *    but does not move to it or change the backsight angle.
 * l: label. Takes a string which is used as the label for subsequent points.
 * ls:list points. No argument.
 * x: exit.
 */

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "closure.h"
#include "point.h"
#include "cogo.h"
#include "angle.h"
#include "icommon.h"
#include "pointlist.h"
#include "bezitopo.h"

using namespace std;

int backsight_angle;
bool have_position,have_backsight;
xy position;
vector<command> mkcommands;
string label;

void help_mk(string args)
{
  int i;
  for (i=0;i<mkcommands.size();i++)
  {
    cout<<mkcommands[i].word<<"  "<<mkcommands[i].desc<<endl;
  }
}

void mkpt_mk(string args)
{
  xy pointxy;
  pointxy=parsexy(args);
  if (std::isnan(pointxy.east()))
    cout<<"Not a valid point"<<endl;
  else
    doc.pl[0].addpoint(1,point(pointxy,0,label));
}

void label_mk(string args)
{
  label=trim(args);
}

void outpnt(point pnt)
{
  cout<<format_length(pnt.east())<<','<<format_length(pnt.north())<<',';
  cout<<format_length(pnt.elev())<<' '<<pnt.note;
}

void listpoints(string args)
{
  ptlist::iterator i;
  for (i=doc.pl[0].points.begin();i!=doc.pl[0].points.end();i++)
  {
    cout<<i->first<<' ';
    outpnt(i->second);
    cout<<endl;
  }
}

void mkpoint_i(string args)
{
  xy displacement,vector,origin(0,0);
  double perimeter,area,misclosure;
  size_t chpos;
  int i,bearing,unitp,cmd;
  double distance;
  string input,cmdstr,argstr;
  mkcommands.clear();
  mkcommands.push_back(command("l",label_mk,"Set the point label"));
  mkcommands.push_back(command("m",mkpt_mk,"Make point: e,n"));
  mkcommands.push_back(command("ls",listpoints,"List the points"));
  mkcommands.push_back(command("help",help_mk,"List commands"));
  mkcommands.push_back(command("x",subexit,"Exit the mkpoint command"));
  perimeter=area=0;
  subcont=true;
  do
  {
    cout<<"mk> ";
    cout.flush();
    getline(cin,input);
    if (!cin.good() && !input.length())
      input="x";
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
    for (cmd=-1,i=0;i<mkcommands.size();i++)
      if (mkcommands[i].word==cmdstr)
	cmd=i;
    if (cmd>=0)
      mkcommands[cmd].fun(argstr);
    else
      cout<<"Unrecognized command, type \"help\" for a list of commands"<<endl;
  }
  while (subcont);
}
