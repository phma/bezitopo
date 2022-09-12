/******************************************************/
/*                                                    */
/* bezitopo.cpp - main program                        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013,2015-2019,2022 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdlib>
#include "config.h"
#include "point.h"
#include "cogo.h"
#include "globals.h"
#include "except.h"
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
#include "firstarg.h"
#include "mkpoint.h"
#include "closure.h"
#include "cvtmeas.h"
#include "scalefactor.h"
#include "contour.h"
#include "geoid.h"
#include "curvefit.h"
#include "csv.h"
#include "ldecimal.h"

using namespace std;

bool cont=true;
document doc;
string savefilename;
geoheader ghead;
ProjectionList allProjections;

void readAllProjections()
{
  ifstream pfile(string(SHARE_DIR)+"/projections.txt");
  allProjections.readFile(pfile);
}

void indpark(string args)
{
  int i,j,itype,npoints;
  double w,e,s,n;
  ofstream ofile("IndependencePark.bez");
  criterion crit1;
  PostScript ps;
  doc.offset=xyz(0,0,0);
  doc.makepointlist(1);
  doc.pl[0].clear();
  doc.ms.setFoot(USSURVEY);
  doc.ms.setCustomary();
  doc.ms.setDefaultUnit(LENGTH,0.552); // geometric mean of meter and foot
  doc.ms.setDefaultPrecision(LENGTH,1.746e-3); // g.m. of 1 mm and 0.01 ft
  npoints=doc.readpnezd("topo0.asc");
  if (npoints<0)
    npoints=doc.readpnezd("../topo0.asc");
  if (npoints<0)
  {
    cerr<<"Can't read topo0.asc"<<endl;
    return;
  }
  doc.pl[1].readCriteria("topo0.crit",doc.ms);
  if (doc.pl[1].crit.size()==0)
    doc.pl[1].readCriteria("../topo0.crit",doc.ms);
  if (doc.pl[1].crit.size()==0)
  {
    cerr<<"Can't read topo0.crit, ignoring just the fire hydrant"<<endl;
    crit1.str="";
    crit1.istopo=true;
    doc.pl[1].crit.push_back(crit1);
    crit1.str="FH";
    crit1.istopo=false; // The point labeled FH has a nonsensical elevation and must be removed.
    doc.pl[1].crit.push_back(crit1);
  }
  try
  {
    i=0;
    doc.pl[1].readBreaklines("topo0.brk");
  }
  catch (BeziExcept &e)
  {
    i=e.getNumber();
  }
  if (i)
  {
    try
    {
      i=0;
      doc.pl[1].readBreaklines("../topo0.brk");
    }
    catch (BeziExcept &e)
    {
      i=e.getNumber();
    }
  }
  if (i)
    cerr<<"Can't read breaklines from topo0.brk"<<endl;
  doc.copytopopoints(1,0);
  //doc.changeOffset(xyz(443392,164096,208));
  doc.pl[1].maketin("bezitopo.ps");
  doc.pl[1].makegrad(0.15);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient(false);
  checkedgediscrepancies(doc.pl[1]);
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  w=doc.pl[1].dirbound(degtobin(0));
  s=doc.pl[1].dirbound(degtobin(90));
  e=-doc.pl[1].dirbound(degtobin(180));
  n=-doc.pl[1].dirbound(degtobin(270));
  cout<<"Writing topo with curved triangles"<<endl;
  rasterdraw(doc.pl[1],xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark.ppm");
  /* There's a tangle in a red contour at elevation 212. Draw the surrounding region.
   */
  rasterdraw(doc.pl[1],xy(443482.5,164115.5)-(xy)doc.offset,0.25,0.35,1000,0,10,"IPmicro.ppm");
  rasterdraw(doc.pl[1],xy(443482.5,164115.5)-(xy)doc.offset,7,7,100,0,10,"IPmini.ppm");
  roughcontours(doc.pl[1],0.1);
  doc.pl[1].removeperimeter();
  smoothcontours(doc.pl[1],0.1);
  ps.open("IndependencePark.ps");
  ps.setpaper(papersizes["A4 landscape"],0);
  ps.prolog();
  ps.startpage();
  ps.setscale(w,s,e,n,0);
  //ps.setscale(443479,164112,443486,164119,0);
  ps.setcolor(0,0.6,0.6);
  for (i=0;i<doc.pl[1].edges.size();i++)
    ps.spline(doc.pl[1].edges[i].getsegment().approx3d(1));
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    if (i<0 && doc.pl[1].contours[i].getElevation()>doc.pl[1].contours[i-1].getElevation())
    // debugging: "i>0" puts each contour elevation on a separate page; "i<0" disables
    {
      ps.endpage();
      ps.startpage();
      ps.setscale(w,s,e,n,0);
    }
    switch (lrint(doc.pl[1].contours[i].getElevation()/0.1)%10)
    {
      case 0:
	ps.setcolor(1,0,0);
	break;
      case 5:
	ps.setcolor(0,0,1);
	break;
      default:
	ps.setcolor(0,0,0);
    }
    ps.comment("Elevation "+ldecimal(doc.pl[1].contours[i].getElevation())+" Contour #"+to_string(i));
    ps.spline(doc.pl[1].contours[i].approx3d(0.1));
  }
  ps.endpage();
  ps.trailer();
  ps.close();
  doc.writeXml(ofile);
  doc.pl[1].setgradient(true);
  cout<<"Writing topo with flat triangles"<<endl;
  rasterdraw(doc.pl[1],xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark-flat.ppm");
}

