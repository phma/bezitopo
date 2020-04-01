/******************************************************/
/*                                                    */
/* clotilde.cpp - tables of approximations to spirals */
/*                                                    */
/******************************************************/
/* Copyright 2018-2020 Pierre Abbat.
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
/* This program is named Clotilde for similarity to "clotoide", the Spanish
 * word for the Euler spiral.
 */
#include <iostream>
#include "manyarc.h"
#include "vball.h"
#include "cmdopt.h"
#include "config.h"
#include "ps.h"
using namespace std;

int verbosity=1;
bool helporversion=false,commandError=false;
double arcLength=NAN,chordLength=NAN;
vector<double> curvature;
vector<int> lengthUnits,angleUnits;

vector<option> options(
  {
    {'h',"help","","Help using the program"},
    {'\0',"version","","Output version number"},
    {'l',"length","length","Arc length"},
    {'C',"chordlength","length","Chord length"},
    {'c',"curvature","cur cur","Start and end curvatures"},
    {'r',"radius","length length","Start and end radii"},
    {'u',"unit","m/ft/deg/dms","Length or angle unit"},
    {'\0',"logo","","Draw logo"}
  });

vector<token> cmdline;

void outhelp()
{
  int i,j;
  cout<<"Clotilde outputs approximations to spiralarcs. Example:\n"
    <<"clotilde -u m -l 200 -r inf 900\n"
    <<"approximates a 200-meter-long spiral starting straight and ending on 900 m radius.\n"
    <<"clotilde -u ft -l 500 -c 0 7\n"
    <<"approximates a 500-foot-long spiral starting straight and ending on a 7° curve.\n"
    <<"clotilde -u usft -l 500 -c 0 7 -u m\n"
    <<"approximates a 500-USfoot-long spiral, outputting the arcs in meters.\n"
    <<"When using feet, curvature is expressed as angle of 100 ft arc,\n"
    <<"and clothance is expressed as change in 100 ft of angle of 100 ft arc.\n";
  for (i=0;i<options.size();i++)
  {
    cout<<(options[i].shopt?options[i].shopt:' ')<<' ';
    cout<<options[i].lopt;
    for (j=options[i].lopt.length();j<14;j++)
      cout<<' ';
    cout<<options[i].args;
    for (j=options[i].args.length();j<20;j++)
      cout<<' ';
    cout<<options[i].desc<<endl;
  }
}

void drawLogo()
{
  PostScript ps;
  int i;
  vector<spiralarc> strokes;
  polyarc approx;
  BoundRect br;
  ps.open("clotilde.ps");
  ps.setpaper(papersizes["A4 landscape"],0);
  ps.prolog();
  strokes.push_back(spiralarc(xyz(0,0,0),xyz(10,0,0))); // lines of writing
  strokes.push_back(spiralarc(xyz(0,1,0),xyz(10,1,0)));
  strokes.push_back(spiralarc(xyz(1,0.72,0),xyz(1,0.22,0)));
  strokes.back().setdelta(degtobin(300),degtobin(-10));
  strokes.push_back(spiralarc(xyz(1.3,2,0),xyz(2,0,0)));
  strokes.back().setdelta(degtobin(90),degtobin(50));
  strokes.push_back(spiralarc(xyz(2.5,0,0),xyz(2.5,1,0)));
  strokes.back().setdelta(degtobin(180),degtobin(0));
  strokes.push_back(spiralarc(xyz(2.5,1,0),xyz(2.5,0,0)));
  strokes.back().setdelta(degtobin(180),degtobin(0));
  strokes.push_back(spiralarc(xyz(3.3,1.5,0),xyz(3.825,0,0)));
  strokes.back().setdelta(degtobin(90),degtobin(50));
  strokes.push_back(spiralarc(xyz(3.1,1,0),xyz(3.5,1,0)));
  strokes.back().setdelta(degtobin(0),degtobin(30));
  for (i=0;i<strokes.size();i++)
    br.include(&strokes[i]);
  ps.startpage();
  ps.setscale(br);
  ps.setcolor(0.7,0.7,1);
  ps.widen(5);
  for (i=0;i<strokes.size();i++)
  {
    approx=manyArc(strokes[i],3);
    ps.spline(approx.approx3d(0.01));
  }
  ps.setcolor(0,0,0);
  ps.widen(1/5.);
  for (i=0;i<strokes.size();i++)
  {
    cout<<i<<" Bearing "<<bintodeg(strokes[i].startbearing())<<' '<<bintodeg(strokes[i].endbearing());
    cout<<" Curvature "<<strokes[i].curvature(0)<<' '<<strokes[i].curvature(strokes[i].length())<<endl;
    ps.spline(strokes[i].approx3d(0.01));
  }
  ps.close();
}

