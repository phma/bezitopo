/******************************************************/
/*                                                    */
/* bezitest.cpp - test program                        */
/*                                                    */
/******************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <cfloat>
#include <cstring>
#include "point.h"
#include "cogo.h"
#include "bezitopo.h"
#include "bezier.h"
#include "test.h"
#include "tin.h"
#include "measure.h"
#include "pnezd.h"
#include "angle.h"
#include "pointlist.h"
#include "vcurve.h"
#include "segment.h"
#include "arc.h"
#include "spiral.h"
#include "cogospiral.h"
#include "qindex.h"
#include "random.h"
#include "ps.h"
#include "raster.h"
#include "stl.h"
#include "halton.h"
#include "polyline.h"
#include "bezier3d.h"
#include "closure.h"
#include "manysum.h"
#include "ldecimal.h"
#include "ellipsoid.h"
#include "color.h"
#include "document.h"
#include "relprime.h"
#include "contour.h"
#include "absorient.h"
#include "hlattice.h"
#include "geoid.h"
#include "binio.h"
#include "sourcegeoid.h"

#define psoutput false
// affects only maketin

#define CBRT2 1.2599210498948731647592197537

char hexdig[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
bool slowmanysum=false;
document doc;

using namespace std;

void testintegertrig()
{
  double sinerror,coserror,ciserror,totsinerror,totcoserror,totciserror;
  int i;
  char bs=8;
  for (totsinerror=totcoserror=totciserror=i=0;i<128;i++)
  {
    sinerror=sin(i<<24)+sin((i+64)<<24);
    coserror=cos(i<<24)+cos((i+64)<<24);
    ciserror=hypot(cos(i<<24),sin(i<<24))-1;
    if (sinerror>0.04 || coserror>0.04 || ciserror>0.04)
    {
      printf("sin(%8x)=%a sin(%8x)=%a\n",i<<24,sin(i<<24),(i+64)<<24,sin((i+64)<<24));
      printf("cos(%8x)=%a cos(%8x)=%a\n",i<<24,cos(i<<24),(i+64)<<24,cos((i+64)<<24));
      printf("abs(cis(%8x))=%a\n",i<<24,hypot(cos(i<<24),sin(i<<24)));
    }
    totsinerror+=sinerror*sinerror;
    totcoserror+=coserror*coserror;
    totciserror+=ciserror*ciserror;
  }
  printf("total sine error=%e\n",totsinerror);
  printf("total cosine error=%e\n",totcoserror);
  printf("total cis error=%e\n",totciserror);
  assert(totsinerror+totcoserror+totciserror<2e-29);
  //On Linux, the total error is 2e-38 and the M_PIl makes a big difference.
  //On DragonFly BSD, the total error is 1.7e-29 and M_PIl is absent.
  assert(bintodeg(0)==0);
  assert(fabs(bintodeg(0x15555555)-60)<0.0000001);
  assert(fabs(bintomin(0x08000000)==1350));
  assert(fabs(bintosec(0x12345678)-184320)<0.001);
  assert(fabs(bintogon(0x1999999a)-80)<0.0000001);
  assert(fabs(bintorad(0x4f1bbcdd)-3.88322208)<0.00000001);
  for (i=-2147400000;i<2147400000;i+=rng.usrandom()+18000)
  {
    cout<<setw(11)<<i<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs;
    cout.flush();
    assert(degtobin(bintodeg(i))==i);
    assert(mintobin(bintomin(i))==i);
    assert(sectobin(bintosec(i))==i);
    assert(gontobin(bintogon(i))==i);
    assert(radtobin(bintorad(i))==i);
  }
  assert(sectobin(1295999.9999)==-2147483648);
  assert(sectobin(1296000.0001)==-2147483648);
  assert(sectobin(-1295999.9999)==-2147483648);
  assert(sectobin(-1296000.0001)==-2147483648);
  cout<<"           "<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs;
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
  doc.pl[0].clear();
  doc.copytopopoints(crit);
  assert(doc.pl[1].points.size()==0);
  doc.pl[0].addpoint(1,point(0,0,0,"eip"));
  doc.pl[0].addpoint(1,point(25,0,0,"eip"));
  doc.pl[0].addpoint(1,point(25,40,0,"eip"));
  doc.pl[0].addpoint(1,point(0,40,0,"eip"));
  doc.pl[0].addpoint(1,point(5,10,0,"house"));
  doc.pl[0].addpoint(1,point(20,10,0,"house"));
  doc.pl[0].addpoint(1,point(20,20,0,"house"));
  doc.pl[0].addpoint(1,point(5,20,0,"house"));
  doc.pl[0].addpoint(1,point(3,-5,0,"pipe"));
  doc.pl[0].addpoint(1,point(3,41,0,"pipe"));
  doc.pl[0].addpoint(1,point(2,22,0,"topo"));
  doc.pl[0].addpoint(1,point(23,22,0,"topo"));
  doc.pl[0].addpoint(1,point(12.5,38,0,"topo"));
  crit1.str="topo";
  crit1.istopo=true;
  crit.push_back(crit1);
  doc.copytopopoints(crit);
  assert(doc.pl[1].points.size()==3);
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
  doc.pl[1].clear();
  lozenge(doc,7);
  for (j=1;j<=9;j++)
    for (k=1;k<=9;k++)
      for (l=1;l<=9;l++)
        for (m=1;m<=9;m++)
	{
	  itype=intersection_type(doc.pl[1].points[j],doc.pl[1].points[k],
			          doc.pl[1].points[l],doc.pl[1].points[m]);
          rightanswers[j-1][k-1][l-1][m-1]=itype;
	}
  for (i=0;i<81;i++)
  {
    movesideways(doc,cos((double)i)/16);
    rotate(doc,1);
    for (j=1;j<=9;j++)
      for (k=1;k<=9;k++)
	for (l=1;l<=9;l++)
	  for (m=1;m<=9;m++)
	  {
	    itype=intersection_type(doc.pl[1].points[j],doc.pl[1].points[k],
				    doc.pl[1].points[l],doc.pl[1].points[m]);
	    checkimpos(itype,doc.pl[1].points[j],doc.pl[1].points[k],
		       doc.pl[1].points[l],doc.pl[1].points[m]);
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
  cout<<"Checking for invalid intersections";
  doc.pl[1].clear();
  aster(doc,9);
  for (j=1;j<=9;j++)
    for (k=1;k<=9;k++)
      for (l=1;l<=9;l++)
        for (m=1;m<=9;m++)
	{
	  itype=intersection_type(doc.pl[1].points[j],doc.pl[1].points[k],
			          doc.pl[1].points[l],doc.pl[1].points[m]);
          rightanswers[j-1][k-1][l-1][m-1]=itype;
	}
  for (i=0,shift=2e7;i<81;i++)
  {
    nmisses=0;
    shift=shift*31/24;
    movesideways(doc,cos((double)i)*shift);
    rotate(doc,1);
    if (!(i&7))
    {
      cout<<".";
      cout.flush();
    }
    for (j=1;j<=9;j++)
      for (k=1;k<=9;k++)
	for (l=1;l<=9;l++)
	  for (m=1;m<=9;m++)
	  {
	    itype=intersection_type(doc.pl[1].points[j],doc.pl[1].points[k],
				    doc.pl[1].points[l],doc.pl[1].points[m]);
	    if (itype!=rightanswers[j-1][k-1][l-1][m-1])
	      //printf("Iter %i, %d %d %d %d, %d should be %d\n",i,j,k,l,m,itype,rightanswers[j-1][k-1][l-1][m-1]);
	      nmisses++;
	    checkimpos(itype,doc.pl[1].points[j],doc.pl[1].points[k],
		       doc.pl[1].points[l],doc.pl[1].points[m]);
	  }
  }
  cout<<endl;
}

void testpointedg()
/* Tests the method point::edg for every triangle in the TIN.
 * When passed the three corners of a triangle, it should return the three sides.
 */
{
  int i;
  edge *sa,*sb,*sc;
  for (i=0;i<doc.pl[1].triangles.size();i++)
  {
    sa=doc.pl[1].triangles[i].a->edg(&doc.pl[1].triangles[i]);
    sb=doc.pl[1].triangles[i].b->edg(&doc.pl[1].triangles[i]);
    sc=doc.pl[1].triangles[i].c->edg(&doc.pl[1].triangles[i]);
    if (sa==sb || sb==sc || sc==sa)
      cout<<"Two sides of triangle "<<i<<" are the same"<<endl;
    if (sa==NULL || sb==NULL || sc==NULL)
      cout<<"A side of triangle "<<i<<" is NULL"<<endl;
    assert(sa!=sb && sb!=sc && sc !=sa && sa!=NULL && sb!=NULL && sc!=NULL);
    if (sa->a==doc.pl[1].triangles[i].b || sa->b==doc.pl[1].triangles[i].b)
      cout<<"sa is not opposite b"<<endl;
  }
}

void testmaketin123()
{
  int i;
  doc.pl[1].clear();
  aster(doc,1);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==notri);
  doc.pl[1].clear();
  aster(doc,2);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==notri);
  doc.pl[1].clear();
  aster(doc,3);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==0);
}

void testmaketindouble()
{
  int i;
  doc.pl[1].clear();
  aster(doc,100);
  for (i=1;i<101;i++)
    doc.pl[1].addpoint(i+100,doc.pl[1].points[i],false);
  cout<<"testmaketindouble: ";
  cout.flush();
  i=0;
  try
  {
    doc.pl[1].maketin(psoutput?"double.ps":"",false);
  }
  catch(int e)
  {
    i=e;
  }
  cout<<"maketin threw "<<i<<endl;
}

void testmaketinaster()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  aster(doc,100);
  doc.pl[1].maketin(psoutput?"aster.ps":"",false);
  assert(doc.pl[1].edges.size()==284);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  assert(fabs(totallength-600.689)<0.001);
}

void testmaketinbigaster()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  aster(doc,5972);
  doc.pl[1].maketin(psoutput?"bigaster.ps":"",true);
  //assert(doc.pl[1].edges.size()==284);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  //dumppointsvalence();
  //assert(fabs(totallength-600.689)<0.001);
  /* Flip zones:
   * 34-46 55-67
   * 102-122 136-156
   * 290-323 345-378
   * 801-855 890-944
   * 2166-2254 2310-2398
   * 5784-5927 6017-6160
   * 15328-15390 15561-?
   */
}

void testmaketinstraightrow()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  straightrow(doc,100);
  rotate(doc,30);
  try
  {
    doc.pl[1].maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==flattri);
}

void testmaketinlongandthin()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  longandthin(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"longandthin.ps":"");
  assert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("longandthin %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  assert(fabs(totallength-123.499)<0.001);
}

void testmaketinlozenge()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  lozenge(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"lozenge.ps":"");
  assert(doc.pl[1].edges.size()==299);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("lozenge %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  assert(fabs(totallength-2111.8775)<0.001);
}

void testmaketinring()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  ring(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"ring.ps":"");
  assert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("ring edges total length %f\n",totallength);
  //Don't assert the total length. There are over 10^56 (2^189) right answers to that.
}

void testmaketinellipse()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  ellipse(doc,100);
  doc.pl[1].maketin(psoutput?"ellipse.ps":"");
  assert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("ellipse edges total length %f\n",totallength);
  assert(fabs(totallength-1329.4675)<0.001);
}

void testrelprime()
{
  assert(relprime(987)==610);
  assert(relprime(610)==377);
  assert(relprime(377)==233);
  assert(relprime(233)==144);
  assert(relprime(144)==89);
  assert(relprime(89)==55);
  assert(relprime(55)==34);
  assert(relprime(100000)==61803);
  assert(relprime(100)==61);
  assert(relprime(0)==1);
  assert(relprime(1)==1);
  assert(relprime(2)==1);
  assert(relprime(3)==2);
  assert(relprime(4)==3);
  assert(relprime(5)==3);
  assert(relprime(6)==5);
}

