/******************************************************/
/*                                                    */
/* cvtmeas.cpp - convert measures interactively       */
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

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "measure.h"
#include "point.h"
#include "cogo.h"
#include "angle.h"
#include "icommon.h"
#include "cvtmeas.h"
using namespace std;

vector<command> cvcommands;

void help_cv(string args)
{
  int i;
  for (i=0;i<cvcommands.size();i++)
  {
    cout<<cvcommands[i].word<<"  "<<cvcommands[i].desc<<endl;
  }
}

void cvtmeas_i(string args)
{
  size_t chpos;
  int i,bearing,unitp,cmd;
  double distance;
  string input,cmdstr,argstr;
  cvcommands.push_back(command("h",help_cv,"List commands"));
  cvcommands.push_back(command("x",subexit,"Exit the cvtmeas command"));
  subcont=true;
  do
  {
    cout<<"cv> ";
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
    for (cmd=-1,i=0;i<cvcommands.size();i++)
      if (cvcommands[i].word==cmdstr)
	cmd=i;
    if (cmd>=0)
      cvcommands[cmd].fun(argstr);
    else
      cout<<"Unrecognized command, type \"h\" for a list of commands"<<endl;
  }
  while (subcont);
}
