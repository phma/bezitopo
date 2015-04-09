/******************************************************/
/*                                                    */
/* cvtmeas.cpp - convert measures interactively       */
/*                                                    */
/******************************************************/

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
      cout<<"Unrecognized command, type \"help\" for a list of commands"<<endl;
  }
  while (subcont);
}
