/******************************************************/
/*                                                    */
/* test.cpp - test patterns and functions             */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013,2014,2015,2017 Pierre Abbat.
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

#define _USE_MATH_DEFINES
#include <cmath>
#include "bezitopo.h"
#include "test.h"
#include "angle.h"
#include "pointlist.h"

using std::map;

void dumppoints()
{
  map<int,point>::iterator i;
  printf("dumppoints\n");
  //for (i=doc.pl[1].points.begin();i!=doc.pl[1].points.end();i++)
  //    i->second.dump();
  printf("end dump\n");
}

void dumppointsvalence(document &doc)
{
  map<int,point>::iterator i;
  printf("dumppoints\n");
  for (i=doc.pl[1].points.begin();i!=doc.pl[1].points.end();i++)
    printf("%d %d\n",i->first,i->second.valence());
  printf("end dump\n");
}

double flatslope(xy pnt)
{
  double z;
  z=pnt.east()/7+pnt.north()/17;
  return z;
}

xy flatslopegrad(xy pnt)
{
  return xy(1/7.,1/17.);
}

double rugae(xy pnt)
{
  double z;
  z=sin(pnt.east())+pnt.north()/50;
  return z;
}

xy rugaegrad(xy pnt)
{
  return xy(cos(pnt.east()),0.02);
}

double hypar(xy pnt)
{
  double z;
  z=(sqr(pnt.east())-sqr(pnt.north()))/50;
  return z;
}

xy hypargrad(xy pnt)
{
  return xy(pnt.east()/25,-pnt.north()/25);
}

double cirpar(xy pnt)
{
  double z;
  z=(sqr(pnt.east())+sqr(pnt.north()))/50;
  return z;
}

xy cirpargrad(xy pnt)
{
  return xy(pnt.east()/25,pnt.north()/25);
}

double hash(xy pnt)
{
  int acc0,acc1,i;
  char *p;
  for (p=(char *)&pnt,i=acc0=acc1=0;i<sizeof(pnt);++i,++p)
  {
    acc0=((acc0<<8)+*p)%263;
    acc1=((acc1<<8)+*p)%269;
  }
  return acc0/263.-acc1/269.;
}

xy hashgrad(xy pnt)
{
  return xy(NAN,NAN);
}

double (*testsurface)(xy pnt)=rugae;
xy (*testsurfacegrad)(xy pnt)=rugaegrad;

void setsurface(int surf)
{
  switch (surf)
  {
    case RUGAE:
      testsurface=rugae;
      testsurfacegrad=rugaegrad;
      break;
    case HYPAR:
      testsurface=hypar;
      testsurfacegrad=hypargrad;
      break;
    case CIRPAR:
      testsurface=cirpar;
      testsurfacegrad=cirpargrad;
      break;
    case HASH:
      testsurface=hash;
      testsurfacegrad=hashgrad;
      break;
    case FLATSLOPE:
      testsurface=flatslope;
      testsurfacegrad=flatslopegrad;
      break;
  }
}

void aster(document &doc,int n)
/* Fill points with asteraceous pattern. Pattern invented by H. Vogel in 1979
   and later by me, not knowing of Vogel. */
{int i;
 double angle=(sqrt(5)-1)*M_PI;
 xy pnt;
 for (i=0;i<n;i++)
     {pnt=xy(cos(angle*i)*sqrt(i+0.5),sin(angle*i)*sqrt(i+0.5));
      doc.pl[1].addpoint(i+1,point(pnt,testsurface(pnt),"test"));
      }
 }

void _ellipse(document &doc,int n,double skewness)
/* Skewness is not eccentricity. When skewness=0.01, eccentricity=0.14072. */
{
  int i;
  double angle=(sqrt(5)-1)*M_PI;
  xy pnt;
  for (i=0;i<n;i++)
  {
    pnt=xy(cos(angle*i)*sqrt(n+0.5)*(1-skewness),sin(angle*i)*sqrt(n+0.5)*(1+skewness));
    doc.pl[1].addpoint(i+1,point(pnt,testsurface(pnt),"test"));
  }
}

void regpolygon(document &doc,int n)
{
  int i;
  double angle=2*M_PI/n;
  xy pnt;
  for (i=0;i<n;i++)
  {
    pnt=xy(cos(angle*i)*sqrt(n+0.5),sin(angle*i)*sqrt(n+0.5));
    doc.pl[1].addpoint(i+1,point(pnt,testsurface(pnt),"test"));
  }
}

void ring(document &doc,int n)
/* Points in a circle, for most ambiguous case of the Delaunay algorithm.
 * The number of different ways to make the TIN is a Catalan number.
 */
{
  _ellipse(doc,n,0);
}

void ellipse(document &doc,int n)
/* Points in an ellipse, for worst case of the Delaunay algorithm. */
{
  _ellipse(doc,n,0.01);
}

void longandthin(document &doc,int n)
{
  _ellipse(doc,n,0.999);
}

void straightrow(document &doc,int n)
// Add points in a straight line.
{
  int i;
  double angle;
  xy pnt;
  for (i=0;i<n;i++)
  {
    angle=(2.0*i/(n-1)-1)*M_PI/6;
    pnt=xy(0,sqrt(n)*tan(angle));
    doc.pl[1].addpoint(i+1,point(pnt,testsurface(pnt),"test"));
  }
}

void lozenge(document &doc,int n)
// Add points on the short diagonal of a rhombus, then add the two other points.
{
  xy pnt;
  straightrow(doc,n);
  pnt=xy(-sqrt(n),0);
  doc.pl[1].addpoint(n+1,point(pnt,testsurface(pnt),"test"));
  pnt=xy(sqrt(n),0);
  doc.pl[1].addpoint(n+2,point(pnt,testsurface(pnt),"test"));
}

void wheelwindow(document &doc,int n)
{
  int i;
  double angle=2*M_PI/n;
  xy pnt;
  pnt=xy(0,0);
  doc.pl[1].addpoint(1,point(pnt,testsurface(pnt),"test"));
  for (i=0;i<n;i++)
  {
    pnt=cossin(angle*i)*sqrt(n+0.5);
    doc.pl[1].addpoint(2*i+1,point(pnt,testsurface(pnt),"test"));
    pnt=cossin(angle*(i+0.5))*sqrt(n+6.5);
    doc.pl[1].addpoint(2*i+2,point(pnt,testsurface(pnt),"test"));
  }
}

void rotate(document &doc,int n)
{int i;
 double tmpx,tmpy;
 map<int,point>::iterator j;
 for (j=doc.pl[1].points.begin();j!=doc.pl[1].points.end();j++)
     for (i=0;i<n;i++)
         {tmpx=j->second.x*0.6-j->second.y*0.8;
          tmpy=j->second.y*0.6+j->second.x*0.8;
          j->second.x=tmpx;
          j->second.y=tmpy;
          }
 }

void movesideways(document &doc,double sw)
{
  int i;
  double tmpx,tmpy;
  map<int,point>::iterator j;
  for (j=doc.pl[1].points.begin();j!=doc.pl[1].points.end();j++)
    j->second.x+=sw;
}

void moveup(document &doc,double sw)
{
  int i;
  double tmpx,tmpy;
  map<int,point>::iterator j;
  for (j=doc.pl[1].points.begin();j!=doc.pl[1].points.end();j++)
    j->second.z+=sw;
}

void enlarge(document &doc,double sc)
{
  int i;
  double tmpx,tmpy;
  map<int,point>::iterator j;
  for (j=doc.pl[1].points.begin();j!=doc.pl[1].points.end();j++)
  {
    j->second.x*=sc;
    j->second.y*=sc;
  }
}
