/******************************************************/
/*                                                    */
/* bezitopo.cpp - main program                        */
/*                                                    */
/******************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "point.h"
#include "cogo.h"
#include "bezitopo.h"
#include "test.h"
#include "tin.h"
#include "measure.h"
#include "pnezd.h"
#include "angle.h"
#include "pointlist.h"
#include "vcurve.h"
#include "raster.h"
#include "ps.h"
#include "icommon.h"
#include "closure.h"

using namespace std;

bool cont=true;

void indpark(string args)
{
  int i,j,itype;
  double w,e,s,n;
  criteria crit;
  criterion crit1;
  set_length_unit(FOOT);
  if (readpnezd("topo0.asc")<0)
    readpnezd("../topo0.asc");
  crit1.str="";
  crit1.istopo=true;
  crit.push_back(crit1);
  crit1.str="FH";
  crit1.istopo=false; // The point labeled FH has a nonsensical elevation and must be removed.
  crit.push_back(crit1);
  copytopopoints(crit);
  //rotate(2);
  topopoints.maketin("bezitopo.ps");
  topopoints.makegrad(0.15);
  topopoints.maketriangles();
  topopoints.setgradient(false);
  topopoints.makeqindex();
  w=topopoints.dirbound(degtobin(0));
  s=topopoints.dirbound(degtobin(90));
  e=-topopoints.dirbound(degtobin(180));
  n=-topopoints.dirbound(degtobin(270));
  cout<<"Writing topo with curved triangles"<<endl;
  rasterdraw(topopoints,xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark.ppm");
  topopoints.setgradient(true);
  cout<<"Writing topo with flat triangles"<<endl;
  rasterdraw(topopoints,xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark-flat.ppm");
}

struct command
{
  string word;
  void (*fun)(string args);
  string desc;
  command(string w,void (*f)(string args),string d)
  {
    word=w;
    fun=f;
    desc=d;
  }
};

vector<command> commands;

void help(string args)
{
  int i;
  for (i=0;i<commands.size();i++)
  {
    cout<<commands[i].word<<"  "<<commands[i].desc<<endl;
  }
}

void exit(string args)
{
  cont=false;
}

int main(int argc, char *argv[])
{
  int i,cmd;
  size_t chpos;
  string cmdline,cmdword,cmdargs;
  commands.push_back(command("indpark",indpark,"Process the Independence Park topo (topo0.asc)"));
  commands.push_back(command("closure",closure_i,"Check closure of a lot"));
  commands.push_back(command("setfoot",setfoot_i,"Set foot unit: int'l, US, Indian"));
  commands.push_back(command("setlunit",setlengthunit_i,"Set length unit: m, ft, ch"));
  commands.push_back(command("help",help,"List commands"));
  commands.push_back(command("exit",exit,"Exit the program"));
  while (cont)
  {
    cout<<"? ";
    cout.flush();
    getline(cin,cmdline);
    chpos=cmdline.find_first_of(' '); // split the string at the first space
    if (chpos>cmdline.length())
      chpos=cmdline.length();
    cmdword=cmdline.substr(0,chpos);
    cmdargs=cmdline.substr(chpos);
    for (cmd=-1,i=0;i<commands.size();i++)
      if (commands[i].word==cmdword)
	cmd=i;
    if (cmd>=0)
      commands[cmd].fun(cmdargs);
    else
      cout<<"Unrecognized command, type \"help\" for a list of commands"<<endl;
  }
  return EXIT_SUCCESS;
}