void testmanysum()
{
  manysum ms;
  int i,j,h;
  double x,naiveforwardsum,forwardsum,naivebackwardsum,backwardsum;
  cout<<"manysum"<<endl;
  ms.clear();
  for (naiveforwardsum=i=0;i>-7;i--)
  {
    x=pow(1000,i);
    for (j=0;j<(slowmanysum?1000000:100000);j++)
    {
      naiveforwardsum+=x;
      ms+=x;
    }
  }
  ms.prune();
  forwardsum=ms.total();
  ms.clear();
  for (naivebackwardsum=0,i=-6;i<1;i++)
  {
    x=pow(1000,i);
    for (j=0;j<(slowmanysum?1000000:100000);j++)
    {
      naivebackwardsum+=x;
      ms+=x;
    }
  }
  ms.prune();
  backwardsum=ms.total();
  cout<<ldecimal(naiveforwardsum)<<' '<<ldecimal(forwardsum)<<' '<<ldecimal(naivebackwardsum)<<' '<<ldecimal(backwardsum)<<endl;
  assert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  assert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  assert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  assert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
  ms.clear();
  h=slowmanysum?1:16;
  for (naiveforwardsum=i=0;i>-0x360000;i-=h)
  {
    x=exp(i/65536.);
    naiveforwardsum+=x;
    ms+=x;
  }
  ms.prune();
  forwardsum=ms.total();
  ms.clear();
  for (naivebackwardsum=0,i=-0x35ffff&-h;i<1;i+=h)
  {
    x=exp(i/65536.);
    naivebackwardsum+=x;
    ms+=x;
  }
  ms.prune();
  backwardsum=ms.total();
  cout<<ldecimal(naiveforwardsum)<<' '<<ldecimal(forwardsum)<<' '<<ldecimal(naivebackwardsum)<<' '<<ldecimal(backwardsum)<<endl;
  assert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  assert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  assert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  assert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
}

void testvcurve()
{
  double result,b1,c1,d1a2,b2,c2,epsilon;
  int i;
  vector<double> extrema,xs;
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
  assert(fabs(vlength(0,1,2,3,7.2)-7.8)<1e-6);
  result=vlength(0,10,0,20,200);
  printf("vertical curve length=%f\n",result);
  assert(result>201 && result<204.427);
  extrema=vextrema(0,1,2,3);
  assert(extrema.size()==0);
  extrema=vextrema(0,4,4,0);
  assert(extrema.size()==1);
  printf("0,4,4,0: extrema[0]=%f\n",extrema[0]);
  assert(extrema[0]==0.5);
  extrema=vextrema(2,4,0,2);
  assert(extrema.size()==2);
  printf("2,4,0,2: extrema[0]=%f, extrema[1]=%f\n",extrema[0],extrema[1]);
  assert(fabs(vslope(2,4,0,2,extrema[0]))<0.001);
  extrema=vextrema(0,4,3,0);
  assert(extrema.size()==1);
  printf("0,4,3,0: extrema[0]=%f\n",extrema[0]);
  assert(fabs(vslope(0,4,3,0,extrema[0]))<0.001);
  extrema=vextrema(M_PI,M_PI,M_E,M_E);
  assert(extrema.size()==2);
  printf("π,π,e,e: extrema[0]=%f,%f\n",extrema[0],extrema[1]);
  assert(extrema[0]==0 && extrema[1]==1);
  extrema=vextrema(M_PI,M_PI,M_E,3);
  assert(extrema.size()==2);
  printf("π,π,e,3: extrema[0]=%f,%f\n",extrema[0],extrema[1]);
  assert(extrema[0]==0);
  for (epsilon=1/128.;epsilon>1e-15;epsilon*=0.5)
  {
    extrema=vextrema(0,1-epsilon,1+epsilon,0);
    assert(extrema.size()==1);
    printf("0,1-ε,1+ε,0 ε=%.3e: extrema[0]-0.5=%f*ε\n",epsilon,(extrema[0]-0.5)/epsilon);
    assert(fabs((extrema[0]-0.5)/epsilon-0.25)<0.001);
    xs.clear();
    for (i=-3;i<4;i+=2)
      xs.push_back(vcurve(0,1-epsilon,1+epsilon,0,(1-i*epsilon)/2));
    //printf("vertex=%f*ε deriv3=%f\n",paravertex(xs),deriv3(xs));
    //assert(extrema[0]==0.5);
  }
  vsplit(1,2,2,1,4./8,b1,c1,d1a2,b2,c2);
  assert(vcurve(1,2,2,1,3./8)==vcurve(1,b1,c1,d1a2,3./4));
  assert(vcurve(1,2,2,1,5./8)==vcurve(d1a2,b2,c2,1,1./4));
  vsplit(0,1,2,0,3./8,b1,c1,d1a2,b2,c2);
  assert(vcurve(0,1,2,0,15./64)==vcurve(0,b1,c1,d1a2,5./8));
}

void testsegment()
{
  int i;
  double cept;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta;
  segment a(beg,end),b,c;
  assert(a.length()==500);
  assert(a.chordlength()==500);
  assert(a.chordbearing()==316933406);
  a.setslope(START,0.3+a.avgslope());
  a.setslope(END,-0.1+a.avgslope());
  assert(fabs(a.elev(1)-3.3)<0.05);
  assert(fabs(a.slope(250)+0.042)<0.001);
  cept=a.contourcept(5);
  cout<<"a crosses 5 at "<<cept<<"; a.elev()="<<a.elev(cept)<<endl;
  assert(fabs(a.elev(cept)-5)<1e-6);
  sta=a.station(200);
  assert(sta==xyz(120,160,31));
  assert(std::isinf(a.radius(0)));
  assert(a.curvature(0)==0);
  assert(!isfinite(a.center().east()));
  a.split(200,b,c);
  assert(dist(b.station(123),a.station(123))<0.001);
  assert(dist(c.station(200),a.station(400))<0.001);
  a.setslope(START,0);
  extrema=a.vextrema(true);
  cout<<extrema.size()<<" extrema"<<endl;
  assert(extrema.size()==2);
  extrema=a.vextrema(false);
  assert(extrema.size()==1);
}

void testarc()
{
  int i;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta,sta2;
  xy ctr;
  arc a(beg,end),b,c;
  assert(fabs(a.length()-500)<0.001);
  assert(a.chordlength()==500);
  a.setdelta(degtobin(60));
  assert(fabs(a.length()-523.599)<0.001);
  assert(a.chordlength()==500);
  a.setslope(START,0.3+a.avgslope());
  a.setslope(END,-0.1+a.avgslope());
  //printf("slope(250) %f\n",a.slope(250));
  //printf("slope(261.8) %f\n",a.slope(261.8));
  assert(fabs(a.elev(1)-3.3)<0.05);
  assert(fabs(a.slope(261.8)+0.042)<0.001);
  sta=a.station(200);
  //printf("sta.x=%.17f sta.y=%.17f sta.z=%.17f \n",sta.east(),sta.north(),sta.elev());
  assert(dist(sta,xyz(163.553,112.7825,32.167))<0.001);
  //printf("arc radius %f\n",a.radius(1));
  assert(fabs(a.radius(0)-500)<0.001);
  assert(fabs(a.curvature(0)-0.002)<0.000001);
  //printf("arc center %f,%f\n",a.center().east(),a.center().north());
  ctr=a.center();
  //printf("distance %f\n",dist(xy(sta),ctr));
  assert(fabs(dist(xy(sta),ctr)-500)<0.001);
  assert(fabs(ctr.east()+196.410)<0.001);
  assert(fabs(ctr.north()-459.8075)<0.001);
  a.split(200,b,c);
  sta=a.station(200);
  //printf("a.station %f,%f,%f\n",sta.east(),sta.north(),sta.elev());
  sta=b.station(200);
  printf("b.station %f,%f,%f %f\n",sta.east(),sta.north(),sta.elev(),b.length());
  assert(dist(b.station(123),a.station(123))<0.001);
  assert(dist(c.station(200),a.station(400))<0.001);
  a.setslope(START,0);
  a.setslope(END,0);
  extrema=a.vextrema(true);
  assert(extrema.size()==2);
  for (i=0;i<extrema.size();i++)
    cout<<"extrema["<<i<<"]="<<extrema[i]<<endl;
  assert(extrema[1]>523 && extrema[1]<524);
  extrema=a.vextrema(false);
  assert(extrema.size()==0);
  sta=xyz(150,200,5);
  b=arc(beg,sta,end);
  sta2=b.station(250);
  cout<<"arc3 "<<sta2.elev()<<endl;
  assert(sta2.elev()==5);
  sta=xyz(150,200,10);
  b=arc(beg,sta,end);
  sta2=b.station(250);
  cout<<"arc3 "<<sta2.elev()<<endl;
  assert(sta2.elev()==10);
  sta=xyz(200,150,10);
  b=arc(beg,sta,end);
  sta2=b.station(252.905);
  cout<<"arc3 "<<sta2.east()<<' '<<sta2.north()<<' '<<sta2.elev()<<endl;
  //cout<<dist(sta,sta2)<<endl;
  assert(dist(sta,sta2)<0.001);
}

void testspiralarc()
{
  int i;
  double bear[3],len;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta;
  xy ctr;
  spiralarc a(beg,end),b,c;
  assert(fabs(a.length()-500)<0.001);
  assert(a.chordlength()==500);
  a._setdelta(degtobin(60),degtobin(60));
  cout<<"chord bearing "<<bintodeg(a.chordbearing())<<endl;
  cout<<"bearing at beg "<<(bear[0]=bintodeg(a.bearing(0)))<<endl;
  cout<<"bearing at mid "<<(bear[1]=bintodeg(a.bearing(250)))<<endl;
  cout<<"bearing at end "<<(bear[2]=bintodeg(a.bearing(500)))<<endl;
  cout<<"delta "<<bear[2]-bear[0]<<" skew "<<bear[0]+bear[2]-2*bear[1]<<endl;
  a._fixends(1);
  cout<<"new length "<<(len=a.length())<<endl;
  cout<<"bearing at beg "<<(bear[0]=bintodeg(a.bearing(0)))<<endl;
  cout<<"bearing at mid "<<(bear[1]=bintodeg(a.bearing(len/2)))<<endl;
  cout<<"bearing at end "<<(bear[2]=bintodeg(a.bearing(len)))<<endl;
  cout<<"delta "<<bear[2]-bear[0]<<" skew "<<bear[0]+bear[2]-2*bear[1]<<endl;
  a.setdelta(0,degtobin(254)); // this barely fails
  a.setdelta(degtobin(26),degtobin(8));
  cout<<"new length "<<(len=a.length())<<endl;
  cout<<"bearing at beg "<<(bear[0]=bintodeg(a.bearing(0)))<<endl;
  cout<<"bearing at mid "<<(bear[1]=bintodeg(a.bearing(len/2)))<<endl;
  cout<<"bearing at end "<<(bear[2]=bintodeg(a.bearing(len)))<<endl;
  cout<<"delta "<<bear[2]-bear[0]<<" skew "<<bear[0]+bear[2]-2*bear[1]<<endl;
  assert(fabs(bear[2]-bear[0]-26)<1e-5);
  assert(fabs(bear[0]+bear[2]-2*bintodeg(a.chordbearing())-8)<1e-5);
  cout<<"curvature at beg "<<a.curvature(0)<<endl;
  cout<<"curvature at end "<<a.curvature(len)<<endl;
  a.split(200,b,c);
  sta=a.station(123);
  printf("a.station %f,%f,%f\n",sta.east(),sta.north(),sta.elev());
  sta=b.station(123);
  printf("b.station %f,%f,%f %f\n",sta.east(),sta.north(),sta.elev(),b.length());
  assert(dist(b.station(123),a.station(123))<0.001);
  assert(dist(c.station(200),a.station(400))<0.001);
}

