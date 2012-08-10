/******************************************************/
/*                                                    */
/* cogo.cpp - coordinate geometry                     */
/*                                                    */
/******************************************************/

#include "cogo.h"
#include "bezitopo.h"
#include <stdarg.h>
#include <stdlib.h>
#include <cmath>
using namespace std;

int debugdel;
FILE *randfil;

char intstable[3][3][3][3]=
/* 0  don't intersect
   1  intersection is in the midst of both AC and BD
   2  one end of BD is in the midst of AC
   3  one end of AC is in the midst of BD
   4  one end of AC is one end of BD
   5  A=C or B=D
   6  all four points are collinear
   9  impossible, probably caused by roundoff error
   */
//- - - 0 0 0 + + + B
//- 0 + - 0 + - 0 + D   A C
{ 1,2,0,2,9,9,0,9,9, // - -
  3,4,0,4,9,9,0,9,9, // - 0
  0,0,0,0,5,0,0,0,0, // - +
  3,4,0,4,9,9,0,9,9, // 0 -
  9,9,5,9,6,9,5,9,9, // 0 0
  9,9,0,9,9,4,0,4,3, // 0 +
  0,0,0,0,5,0,0,0,0, // + -
  9,9,0,9,9,4,0,4,3, // + 0
  9,9,0,9,9,2,0,2,1  // + +
  };

double area3(xy a,xy b,xy c)
{int i,j;
 double surface,area[6];
 bool cont;
 area[0]=a.east()*b.north();
 area[1]=-b.east()*a.north();
 area[2]=b.east()*c.north();
 area[3]=-c.east()*b.north();
 area[4]=c.east()*a.north();
 area[5]=-a.east()*c.north();
 do {cont=false; // Sort the six areas into absolute value order for numerical stability.
     for (i=0;i<5;i++)
         if (fabs(area[i+1])<fabs(area[i]))
            {surface=area[i];
             area[i]=area[i+1];
             area[i+1]=surface;
             cont=true;
             }
     } while (cont);
 for (surface=i=0;i<6;i++)
     surface+=area[i];
 surface/=2;
 return surface;
 }

xy intersection (xy a,xy c,xy b,xy d)
//Intersection of lines ac and bd.
{double A,B,C,D;
 A=area3(b,c,d);
 B=area3(c,d,a);
 C=area3(d,a,b);
 D=area3(a,b,c);
 return ((a*A+c*C)+(b*B+d*D))/((A+C)+(B+D));
 }

int intstype (xy a,xy c,xy b,xy d)
//Intersection type - one of 81 numbers, not all possible.
{double A,B,C,D;
 A=area3(b,c,d);
 B=area3(c,d,a);
 C=area3(d,a,b);
 D=area3(a,b,c);
 return (27*sign(A)+9*sign(C)+3*sign(B)+sign(D));
 }

int intersection_type(xy a,xy c,xy b,xy d)
{int itype=intstype(a,c,b,d)+40;
 itype=intstable[itype/27][itype%27/9][itype%9/3][itype%3];
 if (itype==9)
    fprintf(stderr,"Intersection type 9\n(%e,%e)=(%e,%e) × (%e,%e)=(%e,%e)\n(%a,%a)=(%a,%a) × (%a,%a)=(%a,%a)\n",
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north(),
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north());
 assert(itype!=9);
 return itype;
 }

double pldist(xy a,xy b,xy c)
/* Signed distance from a to the line bc. */
{return area3(a,b,c)/dist(b,c)*2;
 }

xy rand2p(xy a,xy b)
/* A random point in the circle with diameter ab. */
{xy mid((a+b)/2);
 unsigned short n;
 xy pnt;
 double angle=(sqrt(5)-1)*M_PI;
 fread(&n,1,2,randfil);
 pnt=xy(cos(angle*n)*sqrt(n+0.5)/256,sin(angle*n)*sqrt(n+0.5)/256);
 pnt=pnt*dist(mid,a)+mid;
 return pnt;
 }

bool delaunay(xy a,xy c,xy b,xy d)
//Returns true if ac satisfies the criterion in the quadrilateral abcd.
//If false, the edge should be flipped to bd.
//The computation is based on the theorem that the two diagonals of
//a quadrilateral inscribed in a circle cut each other into parts
//whose products are equal.
{xy ints;
 double dista,distc,distb,distd,distac,distbd;
 ints=intersection(a,c,b,d);
 distac=dist(a,c);
 distbd=dist(b,d);
 if (isnan(ints.north()))
    {//printf("delaunay:No intersection, distac=%a, distbd=%a\n",distac,distbd);
     return distac<=distbd;
     }
 else
    {dista=dist(a,ints);
     distb=dist(b,ints);
     distc=dist(c,ints);
     distd=dist(d,ints);
     if (dista>distac || distc>distac) dista=-dista;
     if (distb>distbd || distd>distbd) distb=-distb;
     if (debugdel && dista*distc>distb*distd)
        printf("delaunay:dista*distc=%a, distb*distd=%a\n",dista*distc,distb*distd);
     if (dista*distc == distb*distd)
        return distac<=distbd;
     else
        return dista*distc<=distb*distd;
     }
 }
