/******************************************************/
/*                                                    */
/* raster.cpp - raster image output                   */
/*                                                    */
/******************************************************/
/* Copyright 2013,2015,2016 Pierre Abbat.
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
#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include "raster.h"

using namespace std;
fstream rfile;

void ropen(string fname)
{
  if (fname=="")
    fname="/dev/stdout";
  rfile.open(fname.c_str(),ios_base::out|ios_base::binary);
}

void rclose()
{
  rfile.close();
}

string color(double elev)
{
  double r,g,b;
  string str("rgb");
  if (isfinite(elev))
  {
    g=elev-floor(elev);
    elev*=10;
    b=elev-floor(elev);
    elev*=10;
    r=elev-floor(elev);
    b=1-b;
  }
  else
    r=g=b=1;
  r=trunc(r*256);
  g=trunc(g*256);
  b=trunc(b*256);
  if (r>255)
    r=255;
  if (g>255)
    g=255;
  if (b>255)
    b=255;
  str[0]=r;
  str[1]=g;
  str[2]=b;
  return str;
}

string gcolor(double elev)
{
  double r,g,b;
  string str("rgb");
  if (isfinite(elev))
  {
    r=(sin(elev*M_PI*5)+1)/2;
    if (elev>1)
      elev=1;
    if (elev<-1)
      elev=-1;
    g=sin((elev+1)*M_PI/4);
    b=sin((1-elev)*M_PI/4);
  }
  else
    r=g=b=1;
  r=rint(r*256);
  g=rint(g*256);
  b=rint(b*256);
  if (r>255)
    r=255;
  if (g>255)
    g=255;
  if (b>255)
    b=255;
  str[0]=r;
  str[1]=g;
  str[2]=b;
  return str;
}

void ppmheader(int width,int height)
{
  rfile<<"P6\n"<<width<<" "<<height<<endl<<255<<endl;
}

void rasterdraw(pointlist &pts,xy center,double width,double height,
	    double scale,int imagetype,double zscale,string filename)
/* scale is in pixels per meter. imagetype is currently ignored.
 */
{
  int i,j,k;
  string pixel;
  int pwidth,pheight;
  xy pnt;
  double z;
  //hvec bend,dir,center,lastcenter,jump;
  char letter;
  ropen(filename);
  if (scale<=0)
    throw(range_error("rasterdraw: scale must be positive"));
  if (width<0 || height<0)
    throw(range_error("rasterdraw: paper size must be nonnegative"));
  pwidth=ceil(width*scale);
  pheight=ceil(height*scale);
  ppmheader(pwidth,pheight);
  for (i=0;i<pheight;i++)
    for (j=0;j<pwidth;j++)
    {
      pnt=xy(j-pwidth/2.,pheight/2.-i);
      z=pts.elevation(center+pnt/scale);
      pixel=color(z/zscale);
      rfile<<pixel;
    }
  rclose();
}

vball foldcube(int panel,double x,double y)
{
  vball v;
  switch (panel)
  {
    case 0:
    case 2:
    case 3:
    case 8:
    case 10:
    case 11:
      v.face=0;
      break;
    case 1:
      v.face=3;
      v.x=-y;
      v.y=x;
      break;
    case 4:
      v.face=5;
      v.x=y;
      v.y=-x;
      break;
    case 5:
      v.face=1;
      v.x=x;
      v.y=y;
      break;
    case 6:
      v.face=2;
      v.x=y;
      v.y=-x;
      break;
    case 7:
      v.face=6;
      v.x=-x;
      v.y=-y;
      break;
    case 9:
      v.face=4;
      v.x=y;
      v.y=-x;
      break;
  }
  return v;
}