void spiralmicroscope(spiralarc a,double aalong,spiralarc b,double balong)
{
  int i,alim,blim;
  xy point;
  double apow2,bpow2,ainc,binc;
  double minx=INFINITY,miny=INFINITY,maxx=-INFINITY,maxy=-INFINITY;
  vector<xy> apoints,bpoints;
  frexp(aalong,&i);
  apow2=ldexp(0.5,i);
  frexp(balong,&i);
  bpow2=ldexp(0.5,i);
  if (abs(apow2)<abs(bpow2))
  {
    blim=128;
    alim=128*bpow2/apow2;
  }
  else
  {
    alim=128;
    blim=128*apow2/bpow2;
  }
  ainc=apow2*DBL_EPSILON;
  binc=bpow2*DBL_EPSILON;
  for (i=-alim;i<=alim;i++)
  {
    point=(a.station(aalong+ainc*i));
    apoints.push_back(point);
    if (point.getx()<minx)
      minx=point.getx();
    if (point.gety()<miny)
      miny=point.gety();
    if (point.getx()>maxx)
      maxx=point.getx();
    if (point.gety()>maxy)
      maxy=point.gety();
  }
  for (i=-blim;i<=blim;i++)
  {
    point=(b.station(balong+binc*i));
    bpoints.push_back(point);
    if (point.getx()<minx)
      minx=point.getx();
    if (point.gety()<miny)
      miny=point.gety();
    if (point.getx()>maxx)
      maxx=point.getx();
    if (point.gety()>maxy)
      maxy=point.gety();
  }
  psopen("spiralmicro.ps");
  psprolog();
  startpage();
  setscale(minx,miny,maxx,maxy,0);
  setcolor(1,0,0);
  for (i=0;i<apoints.size();i++)
    dot(apoints[i]);
  setcolor(0,0,1);
  for (i=0;i<bpoints.size();i++)
    dot(bpoints[i]);
  endpage();
  startpage();
  setscale(minx,miny,maxx,maxy,0);
  setcolor(0,0,1);
  for (i=0;i<apoints.size();i++)
  {
    if (i==apoints.size()/2)
      setcolor(0,0.6,0);
    dot(apoints[i]);
    if (i==apoints.size()/2)
      setcolor(0,0,1);
  }
  endpage();
  startpage();
  setscale(minx,miny,maxx,maxy,0);
  setcolor(0,0,1);
  for (i=0;i<bpoints.size();i++)
  {
    if (i==bpoints.size()/2)
      setcolor(0,0.6,0);
    dot(bpoints[i]);
    if (i==bpoints.size()/2)
      setcolor(0,0,1);
  }
  endpage();
  pstrailer();
  psclose();
}

void testcogospiral()
{
  int i;
  xyz beg0(-1193,-489,0),end0(0xc07,0x50b,0), // slope 5/12
      beg1(-722,983,0),end1(382,-489,0), // slope -4/3
      beg2(-101,1,0),end2(99,1,0),beg3(-99,-1,0),end3(101,-1,0),
      beg4(-5,0,0),end4(5,0,0),beg5(-4,3,0),end5(4,3,0);
  segment a(beg0,end0),b(beg1,end1);
  spiralarc c(beg2,end2),d(beg3,end3),e(beg4,end4),f(beg5,end5);
  c.setdelta(0,DEG30);
  d.setdelta(0,-DEG30);
  e.setdelta(-DEG60,0);
  f.setdelta(DEG90,0); // e and f are 0.034 away from touching
  xy intpoint; // (7,11)
  vector<alosta> intlist;
  intlist=intersection1(a,0,a.length(),b,0,b.length(),false);
  /* The distance along both lines to the intersection point is exactly an integer,
   * so the two points are exactly equal to (7,11).
   */
  cout<<"testcogospiral: "<<intlist.size()<<" alostas"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlist.size();i++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlist[i].along<<' '<<ldecimal(intlist[i].station.east())<<' '<<ldecimal(intlist[i].station.north())<<endl;
    intpoint+=intlist[i].station;
  }
  intpoint/=i;
  assert(dist(intpoint,xy(7,11))<1e-5);
  intlist=intersection1(spiralarc(a),0,a.length(),spiralarc(b),0,b.length(),false);
  /* The distances along the curves are in the hundreds, but the midpoints
   * are closer to the origin, so the two intersection points do not exactly
   * coincide. They aren't exactly (7,11) either, because the bearings have
   * been rounded to the nearest 1657th of a second.
   */
  cout<<"testcogospiral: "<<intlist.size()<<" alostas"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlist.size();i++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlist[i].along<<' '<<ldecimal(intlist[i].station.east())<<' '<<ldecimal(intlist[i].station.north())<<endl;
    intpoint+=intlist[i].station;
  }
  intpoint/=i;
  assert(dist(intpoint,xy(7,11))<1e-5);
  spiralmicroscope(a,intlist[0].along,b,intlist[1].along);
  intlist=intersection1(c,50,c.length()-50,d,50,c.length()-50,false);
  cout<<"testcogospiral: "<<intlist.size()<<" alostas"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlist.size();i++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlist[i].along<<' '<<ldecimal(intlist[i].station.east())<<' '<<ldecimal(intlist[i].station.north())<<endl;
    intpoint+=intlist[i].station;
  }
  intpoint/=i;
  intlist=intersection1(e,0,1,f,0,1,false);
  cout<<"testcogospiral: "<<intlist.size()<<" alostas"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlist.size();i++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlist[i].along<<' '<<ldecimal(intlist[i].station.east())<<' '<<ldecimal(intlist[i].station.north())<<endl;
    intpoint+=intlist[i].station;
  }
  intpoint/=i;
}

void testclosest()
{
  xyz beg(-30,0,0),end(30,0,0);
  spiralarc a(beg,end);
  int d1[]={0,DEG60,170891318,0,85445658};
  int d2[]={0,0,0,DEG60,28512070};
  /* 170891318 is 0.5 radian rounded down. 0.5 radian is 170891318.9.
   * The circular arc is started in closest with 3 points if its delta
   * is less than 0.5 radian. From 0.5 to 1.5 radians it is started
   * with 4 points.
   * 
   * 0: straight line
   * 1: 60° arc
   * 2: 0.5 radian arc, the most an arc can bend and still start with 3 points
   * 3: 60° spiralarc
   * 4: The most a spiralarc straight at one end can bend and still start with 3 points
   */
  int i,j,ang;
  bool showinaccurate=false;
  double close,close15,minquick,d,d15;
  doc.pl[1].clear();
  aster(doc,1000);
  psopen("closest.ps");
  psprolog();
  for (i=0;i<5;i++)
  {
    a.setdelta(d1[i],d2[i]);
    startpage();
    minquick=INFINITY;
    /* minquick is the minimum distance that it calculates quickly and inaccurately.
     * It should be greater than closesofar, which is 15.
     */
    setscale(-32,-32,32,32,0);
    cout<<"Curvature*length at start "<<ldecimal(a.curvature(0)*a.length())<<", at end "<<ldecimal(a.curvature(a.length())*a.length())<<endl;
    for (j=1;j<=1000;j++)
    {
      close=a.closest(doc.pl[1].points[j],INFINITY);
      close15=a.closest(doc.pl[1].points[j],15);
      if (close!=close15)
      {
	d=dist(doc.pl[1].points[j],a.station(close));
	d15=dist(doc.pl[1].points[j],a.station(close15));
	if (minquick>d)
	  minquick=d;
      }
      if (showinaccurate)
	close=close15;
      if (isfinite(close))
      {
	ang=a.bearing(close)-atan2i(doc.pl[1].points[j]-a.station(close));
	ang=(ang&(DEG180-1))-DEG90;
	if (abs(ang)>10)
	  if (close==0 || close==a.length())
	    setcolor(0,0,1);
	  else
	    setcolor(1,0,0);
	else
	  setcolor(0,0,0);
	line2p(doc.pl[1].points[j],a.station(close));
      }
    }
    endpage();
    cout<<"Minimum distance that is calculated inaccurately is "<<minquick<<endl;
    assert(minquick>15);
  }
  pstrailer();
  psclose();
}

void testspiral()
{
  xy a,b,c,limitpoint;
  int i,j,bearing,lastbearing,curvebearing,diff,badcount;
  double t;
  vector<xy> spoints;
  a=cornu(0);
  assert(a==xy(0,0));
  psopen("spiral.ps");
  psprolog();
  startpage();
  setscale(-1,-1,1,1,degtobin(0));
  //widen(10);
  for (i=-120;i<121;i++)
  {
    b=cornu(t=i/20.);
    if (i*i==14400)
    {
      limitpoint=b;
      dot(b);
    }
    else
      if (i>-119)
	line2p(c,b);
    //printf("spiral %f %f,%f %f\n",t,b.east(),b.north(),1/sqr(dist(b,limitpoint)));
    if (i>=0)
      spoints.push_back(b);
    c=b;
  }
  for (i=1,badcount=0;i<119;i++)
  {
    curvebearing=ispiralbearing(i/20.,0,2);
    bearing=dir(spoints[i-1],spoints[i+1]); // compute the difference between a chord of the spiral
    diff=(curvebearing-bearing)&0x7fffffff; // and a tangent in the middle of the arc
    diff|=(diff&0x40000000)<<1; // diff could be near 0° or 360°; this bit manipulation puts it near 0°
    //printf("%3d diff=%d (%f')\n",i,diff,bintomin(diff));
    badcount+=(diff<=-300000 || diff>=-250000); // diff is between -3'00" and -2'30" when the increment is 1/20
  }
  /* On i386 (Pentium), the last 12 bearings are off by up to 2°
   * On x86_64 (both Intel Core and Intel Atom), they are all accurate.
   * This is NOT explained by sizeof(long double), which is 12 bytes on i386
   * and 16 bytes on x86_64; only 10 bytes are stored, and the rest is wasted.
   * When computing cornu(-6), the fifth step is facpower*=36/5,
   * where facpower is -419904 before and -3023308.8 after.
   * On x86_64, facpower is 0xc014b887333333333333.
   * On i386,   facpower is 0xc014b887333333333800.
   * I checked this with 64-bit Linux, 64-bit DragonFly, and 32-bit DragonFly;
   * it depends on the processor, not the operating system.
   * The ARM on a Raspberry Pi does not have a distinct long double type
   * and fails this test. The Pi is not suitable to run this program.
   * Running under Valgrind, the program says there are 10 bad bearings.
   */
  printf("%d bad bearings out of 118\n",badcount);
  assert(badcount<=13);
  for (bearing=i=0,lastbearing=1;i<100 && bearing!=lastbearing;i++)
  {
    t=bintorad(bearing);
    a=cornu(-sqrt(t));
    b=cornu(sqrt(t+M_PI/2));
    lastbearing=bearing;
    bearing=dir(a,b);
  }
  setcolor(0,0,1);
  line2p(a,b);
  endpage();
  //b=cornu(1,1,1);
  startpage();
  for (j=-3;j<=3;j++)
  {
    switch ((j+99)%3)
    {
      case 0:
	setcolor(1,0,0);
	break;
      case 1:
	setcolor(0,0.4,0);
	break;
      case 2:
	setcolor(0,0,1);
	break;
    }
    for (i=-20;i<21;i++)
    {
      b=cornu(t=i/20.,2*j,2);
      if (i>-20)
      {
	line2p(c,b);
	assert(dist(c,b)>0.049 && dist(c,b)<=0.05);
	//cout<<dist(c,b)<<' ';
      }
      c=b;
    }
    //cout<<endl;
  }
  endpage();
  //startpage();
  a=cornu(sqrt(M_PI*2));
  for (i=-20;i<21;i++)
  {
    b=cornu(i/20.,0,2);
    c=cornu(i/20.);
    //cout<<i<<' '<<dist(b,c)<<endl;
    assert(dist(b,c)<1e-12); // it's less than 6e-17 on 64-bit
    b=cornu(i/20.,sqrt(M_PI*8),2);
    c=cornu(i/20.+sqrt(M_PI*2))-a;
    //cout<<i<<' '<<dist(b,c)<<endl;
    assert(dist(b,c)<1e-12); // it's less than 1.1e-15 on 64-bit
    //line2p(b,c);
    b=cornu(i/20.,1,0);
    c=xy(0,1);
    //cout<<i<<' '<<dist(b,c)-1<<endl;
    assert(fabs(dist(b,c)-1)<1e-12); // it's 0 or -1.11e-16 on 64-bit
  }
  //endpage();
  pstrailer();
  psclose();
  assert(bearing==162105696);
  printf("Maximum useful t of spiral is %f\n",sqrt(t+M_PI/2));
}