void startHtml(spiralarc s,Measure ms)
{
  cout<<"<html><head><title>Approximation ";
  cout<<ms.formatMeasurementUnit(s.length(),LENGTH,0,0.522)<<"</title></head><body>\n";
}

void endHtml()
{
  cout<<"</body></html>\n";
}

void outSpiral(spiralarc s,Measure ms)
{
  double startCur=s.curvature(0),endCur=s.curvature(s.length());
  double startTan=s.tangentLength(START),endTan=s.tangentLength(END);
  if (fabs(startCur)<1/EARTHRAD)
    startCur=0;
  if (fabs(endCur)<1/EARTHRAD)
    endCur=0;
  cout<<"<table border><tr><td><h1>Arc length: "<<ms.formatMeasurementUnit(s.length(),LENGTH)<<"</h1></td>\n";
  cout<<"<td><h1>Chord length: "<<ms.formatMeasurementUnit(s.chordlength(),LENGTH)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Start curvature: "<<formatCurvature(startCur,ms)<<"</h1></td>\n";
  cout<<"<td><h1>End curvature: "<<formatCurvature(endCur,ms)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Start radius: "<<ms.formatMeasurementUnit(1/startCur,LENGTH)<<"</h1></td>\n";
  cout<<"<td><h1>End radius: "<<ms.formatMeasurementUnit(1/endCur,LENGTH)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Start tangent: "<<ms.formatMeasurementUnit(startTan,LENGTH)<<"</h1></td>\n";
  cout<<"<td><h1>End tangent: "<<ms.formatMeasurementUnit(endTan,LENGTH)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Clothance: "<<formatClothance(s.clothance(),ms)<<"</h1></td>\n";
  cout<<"<td><h1>Delta: "<<ms.formatMeasurementUnit(s.getdelta(),ANGLE_B)<<"</h1></td></tr></table>\n";
}

void outArc(arc oneArc,Measure ms)
{
  double relprec=abs(oneArc.getdelta());
  if (relprec==0)
    relprec=1;
  cout<<"<tr><td colspan=4>"<<ms.formatMeasurementUnit(oneArc.length(),LENGTH)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.chordlength(),LENGTH)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.getdelta(),ANGLE_B)<<"</td>";
  cout<<"<td colspan=4>"<<formatCurvature(oneArc.curvature(0),ms,oneArc.curvature(0)/relprec)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.radius(0),LENGTH,0,oneArc.radius(0)/relprec)<<"</td></tr>\n";
}

void outPoint(xy pnt,spiralarc s,Measure ms)
{
  int sb=s.startbearing(),eb=s.endbearing();
  xy sp=s.getstart(),ep=s.getend();
  cout<<"<tr><td colspan=5>"<<ms.formatMeasurementUnit((sp==pnt)?0:(dir(sp,pnt)-sb),ANGLE_B)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit(dist(sp,pnt),LENGTH)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit((pnt==ep)?0:(dir(pnt,ep)-eb),ANGLE_B)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit(dist(pnt,ep),LENGTH)<<"</td></tr>\n";
}

