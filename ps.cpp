/******************************************************/
/*                                                    */
/* ps.cpp - PostScript output                         */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013,2014,2015,2016 Pierre Abbat.
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
 * 
 * Literal PostScript code in this file, which is written to Bezitopo's
 * output, is in the public domain.
 */
#include <cstdio>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cassert>
#include <iomanip>
#include <unistd.h>
#include "config.h"
#include "ps.h"
#include "point.h"
#include "tin.h"
#include "pointlist.h"
#include "plot.h"
#include "document.h"
using namespace std;

FILE *psfile;
int pages;
double scale=1; // paper size is in millimeters, but model space is in meters
int orientation=0;
double oldr=-1,oldg=-2,oldb=-3;
xy paper(210,297),modelcenter;
char rscales[]={10,12,15,20,25,30,40,50,60,80};
const double PSPoint=25.4/72;

int fibmod3(int n)
{
  int i,a,b;
  for (i=a=0,b=1;a<n;i++)
  {
    b+=a;
    a=b-a;
  }
  return (a==n)?(i%3):-1;
}

PostScript::PostScript()
{
  oldr=oldg=oldb=NAN;
  paper=xy(210,297);
  scale=1;
  orientation=pages=0;
  indocument=inpage=false;
  psfile=nullptr;
}

PostScript::~PostScript()
{
  if (psfile)
    close();
}

void PostScript::open(string psfname)
{
  if (psfile)
    close();
  psfile=new ofstream(psfname);
}

void PostScript::prolog()
{
  if (psfile && !indocument)
  {
    *psfile<<"%!PS-Adobe-3.0\n%%BeginProlog\n%%%%Pages: (atend)"<<endl;
    *psfile<<"%%BoundingBox: 0 0 "<<rint(paper.getx()/PSPoint)<<' '<<rint(paper.gety()/PSPoint)<<endl;
    *psfile<<"\n/. % ( x y )\n{ newpath 0.1 0 360 arc fill } bind def\n\n";
    *psfile<<"/- % ( x1 y1 x2 y2 )\n\n{ newpath moveto lineto stroke } bind def\n\n";
    *psfile<<"/mmscale { 720 254 div dup scale } bind def\n";
    *psfile<<"%%EndProlog"<<endl;
    indocument=true;
    pages=0;
  }
}

void PostScript::startpage()
{
  if (psfile && indocument && !inpage)
  {
    ++pages;
    *psfile<<"%%Page: "<<pages<<' '<<pages<<"\ngsave mmscale 0.1 setlinewidth\n";
    *psfile<<"/Helvetica findfont 3 scalefont setfont"<<endl;
    oldr=oldg=oldb=NAN;
    inpage=true;
  }
}

void PostScript::endpage()
{
  if (psfile && indocument && inpage)
  {
    *psfile<<"grestore showpage"<<endl;
    inpage=false;
  }
}

void PostScript::trailer()
{
  if (inpage)
    endpage();
  if (psfile && indocument)
  {
    *psfile<<"%%BeginTrailer\n%%Pages: "<<pages<<"\n%%EndTrailer"<<endl;
    indocument=false;
  }
}

void PostScript::close()
{
  if (indocument)
    trailer();
  delete(psfile);
  psfile=nullptr;
}

void PostScript::setDoc(document &docu)
{
  doc=&docu;
}

double PostScript::xscale(double x)
{
  return scale*(x-modelcenter.east())+paper.getx()/2;
}

double PostScript::yscale(double y)
{
  return scale*(y-modelcenter.north())+paper.gety()/2;
}

void PostScript::setcolor(double r,double g,double b)
{
  if (r!=oldr || g!=oldg || b!=oldb)
  {
    *psfile<<fixed<<setprecision(3)<<r<<' '<<g<<' '<<b<<" setrgbcolor"<<endl;
    oldr=r;
    oldg=g;
    oldb=b;
  }
}