void testarea3()
{
  int i,j,itype;
  xy a(0,0),b(4,0),c(0,3),d(4,4),e;
  assert(area3(c,a,b)==6);
}

void testtriangle()
{
  int i;
  triangle tri;
  double elev,elevd,elevg,eleva;
  xy o(0,0),a(1,0),d(-0.5,0),g(0.5,0);
  doc.pl[0].clear();
  // Start with an identically 0 surface. The elevation at the center should be 0.
  doc.pl[0].addpoint(1,point(1,0,0,"eip"));
  doc.pl[0].addpoint(1,point(-0.5,M_SQRT_3_4,0,"eip"));
  doc.pl[0].addpoint(1,point(-0.5,-M_SQRT_3_4,0,"eip"));
  tri.a=&doc.pl[0].points[1];
  tri.b=&doc.pl[0].points[2];
  tri.c=&doc.pl[0].points[3];
  tri.flatten(); // sets area, needed for computing elevations
  for (i=0;i<7;i++)
    tri.ctrl[i]=0;
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  assert(elev==0);
  // Now make a constant surface at elevation 1.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(1);
  doc.pl[0].points[3].setelev(1);
  for (i=0;i<7;i++)
    tri.ctrl[i]=1;
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  assert(elev==1);
  // Now make a linear surface.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(0);
  doc.pl[0].points[3].setelev(0);
  tri.ctrl[0]=tri.ctrl[1]=2/3.;
  tri.ctrl[2]=tri.ctrl[4]=1/3.;
  tri.ctrl[5]=tri.ctrl[6]=0;
  tri.setcentercp();
  elev=tri.elevation(o);
  printf("ctrl[3]=%f elevation=%f\n",tri.ctrl[3],elev);
  assert(abs(elev*3-1)<1e-7);
  // Now make a quadratic surface. It is a paraboloid z=r². Check that the cubic component is 0.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(1);
  doc.pl[0].points[3].setelev(1);
  for (i=0;i<7;i++)
    tri.ctrl[i]=0;
  tri.setcentercp();
  elev=tri.elevation(o);
  elevd=tri.elevation(d);
  elevg=tri.elevation(g);
  eleva=tri.elevation(a);
  printf("ctrl[3]=%f elevation=%f %f %f %f\n",tri.ctrl[3],elevd,elev,elevg,eleva);
  assert(abs(elevd-elev*3+elevg*3-eleva)<1e-7);
  // Now turn the quadratic surface upside-down, using setgradient.
  doc.pl[0].points[1].setelev(0);
  doc.pl[0].points[2].setelev(0);
  doc.pl[0].points[3].setelev(0);
  tri.setgradient(doc.pl[0].points[1],(xy)doc.pl[0].points[1]*-2);
  tri.setgradient(doc.pl[0].points[2],(xy)doc.pl[0].points[2]*-2);
  tri.setgradient(doc.pl[0].points[3],(xy)doc.pl[0].points[3]*-2);
  tri.setcentercp();
  elev=tri.elevation(o);
  elevd=tri.elevation(d);
  elevg=tri.elevation(g);
  eleva=tri.elevation(a);
  printf("ctrl[3]=%f elevation=%f %f %f %f\n",tri.ctrl[3],elevd,elev,elevg,eleva);
  assert(abs(elevd-elev*3+elevg*3-eleva)<1e-7);
  assert(abs(elev-1)<1e-7);
}

void testqindex()
{
  qindex qinx;
  int i,qs,ntri;
  triangle *ptri;
  vector<xy> plist;
  double pathlength;
  vector<qindex*> hilbertpath;
  xy offset(16,8),bone1(3,4),bone2(-3,-4),bone3(49,-64);
  doc.pl[1].clear();
  plist.clear();
  doc.pl[1].addpoint(1,point(0.3,0.3,0,""));
  doc.pl[1].addpoint(1,point(0.6,0.8,0,""));
  doc.pl[1].addpoint(1,point(0.8,0.6,0,""));
  for (i=0;i<3;i++)
    plist.push_back(doc.pl[1].points[i+1]+offset);
  qinx.sizefit(plist);
  printf("side=%f x=%f y=%f\n",qinx.side,qinx.x,qinx.y);
  assert(qinx.side==1);
  doc.pl[1].maketin();
  doc.pl[1].maketriangles();
  printf("%d triangle, should be 1\n",(int)doc.pl[1].triangles.size());
  assert(doc.pl[1].triangles.size()==1);
  qinx.clear();
  doc.pl[1].clear();
  plist.clear();
  psopen("qindex.ps");
  psprolog();
  startpage();
  setscale(1,-7,31,23);
  aster(doc,100);
  doc.pl[1].maketin();
  enlarge(doc,pow(2,(rng.usrandom()-32767.5)/65536));
  for (i=0;i<100;i++)
  {
    dot(doc.pl[1].points[i+1]+offset);
    plist.push_back(doc.pl[1].points[i+1]+offset);
  }
  qinx.sizefit(plist);
  qinx.split(plist);
  printf("%d nodes, ",qs=qinx.size());
  qs--;
  qs=qs*3/4; // convert to number of leaves of the tree (undivided squares in the drawing)
  qs++;
  printf("%d leaves\n",qs);
  assert(qs>=79 && qs<=133);
  qinx.draw();
  endpage();
  startpage();
  hilbertpath=qinx.traverse();
  assert(hilbertpath.size()==qs);
  setscale(1,-7,31,23);
  qinx.draw();
  setcolor(0,0,1);
  for (i=1,pathlength=0;i<hilbertpath.size();i++)
  {
    line2p(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
    pathlength+=dist(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
  }
  printf("pathlength %f\n",pathlength);
  assert(pathlength>100 && pathlength<400);
  endpage();
  startpage();
  setscale(-15,-15,15,15);
  doc.pl[1].maketin();
  doc.pl[1].maketriangles();
  for (i=ntri=0;i<doc.pl[1].edges.size();i++)
  {
    ntri+=doc.pl[1].edges[i].tria!=NULL;
    ntri+=doc.pl[1].edges[i].trib!=NULL;
    line(doc,doc.pl[1].edges[i],i,false,true);
    setcolor(0.6,0.4,0);
    if (doc.pl[1].edges[i].tria)
      line2p(doc.pl[1].edges[i].midpoint(),doc.pl[1].edges[i].tria->centroid());
    setcolor(0,0.4,0.6);
    if (doc.pl[1].edges[i].trib)
      line2p(doc.pl[1].edges[i].midpoint(),doc.pl[1].edges[i].trib->centroid());
  }
  printf("%d edges ntri=%d\n",i,ntri);
  assert(ntri>i/2);
  setcolor(1,0,0);
  for (i=0;i<doc.pl[1].triangles.size();i++)
  {
    assert(doc.pl[1].triangles[i].area()>0);
    //printf("tri %d area %f\n",i,doc.pl[1].triangles[i].area());
    dot(doc.pl[1].triangles[i].centroid());
  }
  printf("%d triangles\n",i);
  assert(ntri==i*3); // ntri is the number of sides of edges which are linked to a triangle
  endpage();
  ptri=&doc.pl[1].triangles[0];
  ptri=ptri->findt(bone1);
  assert(ptri->in(bone1));
  assert(!ptri->in(bone2));
  ptri=ptri->findt(bone2);
  assert(ptri->in(bone2));
  assert(!ptri->in(bone1));
  assert(ptri->findt(bone3,true));
  assert(!ptri->findt(bone3,false));
  startpage();
  setscale(-15,-15,15,15);
  plist.clear();
  for (i=0;i<100;i++)
    plist.push_back(doc.pl[1].points[i+1]);
  qinx.sizefit(plist);
  qinx.split(plist);
  qinx.draw();
  qinx.settri(&doc.pl[1].triangles[0]);
  for (i=ntri=0;i<doc.pl[1].edges.size();i++)
    line(doc,doc.pl[1].edges[i],i,false);
  setcolor(1,0,0);
  hilbertpath=qinx.traverse();
  for (i=pathlength=0;i<hilbertpath.size();i++)
  {
    line2p(hilbertpath[i]->tri->centroid(),hilbertpath[i]->middle());
    pathlength+=dist(hilbertpath[i]->tri->centroid(),hilbertpath[i]->middle());
  }
  printf("settri: pathlength=%f\n",pathlength);
  assert(pathlength>50 && pathlength<250);
  endpage();
  ptri=qinx.findt(bone1);
  assert(ptri->in(bone1));
  assert(!ptri->in(bone2));
  ptri=qinx.findt(bone2);
  assert(ptri->in(bone2));
  assert(!ptri->in(bone1));
  assert(qinx.findt(bone3,true));
  assert(!qinx.findt(bone3,false));
  pstrailer();
  psclose();
}

void drawgrad(double scale)
{
  ptlist::iterator i;
  for (i=doc.pl[1].points.begin();i!=doc.pl[1].points.end();i++)
  {
    setcolor(0,1,0);
    line2p(i->second,xy(i->second)+testsurfacegrad(i->second)*scale);
    setcolor(0,0,0);
    dot(i->second);
    line2p(i->second,xy(i->second)+i->second.gradient*scale);
  }
}

void checkgrad(double &avgerror,double &maxerror)
{
  ptlist::iterator i;
  double error;
  int n;
  avgerror=maxerror=0;
  for (n=0,i=doc.pl[1].points.begin();i!=doc.pl[1].points.end();i++,n++)
  {
    error=dist(i->second.gradient,testsurfacegrad(i->second));
    avgerror+=error*error;
    if (error>maxerror)
      maxerror=error;
  }
  avgerror=sqrt(error/n);
}

void testmakegrad()
{
  double avgerror,maxerror,corr;
  xy grad63,grad63half;
  doc.pl[1].clear();
  setsurface(HYPAR);
  aster(doc,100);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.15);
  grad63=doc.pl[1].points[63].gradient;
  enlarge(doc,2); // affects x and y but not z, so gradient should be halved
  doc.pl[1].makegrad(0.15);
  grad63half=doc.pl[1].points[63].gradient;
  printf("grad63 %f %f grad63half %f %f\n",grad63.east(),grad63.north(),grad63half.east(),grad63half.north());
  assert(grad63==grad63half*2);
  enlarge(doc,0.5);
  psopen("gradient.ps");
  psprolog();
  for (corr=0;corr<=1;corr+=0.1)
  {
    startpage();
    doc.pl[1].makegrad(corr);
    checkgrad(avgerror,maxerror);
    printf("testmakegrad: corr=%f avgerror=%f maxerror=%f\n",corr,avgerror,maxerror);
    drawgrad(3);
    endpage();
  }
  pstrailer();
  psclose();
}

void testrasterdraw()
{
  doc.pl[1].clear();
  setsurface(HYPAR);
  aster(doc,100);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"raster.ppm");
  doc.pl[1].setgradient(true);
  rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"rasterflat.ppm");
  testpointedg();
}