void outApprox(polyarc approx,spiralarc s,Measure ms)
{
  int i;
  arc oneArc;
  double err=maxError(approx,s);
  cout<<"<table border><tr><th colspan=20>"<<approx.size()<<" arcs, error "
    <<ms.formatMeasurementUnit(err,LENGTH,0,err/32)<<"</th></tr>\n";
  for (i=0;i<approx.size();i++)
  {
    oneArc=approx.getarc(i);
    outPoint(oneArc.getstart(),s,ms);
    outArc(oneArc,ms);
  }
  outPoint(oneArc.getend(),s,ms);
  cout<<"</table>\n";
}

void argpass2()
/* Pass 2 does not parse lengths or curvatures, since the units may be
 * specified after the lengths and curvatures, or both input and output
 * units may be specified before the lengths.
 */
{
  int i,j;
  for (i=0;i<cmdline.size();i++)
    switch (cmdline[i].optnum)
    {
      case 0:
	helporversion=true;
	outhelp();
	break;
      case 1:
	helporversion=true;
	cout<<"Clotilde, part of Bezitopo version "<<VERSION<<" © "<<COPY_YEAR<<" Pierre Abbat\n"
	<<"Distributed under LGPL v3 or later. This is free software with no warranty."<<endl;
	break;
      case 2: // arc length
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	}
	break;
      case 3: // chord length
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	}
	break;
      case 4: // curvature
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	  if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	    i++;
	}
	break;
      case 5: // radius
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	  if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	    i++;
	}
	break;
      case 6: // unit
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	  if (cmdline[i].nonopt=="m")
	    lengthUnits.push_back(255);
	  else if (cmdline[i].nonopt=="ft")
	    lengthUnits.push_back(INTERNATIONAL);
	  else if (cmdline[i].nonopt=="usft")
	    lengthUnits.push_back(USSURVEY);
	  else if (cmdline[i].nonopt=="inft")
	    lengthUnits.push_back(INSURVEY);
	  else if (cmdline[i].nonopt=="deg")
	    angleUnits.push_back(9000);
	  else if (cmdline[i].nonopt=="dms")
	    angleUnits.push_back(5400);
	  else if (cmdline[i].nonopt=="gon")
	    angleUnits.push_back(10000);
	  else
	  {
	    commandError=true;
	    cerr<<"Unrecognized unit "<<cmdline[i].nonopt<<"; should be m, ft, usft, inft, deg, dms, or gon.\n";
	  }
	}
	else
	{
	  cerr<<"--unit requires an argument, one of m, ft, usft, inft, deg, dms, and gon.\n";
	  commandError=true;
	}
	break;
      case 7: // logo
	helporversion=true;
	drawLogo();
	break;
      default:
	;
    }
}

void argpass3(Measure ms)
{
  int i,j;
  for (i=0;i<cmdline.size();i++)
    switch (cmdline[i].optnum)
    {
      case 0:
	helporversion=true;
	break;
      case 1:
	helporversion=true;
	break;
      case 2: // arc length
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            arcLength=ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude;
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a length"<<endl;
            commandError=true;
          }
	}
	break;
      case 3: // chord length
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            chordLength=ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude;
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a length"<<endl;
            commandError=true;
          }
	}
	break;
      case 4: // curvature
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            curvature.push_back(parseCurvature(cmdline[i].nonopt,ms));
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a curvature"<<endl;
            commandError=true;
          }
	  if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	  {
	    i++;
	    try
	    {
	      curvature.push_back(parseCurvature(cmdline[i].nonopt,ms));
	    }
	    catch (...)
	    {
	      cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a curvature"<<endl;
	      commandError=true;
	    }
	  }
	}
	break;
      case 5: // radius
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            curvature.push_back(1/ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude);
          }
          catch (...)
          {
	    if (cmdline[i].nonopt=="inf")
	      curvature.push_back(0);
	    else
	    {
	      cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a length"<<endl;
	      commandError=true;
	    }
          }
	  if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	  {
	    i++;
	    try
	    {
	      curvature.push_back(1/ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude);
	    }
	    catch (...)
	    {
	      cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a length"<<endl;
	      commandError=true;
	    }
	  }
	}
	break;
      case 6:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	}
	break;
      default:
	;
    }
}

