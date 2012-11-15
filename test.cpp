/******************************************************/
/*                                                    */
/* test.cpp - test patterns and functions             */
/*                                                    */
/******************************************************/

#include <cmath>
#include "bezitopo.h"
#include "test.h"
#include "pointlist.h"

using std::map;

void dumppoints()
{map<int,point>::iterator i;
 printf("dumppoints\n");
 for (i=topopoints.points.begin();i!=topopoints.points.end();i++)
     i->second.dump();
 printf("end dump\n");
 }

void dumppointsvalence()
{
  map<int,point>::iterator i;
  printf("dumppoints\n");
  for (i=topopoints.points.begin();i!=topopoints.points.end();i++)
    printf("%d %d\n",i->first,i->second.valence());
  printf("end dump\n");
}

double rugae(xy pnt)
{double z;
 z=sin(pnt.east())+pnt.north()/50;
 return z;
 }

double (*testsurface)(xy pnt)=rugae;

void aster(int n)
/* Fill points with asteraceous pattern. Pattern invented by H. Vogel in 1979
   and later by me, not knowing of Vogel. */
{int i;
 double angle=(sqrt(5)-1)*M_PI;
 xy pnt;
 for (i=0;i<n;i++)
     {pnt=xy(cos(angle*i)*sqrt(i+0.5),sin(angle*i)*sqrt(i+0.5));
      topopoints.addpoint(i+1,point(pnt,testsurface(pnt),"test"));
      }
 }

void _ellipse(int n,double skewness)
/* Skewness is not eccentricity. When skewness=0.01, eccentricity=0.14072. */
{
  int i;
  double angle=(sqrt(5)-1)*M_PI;
  xy pnt;
  for (i=0;i<n;i++)
  {
    pnt=xy(cos(angle*i)*sqrt(n+0.5)*(1-skewness),sin(angle*i)*sqrt(n+0.5)*(1+skewness));
    topopoints.addpoint(i+1,point(pnt,testsurface(pnt),"test"));
  }
}

void ring(int n)
/* Points in a circle, for most ambiguous case of the Delaunay algorithm.
 * The number of different ways to make the TIN is a Catalan number.
 */
{
  _ellipse(n,0);
}

void ellipse(int n)
/* Points in an ellipse, for worst case of the Delaunay algorithm. */
{
  _ellipse(n,0.01);
}

void longandthin(int n)
{
  _ellipse(n,0.999);
}

void straightrow(int n)
// Add points in a straight line.
{
  int i;
  double angle;
  xy pnt;
  for (i=0;i<n;i++)
  {
    angle=(2.0*i/(n-1)-1)*M_PI/6;
    pnt=xy(0,sqrt(n)*tan(angle));
    topopoints.addpoint(i+1,point(pnt,testsurface(pnt),"test"));
  }
}

void lozenge(int n)
// Add points on the short diagonal of a rhombus, then add the two other points.
{
  xy pnt;
  straightrow(n);
  pnt=xy(-sqrt(n),0);
  topopoints.addpoint(n+1,point(pnt,testsurface(pnt),"test"));
  pnt=xy(sqrt(n),0);
  topopoints.addpoint(n+2,point(pnt,testsurface(pnt),"test"));
}

void rotate(int n)
{int i;
 double tmpx,tmpy;
 map<int,point>::iterator j;
 for (j=topopoints.points.begin();j!=topopoints.points.end();j++)
     for (i=0;i<n;i++)
         {tmpx=j->second.x*0.6-j->second.y*0.8;
          tmpy=j->second.y*0.6+j->second.x*0.8;
          j->second.x=tmpx;
          j->second.y=tmpy;
          }
 }

void movesideways(double sw)
{
  int i;
  double tmpx,tmpy;
  map<int,point>::iterator j;
  for (j=topopoints.points.begin();j!=topopoints.points.end();j++)
    j->second.x+=sw;
}