void PostScript::setscale(double minx,double miny,double maxx,double maxy,int ori)
/* To compute minx etc. using dirbound on e.g. a pointlist pl:
 * minx=pl.dirbound(-ori);
 * miny=pl.dirbound(DEG90-ori);
 * maxx=-pl.dirbound(DEG180-ori);
 * maxy=-pl.dirbound(DEG270-ori);
 */
{
  double xsize,ysize;
  int i;
  orientation=ori;
  modelcenter=xy(minx+maxx,miny+maxy)/2;
  xsize=fabs(minx-maxx);
  ysize=fabs(miny-maxy);
  for (scale=1;scale*xsize/10<paper.east() && scale*ysize/10<paper.north();scale*=10);
  for (;scale*xsize/80>paper.east()*0.9 || scale*ysize/80>paper.north()*0.9;scale/=10);
  for (i=0;i<9 && (scale*xsize/rscales[i]>paper.east()*0.9 || scale*ysize/rscales[i]>paper.north()*0.9);i++);
  scale/=rscales[i];
  *psfile<<"% minx="<<minx<<" miny="<<miny<<" maxx="<<maxx<<" maxy="<<maxy<<" scale="<<scale<<endl;
}

void PostScript::dot(xy pnt,string comment)
{
  assert(psfile);
  pnt=turn(pnt,orientation);
  if (isfinite(pnt.east()) && isfinite(pnt.north()))
  {
    *psfile<<fixed<<setprecision(2)<<xscale(pnt.east())<<' '<<yscale(pnt.north())<<" .";
    if (comment.length())
      *psfile<<" %"<<comment;
    *psfile<<endl;
  }
}

void PostScript::circle(xy pnt,double radius)
{
  pnt=turn(pnt,orientation);
  *psfile<<fixed<<setprecision(2)<<xscale(pnt.east())<<yscale(pnt.north())
  <<" newpath "<<scale*radius<<" 0 360 arc fill %"<<radius*radius<<endl;
}

void PostScript::line(edge lin,int num,bool colorfibaster,bool directed)
/* Used in bezitest to show the 2D TIN before the 3D triangles are constructed on it.
 * In bezitopo, use spline(lin.getsegment().approx3d(x)) to show it in 3D.
 */
{
  xy mid,disp,base,ab1,ab2,a,b;
  char *rgb;
  a=*lin.a;
  b=*lin.b;
  a=turn(a,orientation);
  b=turn(b,orientation);
  if (lin.delaunay())
    if (colorfibaster)
      switch (fibmod3(abs(doc->pl[1].revpoints[lin.a]-doc->pl[1].revpoints[lin.b])))
      {
	case -1:
	  setcolor(0.3,0.3,0.3);
	  break;
	case 0:
	  setcolor(1,0.3,0.3);
	  break;
	case 1:
	  setcolor(0,1,0);
	  break;
	case 2:
	  setcolor(0.3,0.3,1);
	  break;
      }
    else
      setcolor(0,0,1);
  else
    setcolor(0,0,0);
  if (directed)
  {
    disp=b-a;
    base=xy(disp.north()/40,disp.east()/-40);
    ab1=a+base;
    ab2=a-base;
    *psfile<<"newpath "<<xscale(b.east())<<' '<<yscale(b.north())<<" moveto "<<xscale(ab1.east())<<' '<<yscale(ab1.north())<<" lineto "<<xscale(ab2.east())<<' '<<yscale(ab2.north())<<" lineto closepath fill"<<endl;
  }
  else
    *psfile<<xscale(a.east())<<' '<<yscale(a.north())<<' '<<xscale(b.east())<<' '<<yscale(b.north())<<" -"<<endl;
  mid=(a+b)/2;
  //fprintf(psfile,"%7.3f %7.3f moveto (%d) show\n",xscale(mid.east()),yscale(mid.north()),num);
}

void PostScript::line2p(xy pnt1,xy pnt2)
{
  pnt1=turn(pnt1,orientation);
  pnt2=turn(pnt2,orientation);
  if (isfinite(pnt1.east()) && isfinite(pnt1.north()) && isfinite(pnt2.east()) && isfinite(pnt2.north()))
    *psfile<<fixed<<setprecision(2)<<xscale(pnt1.east())<<' '<<yscale(pnt1.north())
    <<' '<<xscale(pnt2.east())<<' '<<yscale(pnt2.north())<<" -"<<endl;
}

