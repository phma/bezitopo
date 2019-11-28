/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
/*                                                    */
/******************************************************/
/* Copyright 2015-2018 Pierre Abbat.
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
#include <ctime>
#include "config.h"
#include "geoid.h"
#include "sourcegeoid.h"
#include "refinegeoid.h"
#include "document.h"
#include "raster.h"
#include "hlattice.h"
#include "relprime.h"
#include "ps.h"
#include "manysum.h"
#include "ldecimal.h"
#include "latlong.h"
#include "halton.h"
#include "kml.h"
#include "smooth5.h"
#include "cmdopt.h"
using namespace std;

document doc;
vector<geoformat> formatlist;
int verbosity=1;
bool helporversion=false,commandError=false,inputKml=false,didConvert=false;
int qsz=4;
int latFineness=0,lonFineness=0;
double bolTolerance=0,bolSubdivision=0,bolSpacing=0;
int nInputFiles=0;
vector<string> infilebasenames,infilenames;
string outfilename;

vector<option> options(
  {
    {'h',"help","","Help using the program"},
    {'\0',"version","","Output version number"},
    {'v',"verbose","","Increase verbosity"},
    {'f',"format","e.g. ngs","Format of the geoid file"},
    {'o',"output","filename","Output geoid file"},
    {'c',"circle","lat long radius","Excerpt a region"},
    {'k',"kml","","Write KML outline of input files"},
    {'F',"fine","e.g. 0.017 or 0-01","Fineness of both latitude and longitude"},
    {'\0',"latfine","e.g. 0.017 or 0-01","Fineness of latitude"},
    {'\0',"lonfine","e.g. 0.017 or 0-01","Fineness of longitude"},
    {'t',"tolerance","distance","Tolerance of geoquads, typ. 1 mm"},
    {'s',"subdiv","distance","Subdivision limit of geoquads, typ. 1 km"},
    {'e',"endian","big/native/little","Output endianness (for ngs)"},
    {'q',"quadsample","n 4-16","Geoquad sampling fineness"},
    {'S',"spacing","distance","Geoquad search spacing, typ. 100 km"}
  });

vector<token> cmdline;
geoid outputgeoid;

void outhelp()
{
  int i,j;
  cout<<"Convertgeoid converts geoid files from one format to another and\n"
    <<"makes excerpts of them. Example:\n"
    <<"convertgeoid g2012bu0.bin -c 38N99W 150km -f gsf -o Macksville.gsf\n"
    <<"makes an excerpt containing a 150 km circle around Macksville, Kansas.\n";
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

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 589824/51409
 * 4: 65536/7225
 * 5: 589824/51409
 */

xy unfold(vball pnt)
{
  xy ret(-2,-2);
  switch (pnt.face)
  {
    case 1:
      ret=xy(pnt.x,pnt.y);
      break;
    case 2:
      ret=xy(2-pnt.y,pnt.x);
      break;
    case 3:
      ret=xy(pnt.y,2-pnt.x);
      break;
    case 4:
      ret=xy(-pnt.y,pnt.x-2);
      break;
    case 5:
      ret=xy(-2-pnt.y,pnt.x);
      break;
    case 6:
      ret=xy(4-pnt.x,-pnt.y);
      break;
  }
  return ret;
}

void plotcenter(PostScript &ps,geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotcenter(ps,*quad.sub[i]);
  else
    ps.dot(unfold(quad.vcenter()));
}

void plotcenters(PostScript &ps)
{
  ps.setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotcenter(ps,cube.faces[i]);
}

void plotinter(PostScript &ps,geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotinter(ps,*quad.sub[i]);
  else
  {
    ps.setcolor(0,0,1);
    for (i=0;i<quad.nums.size();i++)
      ps.dot(unfold(vball(quad.face,quad.nums[i])));
    ps.setcolor(1,0,0);
    for (i=0;i<quad.nans.size();i++)
      ps.dot(unfold(vball(quad.face,quad.nans[i])));
  }
}

void plotinters(PostScript &ps)
{
  ps.setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotinter(ps,cube.faces[i]);
}

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
  cout<<"c: "<<cube.undulation(lat,lon)<<endl;
}

