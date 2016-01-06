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
#include "mkpoint.h"
#include "closure.h"
#include "cvtmeas.h"
#include "contour.h"

using namespace std;

bool cont=true;
document doc;

void indpark(string args)
{
  int i,j,itype;
  double w,e,s,n;
  ofstream ofile("IndependencePark.bez");
  criteria crit;
  criterion crit1;
  setfoot(USSURVEY);
  set_length_unit(FOOT+DEC2);
  if (doc.readpnezd("topo0.asc")<0)
    doc.readpnezd("../topo0.asc");
  crit1.str="";
  crit1.istopo=true;
  crit.push_back(crit1);
  crit1.str="FH";
  crit1.istopo=false; // The point labeled FH has a nonsensical elevation and must be removed.
  crit.push_back(crit1);
  doc.copytopopoints(crit);
  //rotate(2);
  doc.pl[1].maketin("bezitopo.ps");
  doc.pl[1].makegrad(0.15);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient(false);
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  w=doc.pl[1].dirbound(degtobin(0));
  s=doc.pl[1].dirbound(degtobin(90));
  e=-doc.pl[1].dirbound(degtobin(180));
  n=-doc.pl[1].dirbound(degtobin(270));
  cout<<"Writing topo with curved triangles"<<endl;
  rasterdraw(doc.pl[1],xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark.ppm");
  /* The first (lowest) contour in which a NaN control point is drawn on a
   * polyspiral contour segment whose ends are real is the following:
   * 302.559,316.867 302.398,316.802 302.488,316.551 302.557,316.826 302.517,316.637
   * Draw the surrounding region to find out where it is. It is next to the
   * box culvert on the northwest side of the field. The surface is grainy.
   */
  rasterdraw(doc.pl[1],xy(443302.5,164316.7),0.25,0.35,1000,0,10,"IPmicro.ppm");
  rasterdraw(doc.pl[1],xy(443302.5,164316.7),7,7,100,0,10,"IPmini.ppm");
  roughcontours(doc.pl[1],0.1);
  doc.pl[1].removeperimeter();
  smoothcontours(doc.pl[1],0.1);
  psopen("IndependencePark.ps");
  psprolog();
  startpage();
  setscale(w,s,e,n,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
    spline(doc.pl[1].contours[i].approx3d(0.1));
  endpage();
  pstrailer();
  psclose();
  doc.writeXml(ofile);
  doc.pl[1].setgradient(true);
  cout<<"Writing topo with flat triangles"<<endl;
  rasterdraw(doc.pl[1],xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark-flat.ppm");
}

vector<command> commands;

void readpoints(string args)
// just pnezd for now
{
  doc.readpnezd(trim(args),false);
}

void writepoints(string args)
// just pnezd for now
{
  doc.writepnezd(trim(args));
}

void maketin_i(string args)
{
  int error=0;
  criteria crit;
  criterion crit1;
  crit1.str="";
  crit1.istopo=true;
  crit.push_back(crit1); // will later make a point-selection command
  doc.copytopopoints(crit);
  try
  {
    doc.pl[1].maketin("maketin.ps");
  }
  catch(int e)
  {
    error=e;
  }
  switch (error)
  {
    case notri:
      cout<<"Less than three points selected.\nPlease load a coordinate file or make points."<<endl;
      break;
    case samepnts:
      cout<<"Two points have the same x and y coordinates. Deselect one."<<endl;
      break;
    case flattri:
      cout<<"Couldn't make a TIN. Looks like all the points are collinear."<<endl;
      break;
    default:
      cout<<"Successfully made TIN."<<endl;
      doc.pl[1].makegrad(0.15);
      doc.pl[1].maketriangles();
      doc.pl[1].setgradient(false);
      doc.pl[1].makeqindex();
  }
}

void drawtin_i(string args)
{
  double w,e,s,n;
  if (doc.pl[1].edges.size())
  {
    w=doc.pl[1].dirbound(degtobin(0));
    s=doc.pl[1].dirbound(degtobin(90));
    e=-doc.pl[1].dirbound(degtobin(180));
    n=-doc.pl[1].dirbound(degtobin(270));
    psopen(trim(args).c_str());
    psprolog();
    setscale(w,s,n,e);
    startpage();
    doc.pl[1].dumpedges_ps(false);
    endpage();
    pstrailer();
    psclose();
  }
  else
    cout<<"No TIN present. Please make a TIN first."<<endl;
}

void rasterdraw_i(string args)
{
  double w,e,s,n;
  w=doc.pl[1].dirbound(degtobin(0));
  s=doc.pl[1].dirbound(degtobin(90));
  e=-doc.pl[1].dirbound(degtobin(180));
  n=-doc.pl[1].dirbound(degtobin(270));
  rasterdraw(doc.pl[1],xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,trim(args));
}

void contourdraw_i(string args)
{
}

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
  commands.push_back(command("mkpoint",mkpoint_i,"Make new points"));
  commands.push_back(command("setfoot",setfoot_i,"Set foot unit: int'l, US, Indian"));
  commands.push_back(command("setlunit",setlengthunit_i,"Set length unit: m, ft, ch"));
  commands.push_back(command("cvtmeas",cvtmeas_i,"Convert measurements"));
  commands.push_back(command("read",readpoints,"Read coordinate file in PNEZD format: filename"));
  commands.push_back(command("write",writepoints,"Write coordinate file in PNEZD format: filename"));
  commands.push_back(command("maketin",maketin_i,"Make triangulated irregular network"));
  commands.push_back(command("drawtin",drawtin_i,"Draw TIN: filename.ps"));
  commands.push_back(command("raster",rasterdraw_i,"Draw raster topo: filename.ppm"));
  commands.push_back(command("help",help,"List commands"));
  commands.push_back(command("exit",exit,"Exit the program"));
  doc.pl.resize(1);
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
