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

using namespace std;

map<int,point> points;
map<point*,int> revpoints;
FILE *randfil;

void addpoint(int numb,point pnt,bool overwrite)
// If numb<0, it's a point added by bezitopo.
{int a;
 if (points.count(numb))
    if (overwrite)
       points[a=numb]=pnt;
    else
       {if (numb<0)
           {a=points.begin()->first-1;
            if (a>=0)
               a=-1;
            }
        else
           {a=points.rbegin()->first+1;
            if (a<=0)
               a=1;
            }
        points[a]=pnt;
        }
 else
    points[a=numb]=pnt;
 revpoints[&(points[a])]=a;
 }

void dumppoints()
{map<int,point>::iterator i;
 printf("dumppoints\n");
 for (i=points.begin();i!=points.end();i++)
     i->second.dump();
 printf("end dump\n");
 }

int main(int argc, char *argv[])
{int i,j,itype;
 xy a(0,0),b(4,0),c(0,3),d(4,4),e;
 //printf("Area is %7.3f\n",area3(c,a,b));
 randfil=fopen("/dev/urandom","rb");
 //lozenge(100);
 //rotate(30);
 set_length_unit(SURVEYFOOT);
 if (readpnezd("topo0.asc")<0)
   readpnezd("../topo0.asc");
 rotate(2);
 /*for (i=0;i<9;i++)
     for (j=0;j<=i;j++)
         {itype=intersection_type(points[i],points[i+1],points[j],points[j+1]);
          e=intersection(points[i],points[i+1],points[j],points[j+1]);
          printf("i=%d j=%d Intersection type %d\nIntersection is (%f,%f)\n",i,j,itype,e.east(),e.north());
          }*/
 maketin();
 printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
 for (i=0;i<128;i++)
     {printf("sin(%8x)=%a sin(%8x)=%a\n",i<<24,sin(i<<24),(i+128)<<24,sin((i+128)<<24));
      printf("cos(%8x)=%a cos(%8x)=%a\n",i<<24,cos(i<<24),(i+128)<<24,cos((i+128)<<24));
      printf("abs(cis(%8x))=%a\n",i<<24,hypot(cos(i<<24),sin((i+128)<<24)));
      }
 fclose(randfil);
 return EXIT_SUCCESS;
 }
