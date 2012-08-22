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
#include "pointlist.h"
#include "vcurve.h"

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
  bool noint;
  inters=intersection(a,c,b,d);
  itype=intersection_type(a,c,b,d);
  noint=itype==0 || itype==5 || itype==6;
  if ((!noint && inters!=inte) || itype!=type)
  {
    printf("intersection is %f,%f, should be %f,%f\n",inters.east(),inters.north(),inte.east(),inte.north());
    printf("intersection type is %d, should be %d\n",itype,type);
  }
  assert(noint || inters==inte);
  assert(itype==type);
}

void testintersection()
{
  xy a(1,0),b(-0.5,0.866),c(-0.5,-0.866),d(-0.5,0),e(0.25,-0.433),f(0.25,0.433),o(0,0);
  int intype;
  test1intersection(a,b,d,e,o,NOINT);
  test1intersection(b,c,e,f,o,NOINT);
  test1intersection(c,a,f,d,o,NOINT);
  test1intersection(a,d,b,e,o,ACXBD);
  test1intersection(b,e,c,f,o,ACXBD);
  test1intersection(c,f,a,d,o,ACXBD);
  test1intersection(a,c,b,e,e,BDTAC);
  test1intersection(b,a,c,f,f,BDTAC);
  test1intersection(c,b,a,d,d,BDTAC);
  test1intersection(d,e,a,c,e,ACTBD);
  test1intersection(e,f,b,a,f,ACTBD);
  test1intersection(f,d,c,b,d,ACTBD);
  test1intersection(a,b,c,a,a,ACVBD);
  test1intersection(b,c,a,b,b,ACVBD);
  test1intersection(c,a,b,c,c,ACVBD);
  test1intersection(a,c,b,b,o,COINC);
  test1intersection(b,a,c,c,o,COINC);
  test1intersection(c,b,a,a,o,COINC);
  test1intersection(b,c,d,b,c,COLIN);
  test1intersection(c,a,e,c,a,COLIN);
  test1intersection(a,b,f,a,b,COLIN);
}

void testcopytopopoints()
{
  criteria crit;
  criterion crit1;
  surveypoints.clear();
  copytopopoints(crit);
  assert(topopoints.points.size()==0);
  surveypoints.addpoint(1,point(0,0,0,"eip"));
  surveypoints.addpoint(1,point(25,0,0,"eip"));
  surveypoints.addpoint(1,point(25,40,0,"eip"));
  surveypoints.addpoint(1,point(0,40,0,"eip"));
  surveypoints.addpoint(1,point(5,10,0,"house"));
  surveypoints.addpoint(1,point(20,10,0,"house"));
  surveypoints.addpoint(1,point(20,20,0,"house"));
  surveypoints.addpoint(1,point(5,20,0,"house"));
  surveypoints.addpoint(1,point(3,-5,0,"pipe"));
  surveypoints.addpoint(1,point(3,41,0,"pipe"));
  surveypoints.addpoint(1,point(2,22,0,"topo"));
  surveypoints.addpoint(1,point(23,22,0,"topo"));
  surveypoints.addpoint(1,point(12.5,38,0,"topo"));
  crit1.str="topo";
  crit1.istopo=true;
  crit.push_back(crit1);
  copytopopoints(crit);
  assert(topopoints.points.size()==3);
}

void checkimpos(int itype,xy a,xy c,xy b,xy d)
{
  if (itype==IMPOS)
    fprintf(stderr,"Impossible intersection type\n(%e,%e)=(%e,%e) × (%e,%e)=(%e,%e)\n(%a,%a)=(%a,%a) × (%a,%a)=(%a,%a)\n",
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north(),
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north());
  assert(itype!=IMPOS);
}

void testinvalidintersectionlozenge()
{
  int i,j,k,l,m,itype;
  char rightanswers[9][9][9][9];
  topopoints.clear();
  lozenge(7);
  for (j=1;j<=9;j++)
    for (k=1;k<=9;k++)
      for (l=1;l<=9;l++)
        for (m=1;m<=9;m++)
	{
	  itype=intersection_type(topopoints.points[j],topopoints.points[k],
			          topopoints.points[l],topopoints.points[m]);
          rightanswers[j-1][k-1][l-1][m-1]=itype;
	}
  for (i=0;i<81;i++)
  {
    movesideways(cos((double)i)/16);
    rotate(1);
    for (j=1;j<=9;j++)
      for (k=1;k<=9;k++)
	for (l=1;l<=9;l++)
	  for (m=1;m<=9;m++)
	  {
	    itype=intersection_type(topopoints.points[j],topopoints.points[k],
				    topopoints.points[l],topopoints.points[m]);
	    checkimpos(itype,topopoints.points[j],topopoints.points[k],
		       topopoints.points[l],topopoints.points[m]);
	    /*if (itype!=rightanswers[j-1][k-1][l-1][m-1])
	      printf("Iter %i, %d %d %d %d, %d should be %d\n",i,j,k,l,m,itype,rightanswers[j-1][k-1][l-1][m-1]);*/
	  }
  }
}

