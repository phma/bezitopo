#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include "raster.h"
#include "geoid.h"

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

void drawglobecube(int side,double zscale,string filename)
/* side is in pixels. Draws 4*side wide by 3*side high. imagetype is currently ignored.
 */
{
  int i,j,panel;
  string pixel;
  double x,y,z;
  vball v;
  //hvec bend,dir,center,lastcenter,jump;
  char letter;
  ropen(filename);
  ppmheader(4*side,3*side);
  for (i=0;i<3*side;i++)
  {
    y=((i%side)+0.5)/side;
    for (j=0;j<4*side;j++)
    {
      x=((j%side)+0.5)/side;
      panel=(i/side)*4+(j/side);
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
	  v.x=x;
	  v.y=y;
	  break;
	case 4:
	  v.face=5;
	  v.x=x;
	  v.y=y;
	  break;
	case 5:
	  v.face=1;
	  v.x=x;
	  v.y=y;
	  break;
	case 6:
	  v.face=2;
	  v.x=x;
	  v.y=y;
	  break;
	case 7:
	  v.face=6;
	  v.x=x;
	  v.y=y;
	  break;
	case 9:
	  v.face=4;
	  v.x=x;
	  v.y=y;
	  break;
      }
      if (v.face)
      {
	z=0;
        pixel=color(z/zscale);
      }
      else
	pixel="@@@";
      rfile<<pixel;
    }
  }
  rclose();
}