void test1tri(string triname,int excrits)
{
  vector<double> xs,lh;
  vector<xyz> slice;
  vector<xy> crits;
  int i,j,side,cubedir,ptype,size0,size1,size2;
  double vertex,offset,arearatio;
  string fname,tfname,psfname;
  fstream ofile;
  fname=triname+".ppm";
  tfname=triname+".txt";
  psfname=triname+".ps";
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  rasterdraw(doc.pl[1],xy(5,0),30,40,30,0,1,fname);
  ofile.open(tfname.c_str(),ios_base::out);
  psopen(psfname.c_str());
  psprolog();
  startpage();
  setscale(-17,-17,17,17);
  for (j=0;j<doc.pl[1].edges.size();j++)
    line(doc,doc.pl[1].edges[j],j,false);
  cubedir=doc.pl[1].triangles[0].findnocubedir();
  ofile<<"Zero cube dir "<<cubedir<<' '<<bintodeg(cubedir)<<"°"<<endl;
  ofile<<"Zero quad offset "<<doc.pl[1].triangles[0].flatoffset()<<endl;
  for (j=30;j>=-30;j--)
  {
    offset=j/20.;
    xs=doc.pl[1].triangles[0].xsect(cubedir,offset);
    vertex=paravertex(xs);
    ofile<<fixed<<setprecision(3)<<setw(7)<<offset<<' '<<setw(7)<<deriv3(xs)<<' '<<setw(7)<<vertex;
    if (vertex<=1.5 && vertex>=-1.5)
      ofile<<string(rint((vertex+1.5)*20),' ')<<'*';
    ofile<<endl;
  }
  line2p(doc.pl[1].triangles[0].spcoord(1.5,-1.5),doc.pl[1].triangles[0].spcoord(-1.5,-1.5));
  line2p(doc.pl[1].triangles[0].spcoord(-1.5,-1.5),doc.pl[1].triangles[0].spcoord(-1.5,1.5));
  for (side=0;side<2;side++)
  {
    ofile<<"Side "<<side<<endl;
    slice=doc.pl[1].triangles[0].slices(side);
    for (j=0;j<slice.size();j++)
    {
      ofile<<fixed<<setprecision(3)<<setw(7)<<slice[j].east()<<setw(7)<<
      slice[j].north()<<setw(7)<<slice[j].elev()<<endl;
      if (j>0)
      {
	if (slice[j-1].elev()>slice[j].elev())
	  setcolor(0,.7,0);
	else
	  setcolor(1,0,1);
	line2p(doc.pl[1].triangles[0].spcoord(slice[j-1].east(),slice[j-1].north()),
		doc.pl[1].triangles[0].spcoord(slice[j].east(),slice[j].north()));
      }
    }
    crits=doc.pl[1].triangles[0].criticalpts_side(side);
    for (j=0;j<crits.size();j++)
    {
      ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
      dot(crits[j]);
    }
  }
  crits=doc.pl[1].triangles[0].criticalpts_axis();
  ofile<<endl;
  for (j=0;j<crits.size();j++)
  {
    ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
    dot(crits[j]);
  }
  doc.pl[1].triangles[0].findcriticalpts();
  crits=doc.pl[1].triangles[0].critpoints;
  for (j=0;j<crits.size();j++)
  {
    ptype=doc.pl[1].triangles[0].pointtype(crits[j]);
    cout<<crits[j].east()<<','<<crits[j].north()<<" type="<<ptype<<endl;
    ofile<<crits[j].east()<<','<<crits[j].north()<<" type="<<ptype<<endl;
    assert(ptype!=PT_SLOPE && ptype!=PT_GRASS);
  }
  for (j=0;j<3;j++)
  {
    doc.pl[1].edges[j].findextrema();
    for (i=0;i<2;i++)
      if (isfinite(doc.pl[1].edges[j].extrema[i]))
	dot(doc.pl[1].edges[j].critpoint(i));
  }
  doc.pl[1].triangles[0].subdivide();
  size0=doc.pl[1].triangles[0].subdiv.size();
  doc.pl[1].triangles[0].addperimeter();
  size1=doc.pl[1].triangles[0].subdiv.size();
  /*for (j=0;j<doc.pl[1].triangles[0].subdiv.size();j++)
  {
    cout<<j<<"L: "<<doc.pl[1].triangles[0].proceed(j,0)<<endl;
    cout<<j<<"R: "<<doc.pl[1].triangles[0].proceed(j+65536,0)<<endl;
  }*/
  doc.pl[1].triangles[0].removeperimeter();
  size2=doc.pl[1].triangles[0].subdiv.size();
  assert(size0==size2);
  cout<<size1-size0<<" monotonic segments in perimeter"<<endl;
  lh=doc.pl[1].triangles[0].lohi();
  cout<<"lohi: "<<setprecision(7)<<lh[0]<<' '<<lh[1]<<' '<<lh[2]<<' '<<lh[3]<<endl;
  for (j=0;j<doc.pl[1].triangles[0].subdiv.size();j++)
    spline(doc.pl[1].triangles[0].subdiv[j].approx3d(1));
  endpage();
  psclose();
  cout<<fname<<endl;
  if (crits.size()!=excrits && excrits>=0)
    cout<<crits.size()<<" critical points found, "<<excrits<<" expected"<<endl;
  assert(crits.size()==excrits || excrits<0);
  testpointedg();
}

void test1grad()
{
  xyz grad3;
  xy pt,grad2;
  map<int,triangle>::iterator i;
  int j;
  vector<double> xsect,ysect;
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  for (i=doc.pl[1].triangles.begin();i!=doc.pl[1].triangles.end();i++)
  {
    pt=(*i->second.a+*i->second.b*2+*i->second.c*3)/6;
    i->second.setgradmat();
    grad3=i->second.gradient3(pt);
    grad2=i->second.gradient(pt);
    //cout<<grad3.east()<<' '<<grad3.north()<<' '<<grad3.elev()<<endl;
    cout<<"Computed gradient: "<<grad2.east()<<','<<grad2.north()<<' ';
    xsect.clear();
    ysect.clear();
    for (j=-3;j<4;j+=2)
    {
      xsect.push_back(i->second.elevation(pt+xy(j*0.5,0)));
      ysect.push_back(i->second.elevation(pt+xy(0,j*0.5)));
    }
    cout<<"Actual gradient: "<<deriv1(xsect)<<','<<deriv1(ysect)<<endl;
    assert(dist(xy(deriv1(xsect),deriv1(ysect)),grad2)<1e-6);
  }
}

void trianglecontours()
/* Pick elevations and gradients at the corners at random and draw color maps
 * of the elevations, to see what combinations of min, max, and saddle can arise.
 * trid306edc96e91f8adcc has three critical points inside the triangle:
 * 7.95533,14.1356,-1.270 minimum
 * -7.83778,-12.9386,1.227 maximum
 * 10.9596,-3.169,0.403 saddle.
 * tria73ddcd27198f6a1bc also has three critical points inside the triangle.
 * tri93db9159a54b09a0fc has two critical points in the triangle, both saddles,
 * two critical points on two sides, and one critical point on the third side.
 * tri45653a366286ff0747 also has two saddles in the triangle.
 * tri083194be44b7dace28 has a contour that does not touch the perimeter and
 * a contour at the same elevation that does.
 * So do tri2c7ae0064f1de9f5b2 and trie1fbf94c3a1f855e9c.
 * 
 * Hypothesis to attack the minimax problem:
 * Every surface in a Bézier triangle can be stretched linearly and rotated
 * into the form ax³+bx²+cx+d+(ex+f)y². That is, there is an axis A such that
 * all lines parallel to A are parabolas (or straight lines), whose vertices
 * lie on a curve B, which is a hyperbola (possibly degenerate) asymptotic to A.
 * All of the up to four extrema lie on curve B, two on one side of A and two
 * on the other. To find them, split from -1.5 to A in geometric progression,
 * and from 1.5 to A in geometric progression, and look for maxima and minima
 * of the height of vertices on each side.
 */
{
  int i,j;
  unsigned char bytes[9]=
  {0xd3,0x06,0xed,0xc9,0x6e,0x91,0xf8,0xad,0xcc};
  string fname;
  xy tilt;
  doc.pl[1].clear();
  regpolygon(doc,3);
  enlarge(doc,10);
  doc.pl[1].maketin();
  fname="tri";
  for (j=0;j<9;j++)
  {
    bytes[j]=rng.ucrandom();
    fname+=hexdig[bytes[j]>>4];
    fname+=hexdig[bytes[j]&15];
  }
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev((bytes[j]-127.5)/100);
    doc.pl[1].points[j+1].gradient=xy((bytes[j+3]-127.5)/1000,(bytes[j+6]-127.5)/1000);
  }
  test1tri(fname,-1);
  fname=fname.substr(0,9)+"flat";
  doc.pl[1].setgradient(true);
  for (j=0;j<3;j++)
    doc.pl[1].points[j+1].gradient=xy(doc.pl[1].elevation(xy(1,0))-doc.pl[1].elevation(xy(0,0)),doc.pl[1].elevation(xy(0,1))-doc.pl[1].elevation(xy(0,0)));
  test1tri(fname,0);
  fname="monkeysaddle";
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(0);
    doc.pl[1].points[j+1].gradient=turn90(xy(doc.pl[1].points[j+1]))/10;
  }
  test1tri(fname,1);
  fname="tilted";
  tilt=(xy(doc.pl[1].points[1])-xy(doc.pl[1].points[3]))/1000;
  /* This triangle has two saddle points on a horizontal line in the no-cube direction.
   * The triangle has three equally spaced nocubedirs; the program picks the one
   * passing through points[2].
   */
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(dot(tilt,xy(doc.pl[1].points[j+1])));
    doc.pl[1].points[j+1].gradient=turn90(xy(doc.pl[1].points[j+1]))/10+tilt;
  }
  test1tri(fname,2);
  fname="tolted";
  tilt=(xy(doc.pl[1].points[3])-xy(doc.pl[1].points[2]))/1000;
  /* This is the same as above, except for tilting in a different direction
   * perpendicular to a different no-cube direction.
   */
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(dot(tilt,xy(doc.pl[1].points[j+1])));
    doc.pl[1].points[j+1].gradient=turn90(xy(doc.pl[1].points[j+1]))/10+tilt;
  }
  test1tri(fname,2);
  fname="slope";
  tilt=xy(0.03,0);
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(dot(tilt,xy(doc.pl[1].points[j+1])));
    doc.pl[1].points[j+1].gradient=tilt;
  }
  test1tri(fname,0);
  fname="table";
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(M_PI);
    doc.pl[1].points[j+1].gradient=xy(0,0);
  }
  test1tri(fname,0);
  i=doc.pl[1].triangles[0].pointtype(xy(0,0));
  fname="floor";
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(0);
    doc.pl[1].points[j+1].gradient=xy(0,0);
  }
  test1tri(fname,0);
}

void testgrad()
{
  int j;
  doc.pl[1].clear();
  setsurface(FLATSLOPE);
  regpolygon(doc,3);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0);
  cout<<"testgrad"<<endl;
  test1grad();
  for (j=0;j<3;j++)
  {
    doc.pl[1].points[j+1].setelev(M_PI);
    doc.pl[1].points[j+1].gradient=xy(0,0);
  }
  test1grad();
  doc.pl[1].clear();
  setsurface(HYPAR);
  aster(doc,20);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0);
  test1grad();
}

#ifndef NDEBUG

void testparabinter()
{
  double y0=-0.9,y1=1.1,z0,z1;
  testfunc func(1,0,-1,0);
  z0=func(y0);
  z1=func(y1);
  cout<<z0<<' '<<func((y0+y1)/2)<<' '<<z1<<endl;
  parabinter(func,y0,z0,y1,z1);
}

#endif