void test360seam()
/* Tests for continuity of the derivative across the seam in a geoid file
 * that covers the whole earth.
 */
{
  int i,j,seamlong,bigpos,creasecount=0;
  double bigdiff;
  vector<double> elevs,diff2;
  for (i=0;i<geo.size();i++)
    if (geo[i].glat && geo[i].glat->wbd-geo[i].glat->ebd==DEG360)
      seamlong=geo[i].glat->wbd;
  for (i=-60*DEG1;i<=60*DEG1;i+=DEG1)
  {
    elevs.clear();
    diff2.clear();
    for (j=-561000;j<=561000;j+=33000) // apx -5'40" to 5'40" by 20"
      elevs.push_back(avgelev(Sphere.geoc(i,j+seamlong,0)));
    for (j=1;j<elevs.size()-1;j++)
      diff2.push_back(elevs[j-1]+elevs[j+1]-2*elevs[j]);
    for (bigdiff=bigpos=j=0;j<diff2.size();j++)
      if (fabs(diff2[j])>bigdiff)
      {
        bigpos=j;
        bigdiff=fabs(diff2[j]);
      }
    //for (j=0;j<diff2.size();j++)
    //  cout<<((bigpos==j)?'*':' ');
    //cout<<endl;
    if (bigpos==diff2.size()/2)
      creasecount++;
  }
  cout<<"creasecount "<<creasecount<<endl;
  if (creasecount>40)
    cout<<"The seam of the whole-earth geoid file is not smoothed properly."<<endl;
}

void initformat(string cmd,string ext,string desc,int readfunc(geoid&,string),void writefunc(geoid&,string))
{
  geoformat gf;
  gf.cmd=cmd;
  gf.ext=ext;
  gf.desc=desc;
  gf.readfunc=readfunc;
  gf.writefunc=writefunc;
  formatlist.push_back(gf);
}

int readgeoid(string filename)
/* Returns 0, 1, or 2 like the geoformat.readfunc functions.
 * If it returns 2, it pushes a geoid back onto geo
 * and the filename and format onto hdr.
 */
{
  int i,ret;
  size_t extpos;
  string basename;
  geoid gd;
  nInputFiles++;
  for (i=0,ret=1;i<formatlist.size() && ret==1;i++)
    ret=formatlist[i].readfunc(gd,filename);
  i--;
  if (ret==2)
  {
    geo.push_back(gd);
    outputgeoid.ghdr->namesFormats.push_back(filename);
    outputgeoid.ghdr->namesFormats.push_back(formatlist[i].cmd);
    cout<<"Read "<<filename<<" in format "<<formatlist[i].cmd<<endl;
    /*if (formatlist[i].cmd=="bol")
    {
      ofstream gdump("gd.dump");
      geo.back().cmap->dump(gdump);
    }*/
    extpos=filename.rfind("."+formatlist[i].ext);
    if (extpos+formatlist[i].ext.length()+1==filename.length())
      basename=filename.substr(0,extpos);
    else
      basename=filename;
    infilenames.push_back(filename);
    infilebasenames.push_back(basename);
  }
  if (ret==1)
    cout<<filename<<" does not appear to be a geoid file."<<endl;
  if (ret==0)
    cout<<"Could not read "<<filename<<endl;
  return ret;
}

void frontformat(string fmt)
{
  int i;
  for (i=formatlist.size()-1;i>0;i--)
    if (formatlist[i].cmd==fmt)
      swap(formatlist[i],formatlist[i-1]);
  if (fmt!=formatlist[0].cmd)
  {
    cout<<"Unrecognized format: "<<fmt<<endl;
    commandError=true;
  }
}

histogram errorspread(double tolerance)
{
  histogram ret(-1/65536.,1/65536.);
  halton hal;
  latlong ll;
  xyz loc;
  int i;
  double origelev,cvtelev;
  ret.addinterval(-tolerance/1.25,tolerance/1.25);
  ret.addinterval(-tolerance,tolerance);
  ret.addinterval(-tolerance*1.25,tolerance*1.25);
  for (i=0;i*ret.nbars()<16777216;i++)
  {
    ll=hal.onearth();
    loc=Sphere.geoc(ll,0);
    cvtelev=outputgeoid.elev(loc);
    origelev=avgelev(loc);
    if (isfinite(cvtelev) && isfinite(origelev))
    {
      ret<<(cvtelev-origelev);
    }
  }
  return ret;
}

