/******************************************************/
/*                                                    */
/* bezitest.cpp - test program                        */
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

void testintegertrig()
{
  double sinerror,coserror,ciserror,totsinerror,totcoserror,totciserror;
  int i;
  for (totsinerror=totcoserror=totciserror=i=0;i<128;i++)
  {
    sinerror=sin(i<<24)+sin((i+128)<<24);
    coserror=cos(i<<24)+cos((i+128)<<24);
    ciserror=hypot(cos(i<<24),sin((i+128)<<24))-1;
    if (sinerror>0.04 || coserror>0.04 || ciserror>0.04)
    {
      printf("sin(%8x)=%a sin(%8x)=%a\n",i<<24,sin(i<<24),(i+128)<<24,sin((i+128)<<24));
      printf("cos(%8x)=%a cos(%8x)=%a\n",i<<24,cos(i<<24),(i+128)<<24,cos((i+128)<<24));
      printf("abs(cis(%8x))=%a\n",i<<24,hypot(cos(i<<24),sin((i+128)<<24)));
    }
    totsinerror+=sinerror*sinerror;
    totcoserror+=coserror*coserror;
    totciserror+=ciserror*ciserror;
  }
  printf("total sine error=%e\n",totsinerror);
  printf("total cosine error=%e\n",totcoserror);
  printf("total cis error=%e\n",totciserror);
  assert(totsinerror+totcoserror+totciserror<1e-29);
  //On Linux, the total error is 6e-39 and the M_PIl makes a big difference.
  //On DragonFly BSD, the total error is 5e-30 and M_PIl is absent.
}

void test1intersection(xy a,xy c,xy b,xy d,xy inte,int type)
{
  xy inters;
  int itype;
  inters=intersection(a,c,b,d);
  itype=intersection_type(a,c,b,d);
  if ((itype && inters!=inte) || itype!=type)
  {
    printf("intersection is %f,%f, should be %f,%f\n",inters.east(),inters.north(),inte.east(),inte.north());
    printf("intersection type is %d, should be %d\n",itype,type);
  }
  assert(itype==0 || inters==inte);
  assert(itype==type);
}

void testintersection()
{
  xy a(1,0),b(-0.5,0.866),c(-0.5,-0.866),d(-0.5,0),e(0.25,-0.433),f(0.25,0.433),o(0,0);
  int intype;
  test1intersection(a,d,b,e,o,1);
  test1intersection(b,e,c,f,o,1);
  test1intersection(c,f,a,d,o,1);
  test1intersection(a,b,d,e,o,0);
  test1intersection(b,c,e,f,o,0);
  test1intersection(c,a,f,d,o,0);
  test1intersection(a,c,b,e,e,2);
  test1intersection(b,a,c,f,f,2);
  test1intersection(c,b,a,d,d,2);
  test1intersection(d,e,a,c,e,3);
  test1intersection(e,f,b,a,f,3);
  test1intersection(f,d,c,b,d,3);
  test1intersection(a,b,c,a,a,4);
  test1intersection(b,c,a,b,b,4);
  test1intersection(c,a,b,c,c,4);
  //test1intersection(a,c,b,b,o,5);
}
  
int main(int argc, char *argv[])
{int i,j,itype;
 randfil=fopen("/dev/urandom","rb");
 xy a(0,0),b(4,0),c(0,3),d(4,4),e;
 assert(area3(c,a,b)==6);
 testintersection();
 lozenge(100);
 rotate(30);
 printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
 testintegertrig();
 fclose(randfil);
 return EXIT_SUCCESS;
 }