void testderivs()
{
  double i;
  vector<double> con,lin,quad,cub,para1;
  for (i=-1.5;i<2;i+=1)
  {
    con.push_back(1);
    lin.push_back(i);
    quad.push_back(i*i);
    cub.push_back(i*i*i);
  }
  assert(deriv3(con)==0);
  assert(deriv3(lin)==0);
  assert(deriv3(quad)==0);
  assert(deriv3(cub)==6);
  assert(deriv2(con)==0);
  assert(deriv2(lin)==0);
  assert(deriv2(quad)==2);
  assert(deriv2(cub)==0);
  assert(deriv1(con)==0);
  assert(deriv1(lin)==1);
  assert(deriv1(quad)==0);
  assert(deriv1(cub)==0);
  assert(deriv0(con)==1);
  assert(deriv0(lin)==0);
  assert(deriv0(quad)==0);
  assert(deriv0(cub)==0);
  cout<<"Zeroth derivative of constant "<<deriv0(con)<<endl;
  cout<<"First derivative of line "<<deriv1(lin)<<endl;
  cout<<"Second derivative of square "<<deriv2(quad)<<endl;
  cout<<"Third derivative of cube "<<deriv3(cub)<<endl;
  para1.push_back(4);
  para1.push_back(1);
  para1.push_back(0);
  para1.push_back(1);
  cout<<"Vertex "<<paravertex(para1)<<endl;
  assert(paravertex(para1)==0.5);
}

void teststl()
{
  stltriangle stltri;
  doc.pl[1].clear();
  setsurface(HYPAR);
  aster(doc,3);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  stltri=stltriangle(doc.pl[1].points[1],doc.pl[1].points[3],doc.pl[1].points[3]);
}

void testdirbound()
{
  double bound;
  int i;
  doc.pl[1].clear();
  aster(doc,100);
  bound=doc.pl[1].dirbound(degtobin(0));
  for (i=1;i<=100;i++)
    if (bound==doc.pl[1].points[i].east())
      printf("westernmost point is %d\n",i);
  assert(bound==doc.pl[1].points[94].east());
  bound=doc.pl[1].dirbound(degtobin(90));
  for (i=1;i<=100;i++)
    if (bound==doc.pl[1].points[i].north())
      printf("southernmost point is %d\n",i);
  assert(bound==doc.pl[1].points[96].north());
}

void print16_9(unsigned long long n,int size)
/* size=1: outputs n%256 in base 16 and n%243 in base 9.
 * size=2: outputs n%65536 in base 16 and n%59049 in base 9.
 * size=4: outputs n%4294967296 in base 16 and n%3486784401 in base 9.
 * size=8: outputs n in base 16 and n%(3^20) in base 9.
 */
{
  vector<char> hex,non;
  int i;
  for (i=0;i<size*2;i++)
    hex.push_back((n>>(i*4))&15);
  for (i=0;i<(size*5)/2;i++)
  {
    non.push_back(n%9);
    n/=9;
  }
  if (size&1)
    non.push_back(n%3);
  for (i=hex.size()-1;i>=0;i--)
    printf("%x",hex[i]);
  printf(" ");
  for (i=non.size()-1;i>=0;i--)
    printf("%x",non[i]);
}

void testsplithalton(unsigned long long n)
{
  vector<unsigned short> splitnum;
  unsigned int i;
  cout<<n<<endl;
  print16_9(n,4);
  putchar('\n');
  splitnum=splithalton(n);
  for (i=0;i<4;i++)
  {
    print16_9(splitnum[i],1);
    putchar(' ');
  }
  putchar('\n');
}

void testbtreverse(unsigned long long n)
{
  unsigned long long nr;
  nr=btreverselong(n);
  cout<<n<<" "<<nr<<endl;
  print16_9(n,4);
  putchar(' ');
  print16_9(nr,4);
  putchar('\n');
}

void testhalton()
{
  unsigned int i;
  halton h;
  initbtreverse();
  for (i=0;i<30;i++)
    printf("%7d ",btreversetable[i]);
  assert(btreversetable[13]==37296);
  /* 13 is 00001101 (2) and 00111 (3).
   * 37296%256=176
   * 37296%243=117
   * 176 is 10110000 (2); 117 is 11100 (3).
   */
  for (i=0;i<62208;i++)
    assert(btreversetable[btreversetable[i]]==i);
  print16_9(4294967296,8);
  putchar('\n');
  print16_9(3486784401,8);
  putchar('\n');
  testbtreverse(62208);
  testbtreverse(256);
  testbtreverse(243);
  testbtreverse(65536);
  testbtreverse(3869835264);
  testbtreverse(588235294117647);
  for (i=0;i<56;i++)
    printf("%9.7f ",h.scalar(1));
}

void testpolyline()
{
  polyline p;
  polyarc q;
  //polyspiral r;
  p.insert(xy(0,0));
  q.insert(xy(0,0));
  p.insert(xy(3,0));
  q.insert(xy(3,0));
  cout<<p.length()<<endl;
  assert(p.length()==6);
  p.insert(xy(3,4));
  q.insert(xy(3,4));
  cout<<p.length()<<endl;
  assert(p.length()==12);
  q.setlengths();
  assert(q.length()==12);
  assert(q.area()==6);
  q.open();
  assert(q.length()==7);
  assert(::isnan(q.area()));
  q.close();
  q.setlengths();
  assert(q.length()==12);
  q.setdelta(0,439875013);
  q.setdelta(1,633866811);
  q.setdelta(2,1073741824);
  q.setlengths();
  /* Total area of circle is 19.6350. Of this,
   * 6.0000 is in the triangle,
   * 9.8175 is on the 5 side,
   * 2.7956 is on the 4 side, and
   * 1.0219 is on the 3 side.
   */
  cout<<"testpolyline: area of circle is "<<q.area()<<endl;
  assert(fabs(q.length()-M_PI*5)<0.0005);
  assert(fabs(q.area()-M_PI*6.25)<0.0005);
  cout<<q.getarc(0).center().north()<<endl;
  cout<<q.length()<<endl;
}

bool before(xy a1,xy a2,xy a3,xy b1,xy b2,xy b3)
/* Returns true if a2 is nearer along than b2.
 * They are on different curves, so this isn't totally well-defined,
 * but one curve is an approximation to the other.
 */
{
  int adir,bdir,avgdir,xdir;
  adir=dir(a1,a3);
  bdir=dir(b1,b3);
  avgdir=adir+(bdir-adir)/2;
  xdir=dir(a2,b2);
  return cos(xdir-avgdir)>0;
}

double sepdist(xy a1,xy a2,xy a3,xy b1,xy b2,xy b3)
/* Separation distance from one curve to the other.
 */
{
  int adir,bdir,avgdir;
  adir=dir(a1,a3);
  bdir=dir(b1,b3);
  avgdir=adir+(bdir-adir)/2;
  return pldist(a2,b2,b2+cossin(avgdir));
}

void testbezier3d()
{
  xyz startpoint,endpoint;
  int startbearing,endbearing;
  double curvature,clothance,totaldist,avgdist;
  int i,j,numdist,ngood,nclose;
  arc arc0(xyz(-50,0,0),xyz(50,0,61));
  spiralarc spiralarc0(xyz(-50,0,0),xyz(50,0,61));
  char buf[32];
  map<double,double> dists,ests;
  xy spipts[21],bezpts[21],lastpt,thispt;
  bezier3d a(xyz(0,0,0),xyz(1,0,0),xyz(2,3,0),xyz(3,9,27)),
  b(xyz(3,9,27),xyz(4,15,54),xyz(7,11,13),xyz(2,3,5)),c;
  xyz pt,pt1;
  arc0.setslope(START,-0.1);
  arc0.setslope(END,0.2);
  spiralarc0.setslope(START,-0.1);
  spiralarc0.setslope(END,0.2);
  assert(a.size()==1);
  pt=a.station(0.4);
  pt1=xyz(1.2,1.44,1.728);
  assert(dist(pt,pt1)<1e-6);
  pt=a.station(1);
  pt1=xyz(3,9,27);
  assert(dist(pt,pt1)<1e-6);
  c=a+b;
  pt=c.station(1.5);
  pt1=b.station(.5);
  cout<<pt.east()<<' '<<pt.north()<<' '<<pt.elev()<<endl;
  assert(pt==pt1);
  psopen("bezier3d.ps");
  psprolog();
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
    {
      startpage();
      setscale(-0.2,-0.5,0.2,0.5,degtobin(90));
      startpoint=xyz(cornu(-0.5,curvature,clothance),0);
      endpoint=xyz(cornu(0.5,curvature,clothance),0);
      startbearing=ispiralbearing(-0.5,curvature,clothance);
      endbearing=ispiralbearing(0.5,curvature,clothance);
      a=bezier3d(startpoint,startbearing,0,0,endbearing,endpoint);
      for (i=-10;i<=10;i++)
      {
	spipts[i+10]=cornu(i/20.,curvature,clothance);
	bezpts[i+10]=a.station((i+10)/20.);
      }
      setcolor(0,0,0);
      sprintf(buf,"cur=%5.3f clo=%5.3f",curvature,clothance);
      pswrite(xy(0,0.2),buf);
      setcolor(1,.5,0); // red is the spiral
      for (i=-10;i<10;i++)
        line2p(spipts[i+10],spipts[i+11]);
      setcolor(0,.5,1); // blue is the Bézier I'm approximating it with
      for (i=-10;i<10;i++)
        line2p(bezpts[i+10],bezpts[i+11]);
      setcolor(0,0,0);
      for (totaldist=numdist=i=j=0,thispt=startpoint;i<20 && j<20;)
      {
	lastpt=thispt;
	if (i==0 && j==0)
	  if (before(spipts[0],spipts[1],spipts[2],bezpts[0],bezpts[1],bezpts[2]))
	    thispt=spipts[i++];
	  else
	    thispt=bezpts[j++];
	else if ((i==0) ^ (j==0))
	  if (j)
	    thispt=spipts[i++];
	  else
	    thispt=bezpts[j++];
	else
	{
	  totaldist+=sqr(sepdist(spipts[i-1],spipts[i],spipts[i+1],bezpts[j-1],bezpts[j],bezpts[j+1]));
	  numdist++;
	  if (before(spipts[i-1],spipts[i],spipts[i+1],bezpts[j-1],bezpts[j],bezpts[j+1]))
	    thispt=spipts[i++];
	  else
	    thispt=bezpts[j++];
	}
	line2p(lastpt,thispt);
      }
      avgdist=sqrt(totaldist/numdist);
      dists[clothance*M_PI+curvature]=avgdist;
      ests[clothance*M_PI+curvature]=bez3destimate(startpoint,startbearing,1,endbearing,endpoint);
      setcolor(0,0,0);
      sprintf(buf,"dist=%5.7f",avgdist);
      pswrite(xy(0,-0.2),buf);
      endpage();
      //cout<<avgdist<<endl;
    }
  startpage();
  setscale(-1,-1,1,1,0);
  setcolor(0.5,0.5,1);
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
      circle(xy(curvature,clothance/6),sqrt(ests[clothance*M_PI+curvature]));
  setcolor(0,0,0);
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
    {
      circle(xy(curvature,clothance/6),sqrt(dists[clothance*M_PI+curvature]));
      if (dists[clothance*M_PI+curvature]>ests[clothance*M_PI+curvature])
	cout<<"estimate too small crv="<<curvature<<" clo="<<clothance<<endl;
      assert(dists[clothance*M_PI+curvature]<=ests[clothance*M_PI+curvature]);
    }
  endpage();
  startpage();
  setscale(-100,-180,100,180,degtobin(0));
  for (i=-300,nclose=0;i<330;i+=60)
  {
    arc0.setdelta(degtobin(i));
    c=arc0.approx3d(1);
    cout<<i<<"° delta 1 m approx "<<c.size();
    c=arc0.approx3d(0.001);
    cout<<" splines; 1 mm approx "<<c.size()<<" splines"<<endl;
    pt=arc0.station(arc0.length()/3);
    pt1=c.station(c.size()/3.);
    //cout<<"distance "<<dist(pt,pt1)<<endl;
    nclose+=(dist(pt,pt1)<1);
    spline(c);
  }
  endpage();
  for (i=-300,ngood=0;i<330;i+=60)
  {
    startpage();
    setscale(-100,-180,100,180,degtobin(0));
    for (j=-300;j<330;j+=60)
    {
      //cout<<j<<"° delta "<<i<<"° skew"<<endl;
      spiralarc0.setdelta(degtobin(j),degtobin(i));
      if (spiralarc0.valid())
      {
	c=spiralarc0.approx3d(1);
	pt=spiralarc0.station(spiralarc0.length()/3);
	pt1=c.station(c.size()/3.);
	// Most of these aren't close, because the splitting is not uniform, but enough are.
	//cout<<"distance "<<dist(pt,pt1)<<endl;
	nclose+=(dist(pt,pt1)<1);
	spline(c);
	ngood++;
      }
    }
    endpage();
  }
  pstrailer();
  psclose();
  cout<<ngood<<" good spirals"<<endl;
  cout<<nclose<<" with 1/3 point close"<<endl;
  assert(ngood>=107);
  assert(nclose>=30);
}