histogram quadsizes()
{
  histogram ret;
  int h,i,j,sz;
  double least=34,most=-100;
  vector<double> areas;
  if (outputgeoid.cmap)
    areas=outputgeoid.cmap->areas();
  sz=areas.size();
  for (i=0;i<sz;i++)
  {
    if (least>log(areas[i]))
      least=log(areas[i]);
    if (most<log(areas[i]))
      most=log(areas[i]);
  }
  if (least<most)
    ret.clear(least,most);
  h=relprime(sz);
  for (i=j=0;i<sz;i++,j=(j+h)%sz)
  ret<<log(areas[j]);
  return ret;
}

int parseFineness(string str)
{
  int angle;
  angle=parseiangle(str,DEGREE);
  if (angle<SMOOTH5LIMIT)
    angle=SMOOTH5LIMIT;
  return nearestSmooth(rint((double)DEG180/angle));
}

void argpass2()
{
  int i,j,foundunit;
  string centerstr;
  latlong ll;
  double radius;
  smallcircle cir;
  for (i=0;i<cmdline.size();i++)
    switch (cmdline[i].optnum)
    {
      case 0:
	helporversion=true;
	outhelp();
	break;
      case 1:
	helporversion=true;
	cout<<"Convertgeoid, part of Bezitopo version "<<VERSION<<" © "<<COPY_YEAR<<" Pierre Abbat\n"
	<<"Distributed under GPL v3 or later. This is free software with no warranty."<<endl;
	break;
      case 2:
	++verbosity;
	cout<<"verbosity "<<verbosity<<endl;
	break;
      case 3:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	  frontformat(cmdline[i].nonopt);
	}
	else
	{
	  cout<<"-f / --format requires an argument, one of:"<<endl;
	  for (j=0;j<formatlist.size();j++)
	    cout<<formatlist[j].cmd<<'\t'<<formatlist[j].desc<<endl;
          commandError=true;
	}
	break;
      case 4:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          outfilename=cmdline[i].nonopt;
	  //cout<<"Geoid will be output to "<<cmdline[i].nonopt<<endl;
	}
	else
	{
	  cout<<"-o / --output requires an argument, a filename"<<endl;
          commandError=true;
	}
	break;
      case 5:
	centerstr="";
	radius=NAN;
	ll=parselatlong(centerstr,DEGREE); // sets it to NAN,NAN
	for (j=1;ll.valid()<2 && i+j<cmdline.size() && cmdline[i+j].optnum<0;j++)
	{
	  centerstr+=cmdline[i+j].nonopt+" ";
	  ll=parselatlong(centerstr,DEGREE);
	}
	i+=j;
	if (ll.valid()==2 && i<cmdline.size() && cmdline[i].optnum<0)
        {
          try
          {
            radius=doc.ms.parseMeasurement(cmdline[i++].nonopt,LENGTH).magnitude;
          }
          catch (...)
          {
          }
        }
	if (radius>0 && radius<=1e7 && ll.lat>=-M_PI/2 && ll.lat<=M_PI/2)
	{
	  cout<<"Excerpt will be centered on "<<radtodeg(ll.lat)<<','<<radtodeg(ll.lon)<<" with radius "<<radius<<" m"<<endl;
	  cir.center=Sphere.geoc(ll,0);
	  cir.setradius(radtobin(radius/Sphere.avgradius()));
	  excerptcircles.push_back(cir);
	}
	else
	{
	  cout<<"-c / --circle requires two arguments, a center (latitude/longitude) and a radius\n";
	  cout<<"radius is 10000 km max"<<endl;
          commandError=true;
	}
	i--;
	break;
      case 6:
        inputKml=true;
        break;
      case 7:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          latFineness=lonFineness=parseFineness(cmdline[i].nonopt);
	}
	else
	{
	  cerr<<"-F / --fine requires an argument, an angle"<<endl;
          commandError=true;
	}
	break;
      case 8:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          latFineness=parseFineness(cmdline[i].nonopt);
	}
	else
	{
	  cerr<<"--latfine requires an argument, an angle"<<endl;
          commandError=true;
	}
	break;
      case 9:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          lonFineness=parseFineness(cmdline[i].nonopt);
	}
	else
	{
	  cerr<<"--lonfine requires an argument, an angle"<<endl;
          commandError=true;
	}
	break;
      case 10:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            bolTolerance=doc.ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude;
            if (bolTolerance<sqrt(6)/65536) // about 37 µm
              bolTolerance=sqrt(6)/65536; // sqrt(6) because a geoquad has six components
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a distance"<<endl;
            commandError=true;
          }
	}
	else
	{
	  cerr<<"--tolerance requires an argument, a distance"<<endl;
          commandError=true;
	}
	break;
      case 11:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            bolSubdivision=doc.ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude;
            if (bolSubdivision<1)
              bolSubdivision=1;
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a distance"<<endl;
            commandError=true;
          }
	}
	else
	{
	  cerr<<"--subdiv requires an argument, a distance"<<endl;
          commandError=true;
	}
	break;
      case 12:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          if (cmdline[i].nonopt=="big")
            setEndian(ENDIAN_BIG);
          else if (cmdline[i].nonopt=="native")
            setEndian(ENDIAN_NATIVE);
          else if (cmdline[i].nonopt=="little")
            setEndian(ENDIAN_LITTLE);
          else
          {
            cerr<<"argument to -e/--endian should be  \"big\" or \"native\" or \"little\""<<endl;
            commandError=true;
          }
	}
	else
	{
	  cerr<<"-e / --endian requires an argument, an endianness"<<endl;
          commandError=true;
	}
	break;
      case 13:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          qsz=stod(cmdline[i].nonopt);
	}
	else
	{
	  cerr<<"-q / --quadsample requires an argument, an integer from 4 to 16"<<endl;
          commandError=true;
	}
	break;
      case 14:
        if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          try
          {
            bolSpacing=doc.ms.parseMeasurement(cmdline[i].nonopt,LENGTH).magnitude;
            if (bolSpacing<1) // limit in interroquad
              bolSpacing=1;
          }
          catch (...)
          {
            cerr<<"Could not parse \""<<cmdline[i].nonopt<<"\" as a distance"<<endl;
            commandError=true;
          }
	}
	else
	{
	  cerr<<"--spacing requires an argument, a distance"<<endl;
          commandError=true;
	}
	break;
      default:
	if (!helporversion)
	  readgeoid(cmdline[i].nonopt);
    }
}