#ifdef NUMSGEOID
void drawglobecube(int side,double zscale,double zmid,geoid *source,int imagetype,string filename)
/* side is in pixels. Draws 4*side wide by 3*side high. imagetype is currently ignored.
 * source is nullptr for xyz color (zscale is ignored), else its geoquads
 * are plotted.
 */
{
  int i,j,panel;
  string pixel;
  double x,y,z,max,min;
  xyz sphloc;
  vball v;
  //hvec bend,dir,center,lastcenter,jump;
  char letter;
  max=-INFINITY;
  min=INFINITY;
  ropen(filename);
  ppmheader(4*side,3*side);
  for (i=0;i<3*side;i++)
  {
    y=1-(((i%side)+0.5)/side)*2;
    for (j=0;j<4*side;j++)
    {
      x=(((j%side)+0.5)/side)*2-1;
      panel=(i/side)*4+(j/side);
      v=foldcube(panel,x,y);
      if (v.face)
      {
	sphloc=decodedir(v);
	if (source)
	{
	  z=source->elev(sphloc);
	  /*if (source==1)
	    z=avgelev(sphloc);
	  if (source==2)
	    z=outputgeoid.cmap->undulation(sphloc);*/
	  if (z<min)
	    min=z;
	  if (z>max)
	    max=z;
	  pixel=gcolor((z-zmid)/zscale);
	}
	else
	{
	  pixel="rgb";
	  pixel[0]=rint((sphloc.getx()+EARTHRAD)*255/(2*EARTHRAD));
	  pixel[1]=rint((sphloc.gety()+EARTHRAD)*255/(2*EARTHRAD));
	  pixel[2]=rint((sphloc.getz()+EARTHRAD)*255/(2*EARTHRAD));
	}
      }
      else
	pixel="@@@";
      rfile<<pixel;
    }
  }
  rclose();
  cout<<"drawglobecube: max "<<max<<" min "<<min<<endl;
}
#endif

void drawglobemicro(int side,xy center,double size,int source,int imagetype,string filename)
{
  int i,j,panel;
  string pixel;
  double x,y,z,max,min,zmid,zscale;
  xyz sphloc;
  vball v;
  char letter;
  max=-INFINITY;
  min=INFINITY;
  ropen(filename);
  ppmheader(side,side);
  for (i=0;i<16;i++)
  {
    y=(((i+0.5)/16)*2-1)*size+center.gety();
    for (j=0;j<16;j++)
    {
      x=(((j+0.5)/16)*2-1)*size+center.getx();
      panel=floor(y)*4+floor(x);
      v=foldcube(panel,(x-floor(x))*2-1,(floor(y)-y)*2+1);
      if (v.face)
      {
	sphloc=decodedir(v);
	if (source)
	{
	  z=0;
#ifdef CONVERTGEOID
	  if (source==1)
	    z=avgelev(sphloc);
	  if (source==2)
	    z=cube.undulation(sphloc);
#endif
	  if (z<min)
	    min=z;
	  if (z>max)
	    max=z;
	}
      }
    }
  }
  zmid=(min+max)/2;
  zscale=(max-min)/2;
  for (i=0;i<side;i++)
  {
    y=(((i+0.5)/side)*2-1)*size+center.gety();
    for (j=0;j<side;j++)
    {
      x=(((j+0.5)/side)*2-1)*size+center.getx();
      panel=floor(y)*4+floor(x);
      v=foldcube(panel,(x-floor(x))*2-1,(floor(y)-y)*2+1);
      if (v.face)
      {
	sphloc=decodedir(v);
	if (source)
	{
	  z=0;
#ifdef CONVERTGEOID
	  if (source==1)
	    z=avgelev(sphloc);
	  if (source==2)
	    z=cube.undulation(sphloc);
#endif
	  pixel=gcolor((z-zmid)/zscale);
	}
	else
	{
	  pixel="rgb";
	  pixel[0]=rint((sphloc.getx()+EARTHRAD)*255/(2*EARTHRAD));
	  pixel[1]=rint((sphloc.gety()+EARTHRAD)*255/(2*EARTHRAD));
	  pixel[2]=rint((sphloc.getz()+EARTHRAD)*255/(2*EARTHRAD));
	}
      }
      else
	pixel="@@@";
      rfile<<pixel;
    }
  }
  rclose();
  cout<<"drawglobemicro: max "<<max<<" min "<<min<<endl;
}