void testangleconv()
{
  string strang,straz,strbear;
  int angle;
  strang=bintoangle(0,DEGREE+SEXAG2);
  cout<<strang<<endl;
  assert(strang=="0°00′00″");
  strang=bintoangle(degtobin(90),DEGREE+SEXAG2);
  cout<<strang<<endl;
  assert(strang=="90°00′00″");
  strang=bintoangle(degtobin(-80),DEGREE+SEXAG2);
  cout<<strang<<endl;
  assert(strang=="-80°00′00″");
  strang=bintoangle(atan2i(2,1),DEGREE+SEXAG2);
  cout<<strang<<endl;
  strang=bintoangle(atan2i(2,1),DEGREE+SEXAG2P2);
  cout<<strang<<endl;
  strang=bintoangle(atan2i(2,1),GON+DEC4);
  cout<<strang<<endl;
  angle=parseangle("63°26′05.82″",DEGREE);
  cout<<hex<<angle<<dec<<endl;
  angle=parsebearing("S77°W",DEGREE);
  cout<<hex<<angle<<dec<<endl;
  angle=parsebearing("S5°W",DEGREE);
  cout<<hex<<angle<<dec<<endl;
  angle=parsebearing("S67°E",DEGREE);
  cout<<hex<<angle<<dec<<endl;
  angle=parsebearing("N41°E",DEGREE);
  cout<<hex<<angle<<dec<<endl;
  angle=parsebearing("N31°W",DEGREE);
  cout<<hex<<angle<<dec<<endl;
}

void testcsvline()
{
  vector<string> words;
  string line;
  words=parsecsvline(line="");
  assert(words.size()==0);
  assert(makecsvline(words)==line);
  words=parsecsvline(line="\"\"");
  assert(words.size()==1);
  //cout<<makecsvline(words)<<endl;
  assert(makecsvline(words)==line);
  words=parsecsvline(line="\"pote\"\"mkin\"");
  assert(words[0].length()==9);
  assert(makecsvline(words)==line);
  words=parsecsvline(line="\"3,4\",\"5,12\"");
  assert(words.size()==2);
  assert(makecsvline(words)==line);
}

void testldecimal()
{
  double d;
  bool looptests=false;
  cout<<ldecimal(1/3.)<<endl;
  cout<<ldecimal(M_PI)<<endl;
  if (looptests)
  {
    for (d=M_SQRT_3-20*DBL_EPSILON;d<=M_SQRT_3+20*DBL_EPSILON;d+=DBL_EPSILON)
      cout<<ldecimal(d)<<endl;
    for (d=1.25-20*DBL_EPSILON;d<=1.25+20*DBL_EPSILON;d+=DBL_EPSILON)
      cout<<ldecimal(d)<<endl;
    for (d=95367431640625;d>1e-14;d/=5)
      cout<<ldecimal(d)<<endl;
    for (d=123400000000000;d>3e-15;d/=10)
      cout<<ldecimal(d)<<endl;
  }
  cout<<ldecimal(0)<<' '<<ldecimal(INFINITY)<<' '<<ldecimal(NAN)<<' '<<ldecimal(-5.67)<<endl;
  cout<<ldecimal(3628800)<<' '<<ldecimal(1296000)<<' '<<ldecimal(0.000016387064)<<endl;
  assert(ldecimal(0)=="0");
  assert(ldecimal(1)=="1");
  assert(ldecimal(-1)=="-1");
  assert(ldecimal(1.7320508)=="1.7320508");
  assert(ldecimal(-0.00064516)=="-.00064516");
  assert(ldecimal(3628800)=="3628800");
  assert(ldecimal(1296000)=="1296e3");
  assert(ldecimal(0.000016387064)=="1.6387064e-5");
}

void testellipsoid()
{
  xyz sealevel,kmhigh,noffset,soffset,diff,benin,bengal,howland,galapagos,npole,spole;
  ellipsoid test1(8026957,0,0.5),test2(8026957,4013478.5,0);
  assert(test1.geteqr()==test2.geteqr());
  assert(test1.getpor()==test2.getpor());
  sealevel=test1.geoc(degtobin(45),0,0);
  kmhigh=test1.geoc(degtobin(45),0,65536000);
  diff=kmhigh-sealevel;
  cout<<diff.east()<<' '<<diff.north()<<' '<<diff.elev()<<endl;
  assert(abs(diff.east()-707.107)<0.001);
  assert(abs(diff.elev()-707.107)<0.001);
  cout<<sealevel.east()<<' '<<sealevel.north()<<' '<<sealevel.elev()<<' '<<ldecimal(sealevel.east()/sealevel.elev())<<endl;
  assert(fabs(sealevel.east()/sealevel.elev()-4)<1e-9);
  noffset=test1.geoc(degtobin(45)+1,0,0);
  soffset=test1.geoc(degtobin(45)-1,0,0);
  diff=noffset-soffset;
  cout<<diff.east()<<' '<<diff.north()<<' '<<diff.elev()<<endl;
  assert(abs(diff.east()+diff.elev())<1e-6);
  cout<<"average radius "<<ldecimal(test1.avgradius())<<endl;
  benin=Sphere.geoc(0,0,0);
  bengal=Sphere.geoc(0,DEG90,0);
  howland=Sphere.geoc(0,DEG180,0);
  galapagos=Sphere.geoc(0,-DEG90,0);
  npole=Sphere.geoc(DEG90,0xdeadbeef,0);
  spole=Sphere.geoc(-DEG90,0x10decade,0);
  cout<<"Benin x      "<<benin.getx()<<endl;
  cout<<"Howland x    "<<howland.getx()<<endl;
  cout<<"Bengal y     "<<bengal.gety()<<endl;
  cout<<"Galápagos y  "<<galapagos.gety()<<endl;
  cout<<"North Pole z "<<npole.getz()<<endl;
  cout<<"South Pole z "<<spole.getz()<<endl;
  assert(benin.getx()>6370999);
  assert(howland.getx()<-6370999);
  assert(bengal.gety()>6370999);
  assert(galapagos.gety()<-6370999);
  assert(npole.getz()>6370999);
  assert(spole.getz()<-6370999);
}

void spotcheckcolor(int col0,int col1)
{
  int col2;
  col2=printingcolor(col0,4);
  if (col2!=col1)
    cout<<hex<<col0<<" gives "<<col2<<", should be "<<col1<<endl<<dec;
  assert(col1==col2);
}

void testcolor()
{
  int i,cint,cint1,g,m,hist[41];
  unsigned short csht,csht1;
  /* Check the points on the green-magenta altitude. fliphue permutes them.
   * The histogram detects an error if two colors are mapped to the same color.
   */
  g=rng.ucrandom();
  m=rng.ucrandom();
  memset(hist,0,sizeof(hist));
  for (i=0;i<256;i++)
  {
    cint=((m+i)&0xff)*0x10001+(((g-2*i)&0xff)<<8);
    cint1=printingcolor(cint,4);
    hist[cint%41]++;
    hist[cint1%41]--;
    //cout<<hex<<cint<<' '<<cint1<<dec<<endl;
  }
  for (i=0;i<41;i++)
    assert(hist[i]==0);
  // fliphue is the identity function on the gray axis.
  for (i=0;i<256;i++)
  {
    cint=i*0x010101;
    cint1=printingcolor(cint,4);
    assert(cint==cint1);
  }
  for (i=0;i<1000;i++)
  {
    cint=(rng.ucrandom()<<16)+rng.usrandom();
    cint1=cint^16777215;
    csht=colorshort(cint);
    csht1=colorshort(cint1);
    assert(csht+csht1==63999);
    csht=rng.usrandom();
    csht1=colorshort(colorint(csht));
    assert(csht==csht1);
    if (csht<64000)
    {
      csht1=63999-csht;
      cint=colorint(csht);
      cint1=colorint(csht1);
      assert(cint+cint1==16777215);
    }
  }
  /* 09 1b 27 3 0 e e n3 i
   *  |   |   | | | | |  i=inside (closer to the middle than the gray point)
   *  |   |   | | | | |  o=outside; f=fixed point (in the middle)
   *  |   |   | | | | the number of points in the triangle or hexagon
   *  |   |   | | | | is not divisible by 3, i.e. there is a point in the middle
   *  |   |   | | | even number of points (including middle, if any) in the altitude
   *  |   |   | | even number of points in the side; o=odd; x=hexagon, odd and even alternate
   *  |   |   | layers 0 and 2 are triangles, 1 is hexagon
   *  |   |   27 mod 6; 3 pairs with 4, 2 with 5, and 1 with 0
   *  |   sum of three numbers in layer 0; always 3 times gray point mod 256
   *  gray point; 090909 is in one of the layers
   */
  spotcheckcolor(0x330909,0x131919); // 17 45  69 3 0 e o n3 o
  spotcheckcolor(0x051105,0x070d07); // 09 1b  27 3 0 e e n3 i
  spotcheckcolor(0xdfefef,0xdfefef); // 3f bd 189 3 2 o o d3 f
  spotcheckcolor(0x0f0f1f,0x0f0f1f); // bf 3d  61 1 0 e o d3 f
  spotcheckcolor(0x7b817b,0x7b817b); // 7d 77 119 5 1 x e n3 f
  spotcheckcolor(0x051005,0x080a08); // 5e 1a  26 2 0 o e d3 i
  spotcheckcolor(0x5555f7,0xcccc09); // 8b a1 161 5 1 x e n3 o
  spotcheckcolor(0x7a7e7a,0x798079); // 26 72 114 0 1 x e d3 i
  spotcheckcolor(0xfeeded,0xdaffff); // 48 d8 216 0 2 e o d3 o
  spotcheckcolor(0xececde,0xefefd8); // 92 b6 182 2 2 e e d3 i
  spotcheckcolor(0xd29797,0x2ee9e9); // 00 00   0 0 2 e o d3 o
  spotcheckcolor(0x808081,0x81817f); // 2b 81 129 3 1 x e d3 o
  for (i=0;i<10;i++)
  {
    cint=(rng.ucrandom()<<16)+rng.usrandom();
    cint&=0xa5a5a5;
    cint1=printingcolor(printingcolor(cint,4),4);
  }
}

void testcontour()
{
  int i,j;
  double conterval;
  psopen("contour.ps");
  psprolog();
  startpage();
  setscale(-10,-10,10,10,0);
  doc.pl[1].clear();
  setsurface(CIRPAR);
  aster(doc,100);
  moveup(doc,-0.001);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"contour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.03;
  roughcontours(doc.pl[1],conterval);
  smoothcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  pstrailer();
  psclose();
}