vector<command> commands;

void readpoints(string args)
{
  string filename,format;
  filename=trim(firstarg(args));
  format=trim(args);
  if (format=="pnezd" || format=="")
    doc.readpnezd(filename,false);
  else if (format=="penzd")
    doc.readpenzd(filename,false);
  else
    cout<<"Formats: pnezd (default), penzd"<<endl;
}

void writepoints(string args)
{
  string filename,format;
  filename=trim(firstarg(args));
  format=trim(args);
  if (format=="pnezd" || format=="")
    doc.writepnezd(filename);
  else if (format=="penzd")
    doc.writepenzd(filename);
  else
    cout<<"Formats: pnezd (default), penzd"<<endl;
}

void maketin_i(string args)
{
  int error=0;
  //criteria crit;
  criterion crit1;
  doc.makepointlist(1);
  crit1.str="";
  crit1.istopo=true;
  doc.pl[1].crit.clear();
  doc.pl[1].crit.push_back(crit1); // will later make a point-selection command
  doc.copytopopoints(1,0);
  try
  {
    doc.pl[1].maketin("maketin.ps");
  }
  catch(BeziExcept e)
  {
    error=e.getNumber();
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
  PostScript ps;
  if (doc.pl[1].edges.size())
  {
    w=doc.pl[1].dirbound(degtobin(0));
    s=doc.pl[1].dirbound(degtobin(90));
    e=-doc.pl[1].dirbound(degtobin(180));
    n=-doc.pl[1].dirbound(degtobin(270));
    ps.open(trim(args));
    ps.setDoc(doc);
    ps.prolog();
    ps.setscale(w,s,n,e);
    ps.startpage();
    doc.pl[1].dumpedges_ps(ps,false);
    ps.endpage();
    ps.trailer();
    ps.close();
  }
  else
    cout<<"No TIN present. Please make a TIN first."<<endl;
}

void trin_i(string args)
{
  triangle *tri;
  xy pnt;
  if (doc.pl[1].edges.size())
  {
    pnt=parsexy(args);
    tri=doc.pl[1].qinx.findt(pnt);
    if (tri)
      cout<<doc.pl[1].revpoints[tri->a]<<' '<<doc.pl[1].revpoints[tri->b]<<' '<<doc.pl[1].revpoints[tri->c]<<endl;
    else
      cout<<"Not in a triangle"<<endl;
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
  string contervalstr;
  double conterval=0;
  double w,e,s,n;
  int i,j;
  PostScript ps;
  contervalstr=firstarg(args);
  try
  {
    conterval=doc.ms.parseMeasurement(contervalstr,LENGTH).magnitude;
  }
  catch (BeziExcept e)
  {
    cerr<<"\""<<contervalstr<<"\": ";
    if (e.getNumber()==badunits)
      cerr<<"unit symbol is not a length unit";
    else if (e.getNumber()==badnumber)
      cerr<<"number is missing";
    else
      cerr<<"an error happened";
    cerr<<endl;
    conterval=NAN;
  }
  if (conterval>5e-6 && conterval<1e5)
    if (doc.pl.size()>1 && doc.pl[1].edges.size())
    {
      doc.pl[1].findcriticalpts();
      doc.pl[1].addperimeter();
      roughcontours(doc.pl[1],conterval);
      doc.pl[1].removeperimeter();
      smoothcontours(doc.pl[1],conterval,true,true);
      w=doc.pl[1].dirbound(degtobin(0));
      s=doc.pl[1].dirbound(degtobin(90));
      e=-doc.pl[1].dirbound(degtobin(180));
      n=-doc.pl[1].dirbound(degtobin(270));
      ps.open(args.c_str());
      ps.prolog();
      ps.startpage();
      ps.setscale(w,s,e,n,0);
      ps.setcolor(0,0.6,0.6);
      for (i=0;i<doc.pl[1].edges.size();i++)
	ps.spline(doc.pl[1].edges[i].getsegment().approx3d(1));
      ps.setcolor(0,1,1);
      for (i=0;i<doc.pl[1].triangles.size();i++)
	for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
	  ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
      for (i=0;i<doc.pl[1].contours.size();i++)
      {
	switch (lrint(doc.pl[1].contours[i].getElevation()/conterval)%10)
	{
	  case 0:
	    ps.setcolor(1,0,0);
	    break;
	  case 5:
	    ps.setcolor(0,0,1);
	    break;
	  default:
	    ps.setcolor(0,0,0);
	}
	ps.comment("Elevation "+ldecimal(doc.pl[1].contours[i].getElevation())+" Contour #"+to_string(i));
	ps.spline(doc.pl[1].contours[i].approx3d(0.1));
      }
      ps.endpage();
      ps.trailer();
      ps.close();
    }
    else
      cout<<"No TIN present. Please make a TIN first."<<endl;
  else if (std::isfinite(conterval))
    cout<<"Contour interval should be between 5 µm and 10 km"<<endl;
}

void save_i(string args)
{
  ofstream ofile;
  args=trim(args);
  if (args.length())
    savefilename=args;
  if (savefilename.length())
  {
    ofile.open(savefilename);
    doc.writeXml(ofile);
    ofile.close();
  }
  else
    cout<<"No filename specified"<<endl;
}

void bdiff_i(string args)
{
  arangle bear1,bear2;
  string arg1,arg2;
  arg1=firstarg(args);
  arg2=firstarg(args);
  bear1=parsearangle(arg1,DEGREE);
  bear2=parsearangle(arg2,DEGREE);
  cout<<bintoangle(bear1.ang-bear2.ang,DEGREE+SEXAG2)<<endl;
}

void readgeoid_i(string args)
// Attempting to read a non-geoid file leaves the geoid unchanged.
{
  string geoidfilename;
  args=trim(args);
  if (args.length())
    geoidfilename=args;
  if (geoidfilename.length())
  {
    try
    {
      ifstream geofile(geoidfilename,ios::binary);
      ghead.readBinary(geofile);
      cube.scale=pow(2,ghead.logScale);
      cube.readBinary(geofile);
      cout<<"read "<<geoidfilename<<endl;
      //ofstream geodump("readgeoid.dump");
      //cube.dump(geodump);
    }
    catch(BeziExcept e)
    {
      cout<<"didn't read "<<geoidfilename<<" for reason "<<e.message().toStdString()<<endl;
    }
  }
  else
    cout<<"No filename specified"<<endl;
}

void curvefit_i(std::string args)
/* Reads the named file as CSV. The line with one column is the tolerance,
 * lines with two columns are points, and lines with three columns are the
 * starting and ending lines and any hints between them, in order. Lines are
 * specified by x, y, and bearing.
 */
{
  string filename;
  filename=trim(firstarg(args));
  ifstream infile;
  double n,e,toler=0;
  vector<string> words;
  vector<xy> points;
  deque<Circle> lines;
  Circle startLine,endLine;
  polyarc fitCurve;
  string line,nstr,estr,bearstr;
  arc oneArc;
  int i,npoints,bear,lastbear=0;
  infile.open(filename);
  npoints=-(!infile.is_open());
  if (infile.is_open())
  {
    do
    {
      getline(infile,line);
      while (line.length() && (line.back()=='\n' || line.back()=='\r'))
	line.pop_back();
      words=parsecsvline(line);
      switch(words.size())
      {
	case 1:
	  toler=doc.ms.parseMeasurement(words[0],LENGTH).magnitude;
	  break;
	case 2:
	  e=doc.ms.parseMeasurement(words[0],LENGTH).magnitude;
	  n=doc.ms.parseMeasurement(words[1],LENGTH).magnitude;
	  points.push_back(xy(e,n));
	  npoints++;
	  break;
	case 3:
	  e=doc.ms.parseMeasurement(words[0],LENGTH).magnitude;
	  n=doc.ms.parseMeasurement(words[1],LENGTH).magnitude;
	  bear=parsearangle(words[2],DEGREE).ang;
	  bear=lastbear+foldangle(bear-lastbear);
	  lines.push_back(Circle(xy(e,n),bear));
	  lastbear=bear;
	  break;
	default:
	  if (infile.good())
	    cerr<<"Ignored line: "<<line<<endl;
      }
    } while (infile.good());
    cout<<npoints<<" points\n";
    infile.close();
  }
  if (toler>0)
    cout<<"Tolerance "<<doc.ms.formatMeasurementUnit(toler,LENGTH)<<endl;
  else if (npoints>=0)
    cout<<"Place tolerance in file on a line by itself\n";
  if (lines.size()<2 && npoints>=0)
    cout<<"Place at least two lines in file (order matters) e.g.\n"
    <<"314.159,271.828,N36°52'12\"E\n"
    <<"The initial curve proceeds N53°07'48\"W from/through/to this point.\n";
  if (toler>0 && lines.size()>1 && npoints>0)
  {
    startLine=lines.front();
    endLine=lines.back();
    lines.pop_back();
    lines.pop_front();
    cout<<"Fitting curve...\n";
    fitCurve=fitPolyarc(startLine,points,endLine,toler,lines,2);
    for (i=0;i<fitCurve.size();i++)
    {
      oneArc=fitCurve.getarc(i);
      cout<<ldecimal(oneArc.getstart().getx())<<','<<ldecimal(oneArc.getstart().gety())<<",end\n";
      cout<<ldecimal(oneArc.midpoint().getx())<<','<<ldecimal(oneArc.midpoint().gety())<<",mid\n";
    }
    cout<<ldecimal(oneArc.getend().getx())<<','<<ldecimal(oneArc.getend().gety())<<",end\n";
  }
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
  commands.push_back(command("bdiff",bdiff_i,"Bearing difference: bearing bearing"));
  commands.push_back(command("geoid",readgeoid_i,"Read geoid file: filename"));
  commands.push_back(command("read",readpoints,"Read coordinate file: filename format"));
  commands.push_back(command("write",writepoints,"Write coordinate file: filename format"));
  commands.push_back(command("save",save_i,"Write scene file: filename.bez"));
  commands.push_back(command("maketin",maketin_i,"Make triangulated irregular network"));
  commands.push_back(command("drawtin",drawtin_i,"Draw TIN: filename.ps"));
  commands.push_back(command("curvefit",curvefit_i,"Fit curve: filename.csv"));
  commands.push_back(command("raster",rasterdraw_i,"Draw raster topo: filename.ppm"));
  commands.push_back(command("contour",contourdraw_i,"Draw contour topo: interval filename.ps"));
  commands.push_back(command("factorll",scalefactorll_i,"Compute map scale factor from latitude and longitude"));
  commands.push_back(command("factorxy",scalefactorxy_i,"Compute map scale factor from grid coordinates"));
  commands.push_back(command("trin",trin_i,"Find what triangle a point is in: x,y"));
  commands.push_back(command("help",help,"List commands"));
  commands.push_back(command("exit",exit,"Exit the program"));
  doc.pl.resize(1);
  doc.ms.setFoot(INTERNATIONAL);
  doc.ms.setMetric();
  doc.ms.setDefaultUnit(LENGTH,0.552); // geometric mean of meter and foot
  doc.ms.setDefaultPrecision(LENGTH,1.746e-3); // g.m. of 1 mm and 0.01 ft
  doc.ms.setDefaultUnit(AREA,0.3048); // for acre/hectare, 6361.5
  doc.ms.setDefaultPrecision(AREA,0.1);
  readTmCoefficients();
  readAllProjections();
  cout<<"Bezitopo version "<<VERSION<<" © "<<COPY_YEAR<<" Pierre Abbat\n"
  <<"Distributed under LGPL v3 or later. This is free software with no warranty."<<endl;
  while (cont)
  {
    cout<<"? ";
    cout.flush();
    cmdline="";
    getline(cin,cmdline);
    chpos=cmdline.find_first_of(' '); // split the string at the first space
    if (chpos>cmdline.length())
      chpos=cmdline.length();
    cmdword=cmdline.substr(0,chpos);
    cmdargs=cmdline.substr(chpos);
    if (!cin.good() && !cmdword.length())
      cmdword="exit";
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