void PostScript::spline(bezier3d spl)
{
  int i,j,n;
  vector<xyz> seg;
  xy pnt;
  n=spl.size();
  pnt=turn(xy(spl[0][0]),orientation);
  *psfile<<fixed<<setprecision(2)<<xscale(pnt.east())<<' '<<yscale(pnt.north())<<" moveto\n";
  for (i=0;i<n;i++)
  {
    seg=spl[i];
    for (j=1;j<4;j++)
    {
      pnt=turn(xy(seg[j]),orientation);
      if (pnt.isnan())
	cerr<<"NaN point"<<endl;
      *psfile<<xscale(pnt.east())<<' '<<yscale(pnt.north())<<' ';
    }
    *psfile<<"curveto\n";
  }
  *psfile<<"stroke"<<endl;
}

void PostScript::write(xy pnt,string text)
{
  pnt=turn(pnt,orientation);
  *psfile<<fixed<<setprecision(2)<<xscale(pnt.east())<<' '<<yscale(pnt.north())
  <<" moveto ("<<text<<") show"<<endl;
}

void PostScript::comment(string text)
{
  *psfile<<'%'<<text<<endl;
}

void setscale(double minx,double miny,double maxx,double maxy,int ori)
/* To compute minx etc. using dirbound on e.g. a pointlist pl:
 * minx=pl.dirbound(-ori);
 * miny=pl.dirbound(DEG90-ori);
 * maxx=-pl.dirbound(DEG180-ori);
 * maxy=-pl.dirbound(DEG270-ori);
 */
{
  double xsize,ysize;
  int i;
  orientation=ori;
  modelcenter=xy(minx+maxx,miny+maxy)/2;
  xsize=fabs(minx-maxx);
  ysize=fabs(miny-maxy);
  for (scale=1;scale*xsize/10<paper.east() && scale*ysize/10<paper.north();scale*=10);
  for (;scale*xsize/80>paper.east()*0.9 || scale*ysize/80>paper.north()*0.9;scale/=10);
  for (i=0;i<9 && (scale*xsize/rscales[i]>paper.east()*0.9 || scale*ysize/rscales[i]>paper.north()*0.9);i++);
  scale/=rscales[i];
  fprintf(psfile,"%% minx=%f miny=%f maxx=%f maxy=%f scale=%f\n",minx,miny,maxx,maxy,scale);
  //printf("scale=%f\n",scale);
  //sleep(3);
}

void widen(double factor)
{fprintf(psfile,"currentlinewidth %f mul setlinewidth\n",factor);
 }

void setcolor(double r,double g,double b)
{
  if (r!=oldr || g!=oldg || b!=oldb)
  {
    fprintf(psfile,"%f %f %f setrgbcolor\n",r,g,b);
    oldr=r;
    oldg=g;
    oldb=b;
  }
}

/*void psopen(const char * psfname)
{psfile=fopen(psfname,"w");
 }*/

void psclose()
{
  fclose(psfile);
  //printf("scale=%f\n",scale);
  //sleep(3);
}

void psprolog()
{fprintf(psfile,"%%!PS-Adobe-3.0\n\
%%%%BeginProlog\n\
%%%%Pages: (atend)\n\
%%%%BoundingBox: 0 0 596 843\n\
%% A4 paper.\n\
\n\
/. %% ( x y )\n\
{ newpath 0.1 0 360 arc fill } bind def\n\
\n\
/- %% ( x1 y1 x2 y2 )\n\
{ newpath moveto lineto stroke } bind def\n\
\n\
/mmscale { 720 254 div dup scale } bind def\n\
%%%%EndProlog\n\
");
 pages=0;
 //scale=10;
 fflush(psfile);
 }

void pstrailer()
{fprintf(psfile,"%%%%BeginTrailer\n\
%%%%Pages: %d\n\
%%%%EndTrailer\n\
",pages);
 }

double xscale(double x)
{return scale*(x-modelcenter.east())+105;
 }

double yscale(double y)
{return scale*(y-modelcenter.north())+148.5;
 }

void startpage()
{
  ++pages;
  fprintf(psfile,"%%%%Page: %d %d\ngsave mmscale 0.1 setlinewidth\n\
/Helvetica findfont 3 scalefont setfont\n",pages,pages);
  oldr=oldg=oldb=NAN;
}

