/******************************************************/
/*                                                    */
/* ps.cpp - PostScript output                         */
/*                                                    */
/******************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "point.h"
#include "tin.h"

FILE *psfile;
int pages;
double scale=1; // paper size is in millimeters, but model space is in meters
int orientation;
xy paper(210,297),modelcenter;
char rscales[]={10,12,15,20,25,30,40,50,60,80};

void setscale(double minx,double miny,double maxx,double maxy)
{double xsize,ysize;
 int i;
 modelcenter=xy(minx+maxx,miny+maxy)/2;
 xsize=fabs(minx-maxx);
 ysize=fabs(miny-maxy);
 for (scale=1;scale*xsize/10<paper.east() && scale*ysize/10<paper.north();scale*=10);
 for (;scale*xsize/80>paper.east()*0.9 || scale*ysize/80>paper.north()*0.9;scale/=10);
 for (i=0;i<9 && (scale*xsize/rscales[i]>paper.east()*0.9 || scale*ysize/rscales[i]>paper.north()*0.9);i++);
 scale/=rscales[i];
 printf("scale=%f\n",scale);
 //sleep(3);
 }

void widen(double factor)
{fprintf(psfile,"currentlinewidth %f mul setlinewidth\n",factor);
 }

void setcolor(double r,double g,double b)
{fprintf(psfile,"%f %f %f setrgbcolor\n",r,g,b);
 }

void psopen(char * psfname)
{psfile=fopen(psfname,"w");
 }

void psclose()
{fclose(psfile);
 printf("scale=%f\n",scale);
 sleep(3);
 }

void psprolog()
{fprintf(psfile,"%%!PS-Adobe-3.0\n\
%%%%BeginProlog\n\
%%%%Pages: (atend)\n\
%%%%BoundingBox: 0 0 596 843\n\
%% A4 paper.\n\
\n\
/. %% ( x y )\n\
{ newpath 0.3 0 360 arc fill } bind def\n\
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
{++pages;
 fprintf(psfile,"%%%%Page: %d %d\ngsave mmscale 0.1 setlinewidth\n\
/Helvetica findfont 3 scalefont setfont\n",pages,pages);
 }

void endpage()
{fputs("grestore showpage\n",psfile);
 fflush(psfile);
 }

void dot(xy pnt)
{double r,g,b;
 fprintf(psfile,"%7.3f %7.3f .\n",
        xscale(pnt.east()),yscale(pnt.north()));
 }

void line(edge lin,int num)
{xy mid;
 if (lin.delaunay())
    fputs("0 0 1 setrgbcolor\n",psfile);
 else
    fputs("0 0 0 setrgbcolor\n",psfile);
 fprintf(psfile,"%7.3f %7.3f %7.3f %7.3f -\n",
        xscale(lin.a->east()),yscale(lin.a->north()),xscale(lin.b->east()),yscale(lin.b->north()));
 mid=((xy)*lin.a+*lin.b)/2;
 //fprintf(psfile,"%7.3f %7.3f moveto (%d) show\n",xscale(mid.east()),yscale(mid.north()),num);
 }

void line2p(xy pnt1,xy pnt2)
{fprintf(psfile,"%7.3f %7.3f %7.3f %7.3f -\n",
        xscale(pnt1.east()),yscale(pnt1.north()),xscale(pnt2.east()),yscale(pnt2.north()));
 }