/* Command line syntax:
 * -f format		Puts format first on the list of formats to try.
 * -o file		Sets the output filename. The file is written after
 * 			all input files are read.
 * -c lat long radius	Excerpts a circle from the geoid file.
 * Outputting the KML file is automatic; there is no option for it.
 * Arguments not tagged by an option are input files.
 * 
 * Example:
 * convertgeoid -f ngs g2012bu0.bin -c 38N99W 150km -f gsf -o Macksville.gsf
 * Reads the Lower 48 file in NGS format, outputs an excerpt called Macksville.gsf
 * containing a circle of radius 150 km centered at 38N99W in GSF format,
 * and outputs the boundary to file Macksville.gsf.kml .
 */

bool oneBoldatni()
{
  if (nInputFiles==1)
    return geo[0].ghdr!=nullptr;
  else
    return false;
}

int main(int argc, char *argv[])
{
  ofstream ofile;
  int i;
  vball v;
  bool conversionError=false;
  PostScript ps;
  histogram errorHist,areaHist;
  histobar intervalBar;
  double percentage;
  vector<cylinterval> excerptintervals,inputbounds;
  array<int,6> undrange;
  array<int,5> undhisto;
  cylinterval latticebound;
  int fineness=10800;
  setEndian(ENDIAN_NATIVE);
  initformat("bol","bol","Bezitopo Boldatni",readboldatni,writeboldatni);
  if (strcmp(FUZZ,"boldatni"))
  {
    initformat("ngs","bin","US National Geodetic Survey binary",readusngsbin,writeusngsbin);
    initformat("gsf","gsf","Carlson Geoid Separation File",readcarlsongsf,writecarlsongsf);
    initformat("ngatxt","grd","US National Geospatial-Intelligence Agency text",readusngatxt,writeusngatxt);
    initformat("ngabin","","US National Geospatial-Intelligence Agency binary",readusngabin,nullptr);
  }
  outputgeoid.cmap=new cubemap;
  outputgeoid.ghdr=new geoheader;
  outputgeoid.glat=new geolattice;
  outputgeoid.cmap->scale=1/65536.;
  outputgeoid.ghdr->logScale=-16;
  outputgeoid.ghdr->planet=BOL_EARTH;
  outputgeoid.ghdr->dataType=BOL_UNDULATION;
  outputgeoid.ghdr->encoding=BOL_VARLENGTH;
  outputgeoid.ghdr->ncomponents=1;
  outputgeoid.ghdr->tolerance=0.003;
  outputgeoid.ghdr->sublimit=1000;
  outputgeoid.ghdr->spacing=1e5;
  correctionHist.setdiscrete(1);
  argpass1(argc,argv);
  argpass2();
  if (qsz<4)
    qsz=4;
  if (qsz>16)
    qsz=16;
  for (i=0;i<excerptcircles.size();i++)
    excerptintervals.push_back(excerptcircles[i].boundrect());
  for (i=0;i<geo.size();i++)
  {
    if (formatlist[0].cmd!="bol")
      inputbounds.push_back(geo[i].boundrect());
    if (latFineness==0)
      latFineness=geo[i].getLatFineness();
    if (lonFineness==0)
      lonFineness=geo[i].getLonFineness();
    if (bolTolerance==0 && geo[i].ghdr)
      bolTolerance=geo[i].ghdr->tolerance;
    if (bolSubdivision==0 && geo[i].ghdr)
      bolSubdivision=geo[i].ghdr->sublimit;
    if (bolSpacing==0 && geo[i].ghdr)
      bolSpacing=geo[i].ghdr->spacing;
  }
  if (excerptintervals.size())
    excerptinterval=combine(excerptintervals);
  else
    excerptinterval.setfull();
  excerptinterval.round(latFineness,lonFineness);
  latticebound=intersect(excerptinterval,combine(inputbounds));
  //cout<<"latticebound "<<formatlatlong(latlong(latticebound.sbd,latticebound.wbd),DEGREE+SEXAG2);
  //cout<<' '<<formatlatlong(latlong(latticebound.nbd,latticebound.ebd),DEGREE+SEXAG2)<<endl;
  if (!helporversion && !commandError && (geo.size() || !nInputFiles))
  {
    if (inputKml)
      for (i=0;i<geo.size();i++)
        outKml(gbounds(geo[i]),infilenames[i]+".kml");
    if (!outfilename.length())
    {
      if (infilebasenames.size()==1)
      {
        outfilename=infilebasenames[0]+"."+formatlist[0].ext;
        if (outfilename==infilenames[0])
        {
          cout<<"Not overwriting "<<outfilename<<endl;
          outfilename="";
        }
      }
      else
        cout<<"Please specify a filename with -o"<<endl;
    }
    if (outfilename.length())
      if (formatlist[0].cmd=="bol")
      {
        if (bolTolerance<=0)
          bolTolerance=0.001;
        if (bolSubdivision<=0)
          bolSubdivision=1000;
        if (bolSpacing<=0)
          bolSpacing=1e5;
        outputgeoid.ghdr->tolerance=bolTolerance;
        outputgeoid.ghdr->sublimit=bolSubdivision;
        outputgeoid.ghdr->spacing=bolSpacing;
	if (oneBoldatni())
	{
	  outputgeoid.ghdr->excerpted=true;
	  outputgeoid.ghdr->origHash=geo[0].ghdr->origHash;
	}
	else
	  outputgeoid.ghdr->excerpted=false;
        for (i=0;i<6;i++)
        {
          //cout<<"Face "<<i+1;
          //cout.flush();
          interroquad(outputgeoid.cmap->faces[i],outputgeoid.ghdr->spacing);
          /*if (cube.faces[i].isfull()>=0)
            cout<<" has data"<<endl;
          else
            cout<<" is empty"<<endl;*/
          refine(outputgeoid.cmap->faces[i],outputgeoid.cmap->scale,outputgeoid.ghdr->tolerance,outputgeoid.ghdr->sublimit,outputgeoid.ghdr->spacing,qsz,allBoldatni());
        }
        outProgress();
        cout<<endl;
        undrange=outputgeoid.cmap->undrange();
        cout<<"Undulation range: constant "<<undrange[0]<<'-'<<undrange[1];
        cout<<" linear "<<undrange[2]<<'-'<<undrange[3];
        cout<<" quadratic "<<undrange[4]<<'-'<<undrange[5]<<endl;
        undhisto=outputgeoid.cmap->undhisto();
        cout<<"1 byte "<<undhisto[0]<<"; 2 bytes "<<undhisto[1]<<"; 3 bytes "<<undhisto[2]<<"; 4 bytes "<<undhisto[3]<<endl;
        if (dataArea.total()>510e12)
          test360seam();
        didConvert=dataArea.total()>0;
        delete outputgeoid.glat;
        outputgeoid.glat=nullptr;
      }
      else
      {
        if (latFineness && lonFineness)
        {
          cout<<"Latitude fineness "<<latFineness<<" ("<<radtoangle(M_PI/latFineness,ARCSECOND+DEC2+FIXLARGER)<<")\n";
          cout<<"Longitude fineness "<<lonFineness<<" ("<<radtoangle(M_PI/lonFineness,ARCSECOND+DEC2+FIXLARGER)<<")\n";
          outputgeoid.glat->setbound(latticebound);
          outputgeoid.glat->setfineness(latFineness,lonFineness);
          outputgeoid.glat->setundula();
          outputgeoid.glat->setslopes();
          didConvert=outputgeoid.glat->boundrect().area()>0;
        }
        else
        {
          delete outputgeoid.glat;
          outputgeoid.glat=nullptr;
          cerr<<"No geolattice files read. Please specify -F when converting boldatni\nto any geolattice format."<<endl;
          conversionError=true;
        }
        delete outputgeoid.ghdr;
        delete outputgeoid.cmap;
        outputgeoid.ghdr=nullptr;
        outputgeoid.cmap=nullptr;
      }
    if (conversionError)
    outfilename="";
    if (outfilename.length() && !conversionError)
      if (formatlist[0].writefunc)
      {
        cout<<"Writing "<<outfilename<<endl;
        formatlist[0].writefunc(outputgeoid,outfilename);
        outKml(gbounds(outputgeoid),outfilename+".kml");
      }
      else
        cerr<<"Can't write in format "<<formatlist[0].cmd<<"; it is a whole-earth-only format."<<endl;
    //drawglobecube(1024,62,-7,&outputgeoid,0,"geoid.ppm");
    if (didConvert && !conversionError)
    {
      cout<<"avgelev called "<<avgelev_interrocount<<" times from interroquad, "<<avgelev_refinecount<<" times from refine"<<endl;
      cout<<"Computing error histogram"<<endl;
      errorHist=errorspread(bolTolerance);
      areaHist=quadsizes();
    }
    if (outfilename.length() && !conversionError)
    {
      if (errorHist.gettotal() || areaHist.gettotal())
      {
        ps.open(outfilename+".ps");
        ps.setpaper(papersizes["A4 portrait"],1);
        ps.prolog();
        if (errorHist.gettotal())
        {
	  for (i=0;i<3;i++)
	  {
	    intervalBar=errorHist.getinterval(i);
	    percentage=intervalBar.count*1e2/errorHist.gettotal();
	    cout<<ldecimal(percentage,percentage*(100-percentage)/2e3)<<"% of separations are within "<<intervalBar.end<<endl;
	  }
          ps.startpage();
          errorHist.plot(ps,HISTO_LINEAR);
          ps.endpage();
        }
        if (areaHist.gettotal())
        {
          ps.startpage();
          areaHist.plot(ps,HISTO_LOG);
          ps.endpage();
        }
        ps.close();
      }
      else
        cout<<"Error histogram empty, not plotting"<<endl;
    }
  }
  if (nInputFiles && !geo.size()) // Specifying no input file is valid.
  {
    cout<<"Input files could not be read or are not geoid files."<<endl;
    return 2;
  }
  else if (commandError)
  {
    cout<<"Run \"convertgeoid --help\" for help."<<endl;
    return 1;
  }
  else return 0;
}
