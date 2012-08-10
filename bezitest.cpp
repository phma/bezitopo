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

int main(int argc, char *argv[])
{int i,j,itype;
 xy a(0,0),b(4,0),c(0,3),d(4,4),e;
 //printf("Area is %7.3f\n",area3(c,a,b));
 //lozenge(100);
 //rotate(30);
 printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
 for (i=0;i<128;i++)
     {printf("sin(%8x)=%a sin(%8x)=%a\n",i<<24,sin(i<<24),(i+128)<<24,sin((i+128)<<24));
      printf("cos(%8x)=%a cos(%8x)=%a\n",i<<24,cos(i<<24),(i+128)<<24,cos((i+128)<<24));
      printf("abs(cis(%8x))=%a\n",i<<24,hypot(cos(i<<24),sin((i+128)<<24)));
      }
 fclose(randfil);
 return EXIT_SUCCESS;
 }