void setUnits(Measure &ms,int unit)
{
  if (unit>256)
  {
    ms.clearUnits(ANGLE);
    ms.clearUnits(ANGLE_B);
  }
  switch (unit)
  {
    case 255:
      ms.setMetric();
      break;
    case 10000:
      ms.addUnit(GON);
      ms.addUnit(GON_B);
      break;
    case 9000:
      ms.addUnit(DEGREE);
      ms.addUnit(DEGREE_B);
      break;
    case 5400:
      ms.addUnit(ARCSECOND+DECYMAL+FIXLARGER);
      ms.addUnit(ARCSECOND_B+DECYMAL+FIXLARGER);
      break;
    default:
      ms.setCustomary();
      ms.setFoot(unit);
  }
}

/* Ways to specify the spiralarc to be approximated:
 * • Start radius, end radius, arc length
 * • Start curvature, end curvature, arc length
 * • Start radius, end radius, chord length
 * • Start curvature, end curvature, chord length
 * Curvature may be specified in diopters or degrees; if in degrees, the length
 * is assumed to be 100 unless otherwise specified.
 */
int main(int argc, char *argv[])
{
  int i;
  spiralarc s;
  polyarc approx;
  Measure ms;
  ms.setMetric();
  ms.setDefaultUnit(LENGTH,0.552);
  ms.setDefaultPrecision(LENGTH,2e-6);
  ms.setDefaultUnit(CURVATURE,0.001);
  ms.setDefaultPrecision(CURVATURE,2e-9);
  ms.setDefaultUnit(CLOTHANCE,1e-6);
  ms.setDefaultPrecision(CLOTHANCE,2e-12);
  ms.setDefaultPrecision(ANGLE_B,1);
  ms.setDefaultPrecision(ANGLE,bintorad(1));
  argpass1(argc,argv);
  argpass2();
  if (angleUnits.size()>2 || lengthUnits.size()>2)
  {
    cerr<<"Too many units\n";
    commandError=true;
  }
  if (lengthUnits.size())
    setUnits(ms,lengthUnits[0]);
  if (angleUnits.size())
    setUnits(ms,angleUnits[0]);
  else
  {
    ms.addUnit(ARCSECOND+DECYMAL+FIXLARGER);
    ms.addUnit(ARCSECOND_B+DECYMAL+FIXLARGER);
  }
  if (!commandError && !helporversion)
    argpass3(ms);
  if (!commandError && !helporversion)
  {
    if (isfinite(arcLength) == isfinite(chordLength))
    {
      commandError=true;
      cerr<<"Please specify one of -l and -C, not both\n";
    }
    if (curvature.size()!=2)
    {
      commandError=true;
      cerr<<"Please specify two radii or curvatures\n";
    }
  }
  if (!commandError && !helporversion)
  {
    if (isfinite(arcLength))
      s=spiralarc(xyz(0,0,0),0,curvature[0],curvature[1],arcLength,0);
    else
      s=spiralarc(xyz(0,0,0),curvature[0],curvature[1],xyz(chordLength,0,0));
    startHtml(s,ms);
    outSpiral(s,ms);
    if (lengthUnits.size()>1)
      setUnits(ms,lengthUnits[1]);
    if (angleUnits.size()>1)
      setUnits(ms,angleUnits[1]);
    if (lengthUnits.size()>1 || angleUnits.size()>1)
      outSpiral(s,ms);
    i=2;
    if (!s.valid())
    {
      cerr<<"Could not make a spiralarc with the given dimensions\n";
      cout<<"<h2>Invalid spiral</h2>\n";
    }
    else
      do
      {
	approx=manyArc(s,i);
	outApprox(approx,s,ms);
	i++;
      } while (maxError(approx,s)>0.01);
    endHtml();
  }
  if (commandError)
    return 2;
  else if (!s.valid())
    return 1;
  else
    return 0;
}