void testinvalidintersectionaster()
{
  int i,j,k,l,m,itype,nmisses;
  char rightanswers[9][9][9][9];
  double shift;
  topopoints.clear();
  aster(9);
  for (j=1;j<=9;j++)
    for (k=1;k<=9;k++)
      for (l=1;l<=9;l++)
        for (m=1;m<=9;m++)
	{
	  itype=intersection_type(topopoints.points[j],topopoints.points[k],
			          topopoints.points[l],topopoints.points[m]);
          rightanswers[j-1][k-1][l-1][m-1]=itype;
	}
  for (i=0,shift=2e7;i<81;i++)
  {
    nmisses=0;
    shift=shift*31/24;
    movesideways(cos((double)i)*shift);
    rotate(1);
    for (j=1;j<=9;j++)
      for (k=1;k<=9;k++)
	for (l=1;l<=9;l++)
	  for (m=1;m<=9;m++)
	  {
	    itype=intersection_type(topopoints.points[j],topopoints.points[k],
				    topopoints.points[l],topopoints.points[m]);
	    if (itype!=rightanswers[j-1][k-1][l-1][m-1])
	      //printf("Iter %i, %d %d %d %d, %d should be %d\n",i,j,k,l,m,itype,rightanswers[j-1][k-1][l-1][m-1]);
	      nmisses++;
	    checkimpos(itype,topopoints.points[j],topopoints.points[k],
		       topopoints.points[l],topopoints.points[m]);
	  }
  }
}

void testmaketinaster()
{
  double totallength;
  int i;
  topopoints.clear();
  aster(100);
  maketin();
  assert(edgelist.size()==284);
  for (totallength=i=0;i<edgelist.size();i++)
    totallength+=edgelist[i].length();
  assert(fabs(totallength-600.689)<0.001);
}

void testvcurve()
{
  assert(vcurve(0,0,0,0,0)==0);
  assert(vcurve(0,1,2,3,0.5)==1.5);
  assert(vcurve(0,4,4,0,0.5)==3);
  assert(vcurve(0,1,-1,0,0.5)==0);
  assert(vcurve(0,0,0,64,0)==0);
  assert(vcurve(0,0,0,64,0.25)==1);
  assert(vcurve(0,0,0,64,0.5)==8);
  assert(vcurve(0,0,0,64,0.75)==27);
  assert(vcurve(0,0,0,64,1)==64);
  assert(vslope(0,0,0,0,0)==0);
  assert(vslope(0,1,2,3,0.5)==3);
  assert(vslope(0,1,2,3,0.25)==3);
  assert(vslope(0,4,4,0,0.5)==0);
  assert(vslope(0,4,4,0,0)==12);
  assert(vslope(0,0,0,64,0)==0);
  assert(vslope(0,0,0,64,0.25)==12);
  assert(vslope(0,0,0,64,0.5)==48);
  assert(vslope(0,0,0,64,0.75)==108);
  assert(vslope(0,0,0,64,1)==192);
  assert(vaccel(0,0,0,0,0)==0);
  assert(vaccel(0,1,2,3,0.5)==0);
  assert(vaccel(0,1,2,3,0.25)==0);
  assert(vaccel(0,4,4,0,0.5)==-24);
  assert(vaccel(0,4,4,0,0)==-24);
  assert(vaccel(0,0,0,64,0)==0);
  assert(vaccel(0,0,0,64,0.25)==96);
  assert(vaccel(0,0,0,64,0.5)==192);
  assert(vaccel(0,0,0,64,0.75)==288);
  assert(vaccel(0,0,0,64,1)==384);
}

int main(int argc, char *argv[])
{int i,j,itype;
 randfil=fopen("/dev/urandom","rb");
 xy a(0,0),b(4,0),c(0,3),d(4,4),e;
 assert(area3(c,a,b)==6);
 testintersection();
 testcopytopopoints();
 testinvalidintersectionlozenge();
 testinvalidintersectionaster();
 testmaketinaster();
 testvcurve();
 lozenge(100);
 rotate(30);
 printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
 testintegertrig();
 fclose(randfil);
 return EXIT_SUCCESS;
 }