void clampcubic()
/* Determine values which will be used to check whether a spiralarc well
 * approximates a contour. The contour segment is a piece of an elliptic
 * curve; the curve approximating it is an Euler spiral. Pretend both are
 * cubics. Find the distance along them of two points such that, if the
 * distance perpendicular to them is less than x, then the distance
 * between the two curves anywhere is less than ax, and a is minimal.
 * 
 * 0--------0----------|--------0
 * y=(x+1)(x+c)(x-1)
 * y'=(x+c)(x-1)+(x+1)(x-1)+(x+1)(x+c)
 * y'(-1)=2(1-c)
 * y'(1)=2(1+c)
 * 
 * Result: clamp=0.3849001794597505 (=sqrt(4/27)) max=1.173913043478261 (=27/23)
 */
{
  double clamp,valclamp,d1,dm1,maxq,maxc,h;
  vector<double> exq,exc;
  xyz st(-1,0,0),nd(1,0,0);
  segment cubic(st,nd),quadratic(st,nd);
  for (clamp=0.375,h=0.015625;fabs(h)>DBL_EPSILON/2;clamp+=h)
  {
    valclamp=(clamp+1)*2*clamp*(clamp-1);
    //cout<<"valclamp="<<valclamp<<endl;
    d1=2*(1+clamp)/valclamp;
    dm1=2*(1-clamp)/valclamp;
    cubic.setslope(END,d1+dm1);
    quadratic.setslope(END,d1-dm1);
    cubic.setslope(START,dm1+d1);
    quadratic.setslope(START,dm1-d1);
    exc=cubic.vextrema(false);
    exq=quadratic.vextrema(false);
    maxc=cubic.station(exc[1]).elev();
    maxq=quadratic.station(exq[0]).elev();
    cout<<"clamp="<<ldecimal(clamp)<<" cubic max="<<ldecimal(maxc)<<" quadratic max="<<ldecimal(maxq)<<endl;
    if ((h>0) ^ (maxq<maxc))
      h*=-0.5;
  }
}

void splitcubic()
/* Find the relation between the values at the clamp points and the extrema
 * of a cubic representing a spiralarc that does not well approximate a
 * contour. The spiralarc will be split at the point that is farthest from
 * the contour.
 */
{
  int i,bcount[65],bar;
  double ratio,bsum[65];
  vector<double> ex;
  polyline pl,plneg,plpos;
  polyspiral ps,psneg,pspos;
  xyz st(0,0,0),nd(1,0,0);
  segment cubic(st,nd);
  cubic.setslope(START,1);
  psopen("splitcubic.ps");
  psprolog();
  startpage();
  setscale(-1,0,1,0.5,0);
  for (i=0;i<65;i++)
    bsum[i]=bcount[i]=0;
  for (i=-310;i<1334;i++)
  {
    cubic.setslope(END,((2*i+1)/2048.)*2-1);
    ratio=cubic.station(1-CCHALONG).elev()/cubic.station(CCHALONG).elev();
    ex=cubic.vextrema(false);
    bar=rint((ratio+1)*32);
    if (bar>=0 && bar<=64)
    {
      bcount[bar]++;
      bsum[bar]+=ex[0];
    }
    if (i%31==0)
      cout<<ratio<<' '<<ex[0]<<endl;
    if (ratio>1)
      plpos.insert(xy(ratio,ex[0]));
    else if (ratio<-1)
      plneg.insert(xy(ratio,ex[0]));
    else
      pl.insert(xy(ratio,ex[0]));
  }
  for (i=0;i<65;i++)
    cout<<setw(2)<<i<<setw(3)<<bcount[i]<<' '<<setprecision(4)<<bsum[i]/bcount[i]<<endl;
  pl.open();
  ps=polyspiral(pl);
  ps.smooth();
  plneg.open();
  psneg=polyspiral(plneg);
  psneg.smooth();
  plpos.open();
  pspos=polyspiral(plpos);
  pspos.smooth();
  spline(ps.approx3d(0.1));
  setcolor(0,0,1);
  spline(pspos.approx3d(0.1));
  spline(psneg.approx3d(0.1));
  endpage();
  pstrailer();
  psclose();
}

void testminquad()
{
  double m;
  m=minquad(-1,1,0,0,1,1);
  cout<<"m="<<m<<endl;
  assert(m==0);
  m=minquad(0,0,1,1,2,4);
  cout<<"m="<<m<<endl;
  assert(m==0);
  m=minquad(0,0,2,4,4,16);
  cout<<"m="<<m<<endl;
  assert(m==0);
  m=minquad(0,0,1,1,3,9);
  cout<<"m="<<m<<endl;
  assert(m==0);
  m=minquad(-8,64,13,169,21,441);
  cout<<"m="<<m<<endl;
  assert(m==0);
}

void testroscat()
{
  xy xy0(32,27),xy1(16,15),xy2(46,58),xy3(58,73),xy4(84,12),xy5(79,47),
     org(0,0),piv0(27,27),piv1(12,12);
  point pt0(32,27,99,""),pt1(16,15,99,""),pt2(46,58,99,""),pt3(58,73,99,"");
  xy0.roscat(piv0,AT34,1,piv1);
  assert(dist(xy0,xy1)<1e-6);
  cout<<"roscat dist "<<dist(xy0,xy1)<<endl;
  xy2.roscat(org,0,CBRT2,org);
  assert(dist(xy2,xy3)<1e-1);
  cout<<"roscat dist "<<dist(xy2,xy3)<<endl;
  xy4.roscat(org,AT0512,13/12.,org);
  assert(dist(xy4,xy5)<1e-6);
  cout<<"roscat dist "<<dist(xy4,xy5)<<endl;
  pt0.roscat(piv0,AT34,1,piv1);
  assert(dist(pt0,pt1)<1e-6);
  cout<<"roscat dist "<<dist(pt0,pt1)<<endl;
  pt2.roscat(org,0,CBRT2,org);
  assert(dist(pt2,pt3)<1e-1);
  cout<<"roscat dist "<<dist(pt2,pt3)<<endl;
}

void testabsorient()
{
  int i;
  double ssd;
  vector<xy> a,b;
  xy big(1000,0),little(1,0),org(0,0);
  for (i=0;i<1000;i++)
  {
    big.roscat(org,AT0512,1,org);
    little.roscat(org,AT34,1,org);
    a.push_back(big);
    b.push_back(big+little);
  }
  ssd=sumsqdist(a,b);
  cout<<"sumsqdist="<<ldecimal(ssd)<<endl;
  assert(fabs(ssd-1000)<1e-9);
}

void testgeoid()
{
  vball v;
  int lat,lon,olat,olon,i,j,k;
  double x,y,sum,qpoints[16][16];
  xyz dir;
  geoquad gq;
  array<unsigned,2> ghash;
  array<double,6> corr;
  cout<<"Testing conversion to and from volleyball coordinates...";
  cout.flush();
  for (lon=-0x3f800000;lon<=0x3f800000;lon+=0x1000000) // every 2.8125°
  {
    //cout<<fixed<<setprecision(3)<<setw(7)<<bintodeg(lon)<<' ';
    for (lat=-0x1e000000;lat<=0x1e000000;lat+=0xf000000) // at 42.2°, it's about half in face 3 and half in faces 1,2,5,6
    {
      dir=WGS84.sphere->geoc(lat,lon,0);
      v=encodedir(dir);
      //cout<<v.face<<setw(7)<<v.x<<setw(7)<<v.y<<' ';
      dir=decodedir(v);
      olat=dir.lati();
      olon=dir.loni();
      if (olat!=lat || olon!=lon)
	cout<<endl<<"should be "<<bintodeg(lat)<<','<<bintodeg(lon)<<" is "<<bintodeg(olat)<<','<<bintodeg(olon)<<endl;
      assert(olat==lat && olon==lon);
    }
    //cout<<endl;
  }
  cout<<"done."<<endl;
  for (i=0;i<6;i++)
  {
    for (j=0;j<6;j++)
      gq.und[j]=(i==j)<<16;
    for (x=-0.9375,sum=0;x<1;x+=0.125)
      for (y=-0.9375;y<1;y+=0.125)
	sum+=sqr(gq.undulation(x,y));
    ghash=gq.hash();
    cout<<i<<' '<<ldecimal(sum)<<hex<<setw(9)<<ghash[0]<<setw(9)<<ghash[1]<<endl;
  }
  cout<<"Testing correction..."<<endl;
  gq.clear();
  for (k=0;k<6;k++)
  {
    gq.und[k]=65536;
    for (i=0;i<16;i++)
      for (j=0;j<16;j++)
	qpoints[i][j]=gq.undulation(-0.9375+0.125*i,-0.9375+0.125*j);
    gq.und[k]=0;
    corr=correction(gq,qpoints);
    for (i=0;i<6;i++)
      cout<<corr[i]<<' ';
    cout<<endl;
  }
}

void testgeint()
{
  int i,j,nancount;
  fstream geintf("geint.dat",fstream::in|fstream::out|fstream::binary|fstream::trunc);
  cout<<"Testing geint"<<endl;
  for (nancount=0,i=-8388625;i<=8388625;i+=47935)
  {
    writegeint(geintf,i);
    writegeint(geintf,i+0x40000000);
    writegeint(geintf,i+0x80000000);
    writegeint(geintf,i+0xc0000000);
  }
  geintf.seekg(0);
  for (i=-8388625;i<=8388625;i+=47935)
  {
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i<<setw(9)<<j;
    assert(i==j);
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0x40000000<<setw(9)<<j;
    assert(i+0x40000000==j);
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0x80000000<<setw(9)<<j;
    assert(i+0x80000000==j || j==(int)0x80000000);
    if (j==(int)0x80000000)
      nancount++;
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0xc0000000<<setw(9)<<j<<endl;
    assert(i+0xc0000000==j);
  }
  cout<<dec<<nancount<<" NANs"<<endl;
  assert(nancount==175);
  cout<<"done."<<endl;
}

void testhlattice()
{
  hlattice latt(31); // 2977 points
  hvec a,b;
  a=nthhvec(1488,31,2977);
  b=latt.nthhvec(1488);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(1457,31,2977);
  b=latt.nthhvec(1457);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(1456,31,2977);
  b=latt.nthhvec(1456);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(1024,31,2977);
  b=latt.nthhvec(1024);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(2048,31,2977);
  b=latt.nthhvec(2048);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(0,31,2977);
  b=latt.nthhvec(0);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
  a=nthhvec(2976,31,2977);
  b=latt.nthhvec(2976);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  assert(a==b);
}

int main(int argc, char *argv[])
{
  doc.pl.resize(2);
  testarea3();
  testrelprime();
  testintersection();
  testtriangle();
  testcopytopopoints();
  testinvalidintersectionlozenge();
  testinvalidintersectionaster();
  testmaketin123();
  testmaketindouble();
  testmaketinaster();
  testmaketinbigaster();
  testmaketinstraightrow();
  testmaketinlongandthin();
  testmaketinlozenge();
  testmaketinring();
  testmaketinellipse();
  testmanysum();
  testvcurve();
  testintegertrig();
  testminquad();
  testsegment();
  testarc();
  testspiral();
  testspiralarc();
  testcogospiral();
  testclosest();
  testqindex();
  testmakegrad();
  testderivs();
  trianglecontours();
#ifndef NDEBUG
  testparabinter();
#endif
  testrasterdraw();
  testdirbound();
  teststl();
  testhalton();
  testpolyline();
  testbezier3d();
  testangleconv();
  testgrad();
  testcsvline();
  testldecimal();
  testellipsoid();
  testcolor();
  testcontour();
  testroscat();
  testabsorient();
  testhlattice();
  testgeoid();
  testgeint();
  //clampcubic();
  //splitcubic();
  printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
  //cornustats();
  //closure_i();
  return EXIT_SUCCESS;
}