void endpage()
{fputs("grestore showpage\n",psfile);
 fflush(psfile);
 }

void dot(xy pnt,string comment)
{
  pnt=turn(pnt,orientation);
  if (isfinite(pnt.east()) && isfinite(pnt.north()))
  {
    fprintf(psfile,"%7.3f %7.3f .",
           xscale(pnt.east()),yscale(pnt.north()));
    if (comment.length())
      fprintf(psfile," %%%s",comment.c_str());
    fprintf(psfile,"\n");
  }
}

void pscomment(string text)
{
  fprintf(psfile," %%%s\n",text.c_str());
}

void circle(xy pnt,double radius)
{
  pnt=turn(pnt,orientation);
  fprintf(psfile,"%7.3f %7.3f newpath %.3f 0 360 arc fill %%%f\n",
         xscale(pnt.east()),yscale(pnt.north()),scale*radius,radius*radius);
}

void line(document &doc,edge lin,int num,bool colorfibaster,bool directed)
{
  xy mid,disp,base,ab1,ab2,a,b;
  char *rgb;
  a=*lin.a;
  b=*lin.b;
  a=turn(a,orientation);
  b=turn(b,orientation);
  if (lin.delaunay())
    if (colorfibaster)
      switch (fibmod3(abs(doc.pl[1].revpoints[lin.a]-doc.pl[1].revpoints[lin.b])))
      {
	case -1:
	  setcolor(0.3,0.3,0.3);
	  break;
	case 0:
	  setcolor(1,0.3,0.3);
	  break;
	case 1:
	  setcolor(0,1,0);
	  break;
	case 2:
	  setcolor(0.3,0.3,1);
	  break;
      }
    else
      setcolor(0,0,1);
  else
    setcolor(0,0,0);
  if (directed)
  {
    disp=b-a;
    base=xy(disp.north()/40,disp.east()/-40);
    ab1=a+base;
    ab2=a-base;
    fprintf(psfile,"newpath %7.3f %7.3f moveto %7.3f %7.3f lineto %7.3f %7.3f lineto closepath fill\n",
	    xscale(b.east()),yscale(b.north()),xscale(ab1.east()),yscale(ab1.north()),xscale(ab2.east()),yscale(ab2.north()));
  }
  else
    fprintf(psfile,"%7.3f %7.3f %7.3f %7.3f -\n",
            xscale(a.east()),yscale(a.north()),xscale(b.east()),yscale(b.north()));
  mid=(a+b)/2;
  //fprintf(psfile,"%7.3f %7.3f moveto (%d) show\n",xscale(mid.east()),yscale(mid.north()),num);
}

void line2p(xy pnt1,xy pnt2)
{
  pnt1=turn(pnt1,orientation);
  pnt2=turn(pnt2,orientation);
  if (isfinite(pnt1.east()) && isfinite(pnt1.north()) && isfinite(pnt2.east()) && isfinite(pnt2.north()))
    fprintf(psfile,"%7.3f %7.3f %7.3f %7.3f -\n",
           xscale(pnt1.east()),yscale(pnt1.north()),xscale(pnt2.east()),yscale(pnt2.north()));
}

void spline(bezier3d spl)
{
  int i,j,n;
  vector<xyz> seg;
  xy pnt;
  n=spl.size();
  pnt=turn(xy(spl[0][0]),orientation);
  fprintf(psfile,"%7.3f %7.3f moveto\n",xscale(pnt.east()),yscale(pnt.north()));
  for (i=0;i<n;i++)
  {
    seg=spl[i];
    for (j=1;j<4;j++)
    {
      pnt=turn(xy(seg[j]),orientation);
      if (pnt.isnan())
	cerr<<"NaN point"<<endl;
      fprintf(psfile,"%7.3f %7.3f ",xscale(pnt.east()),yscale(pnt.north()));
    }
    fprintf(psfile,"curveto\n");
  }
  fprintf(psfile,"stroke\n");
}

void pswrite(xy pnt,string text)
{
  pnt=turn(pnt,orientation);
  fprintf(psfile,"%7.3f %7.3f moveto (%s) show\n",xscale(pnt.east()),yscale(pnt.north()),text.c_str());
}
