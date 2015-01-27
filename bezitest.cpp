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
#include "qindex.h"
#include "random.h"
#include "ps.h"
#include "raster.h"
#include "stl.h"
#include "halton.h"
#include "polyline.h"
#include "bezier3d.h"
#include "closure.h"

#define psoutput false
// affects only maketin

char hexdig[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

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
  cout<<"Checking for invalid intersections";
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
	    itype=intersection_type(topopoints.points[j],topopoints.points[k],
				    topopoints.points[l],topopoints.points[m]);
	    if (itype!=rightanswers[j-1][k-1][l-1][m-1])
	      //printf("Iter %i, %d %d %d %d, %d should be %d\n",i,j,k,l,m,itype,rightanswers[j-1][k-1][l-1][m-1]);
	      nmisses++;
	    checkimpos(itype,topopoints.points[j],topopoints.points[k],
		       topopoints.points[l],topopoints.points[m]);
	  }
  }
  cout<<endl;
}

void testmaketin123()
{
  int i;
  topopoints.clear();
  aster(1);
  i=0;
  try
  {
    topopoints.maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==notri);
  topopoints.clear();
  aster(2);
  i=0;
  try
  {
    topopoints.maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==notri);
  topopoints.clear();
  aster(3);
  i=0;
  try
  {
    topopoints.maketin();
  }
  catch(int e)
  {
    i=e;
  }
  assert(i==0);
}

void testmaketinaster()
{
  double totallength;
  int i;
  topopoints.clear();
  aster(100);
  topopoints.maketin(psoutput?"aster.ps":"",false);
  assert(topopoints.edges.size()==284);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
  assert(fabs(totallength-600.689)<0.001);
}

void testmaketinbigaster()
{
  double totallength;
  int i;
  topopoints.clear();
  aster(5972);
  topopoints.maketin(psoutput?"bigaster.ps":"",true);
  //assert(topopoints.edges.size()==284);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
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
  topopoints.clear();
  straightrow(100);
  rotate(30);
  try
  {
    topopoints.maketin();
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
  topopoints.clear();
  longandthin(100);
  rotate(30);
  topopoints.maketin(psoutput?"longandthin.ps":"");
  assert(topopoints.edges.size()==197);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
  printf("longandthin %ld edges total length %f\n",topopoints.edges.size(),totallength);
  assert(fabs(totallength-123.499)<0.001);
}

void testmaketinlozenge()
{
  double totallength;
  int i;
  topopoints.clear();
  lozenge(100);
  rotate(30);
  topopoints.maketin(psoutput?"lozenge.ps":"");
  assert(topopoints.edges.size()==299);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
  printf("lozenge %ld edges total length %f\n",topopoints.edges.size(),totallength);
  assert(fabs(totallength-2111.8775)<0.001);
}

void testmaketinring()
{
  double totallength;
  int i;
  topopoints.clear();
  ring(100);
  rotate(30);
  topopoints.maketin(psoutput?"ring.ps":"");
  assert(topopoints.edges.size()==197);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
  printf("ring edges total length %f\n",totallength);
  //Don't assert the total length. There are over 10^56 (2^189) right answers to that.
}

void testmaketinellipse()
{
  double totallength;
  int i;
  topopoints.clear();
  ellipse(100);
  topopoints.maketin(psoutput?"ellipse.ps":"");
  assert(topopoints.edges.size()==197);
  for (totallength=i=0;i<topopoints.edges.size();i++)
    totallength+=topopoints.edges[i].length();
  printf("ellipse edges total length %f\n",totallength);
  assert(fabs(totallength-1329.4675)<0.001);
}

void testvcurve()
{
  double result,b1,c1,d1a2,b2,c2;
  vector<double> extrema;
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
  vsplit(1,2,2,1,4./8,b1,c1,d1a2,b2,c2);
  assert(vcurve(1,2,2,1,3./8)==vcurve(1,b1,c1,d1a2,3./4));
  assert(vcurve(1,2,2,1,5./8)==vcurve(d1a2,b2,c2,1,1./4));
  vsplit(0,1,2,0,3./8,b1,c1,d1a2,b2,c2);
  assert(vcurve(0,1,2,0,15./64)==vcurve(0,b1,c1,d1a2,5./8));
}

void testsegment()
{
  int i;
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
  sta=a.station(200);
  assert(sta==xyz(120,160,31));
  assert(isinf(a.radius(0)));
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
  xyz beg(0,0,3),end(300,400,7),sta;
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
    curvebearing=ispiralbearing(i/20.,0,1);
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
      b=cornu(t=i/20.,2*j,1);
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
    b=cornu(i/20.,0,1);
    c=cornu(i/20.);
    //cout<<i<<' '<<dist(b,c)<<endl;
    assert(dist(b,c)<1e-12); // it's less than 6e-17 on 64-bit
    b=cornu(i/20.,sqrt(M_PI*8),1);
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
  surveypoints.clear();
  // Start with an identically 0 surface. The elevation at the center should be 0.
  surveypoints.addpoint(1,point(1,0,0,"eip"));
  surveypoints.addpoint(1,point(-0.5,M_SQRT_3_4,0,"eip"));
  surveypoints.addpoint(1,point(-0.5,-M_SQRT_3_4,0,"eip"));
  tri.a=&surveypoints.points[1];
  tri.b=&surveypoints.points[2];
  tri.c=&surveypoints.points[3];
  tri.flatten(); // sets area, needed for computing elevations
  for (i=0;i<7;i++)
    tri.ctrl[i]=0;
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  assert(elev==0);
  // Now make a constant surface at elevation 1.
  surveypoints.points[1].setelev(1);
  surveypoints.points[2].setelev(1);
  surveypoints.points[3].setelev(1);
  for (i=0;i<7;i++)
    tri.ctrl[i]=1;
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  assert(elev==1);
  // Now make a linear surface.
  surveypoints.points[1].setelev(1);
  surveypoints.points[2].setelev(0);
  surveypoints.points[3].setelev(0);
  tri.ctrl[0]=tri.ctrl[1]=2/3.;
  tri.ctrl[2]=tri.ctrl[4]=1/3.;
  tri.ctrl[5]=tri.ctrl[6]=0;
  tri.setcentercp();
  elev=tri.elevation(o);
  printf("ctrl[3]=%f elevation=%f\n",tri.ctrl[3],elev);
  assert(abs(elev*3-1)<1e-7);
  // Now make a quadratic surface. It is a paraboloid z=r². Check that the cubic component is 0.
  surveypoints.points[1].setelev(1);
  surveypoints.points[2].setelev(1);
  surveypoints.points[3].setelev(1);
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
  surveypoints.points[1].setelev(0);
  surveypoints.points[2].setelev(0);
  surveypoints.points[3].setelev(0);
  tri.setgradient(surveypoints.points[1],(xy)surveypoints.points[1]*-2);
  tri.setgradient(surveypoints.points[2],(xy)surveypoints.points[2]*-2);
  tri.setgradient(surveypoints.points[3],(xy)surveypoints.points[3]*-2);
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
  topopoints.clear();
  plist.clear();
  topopoints.addpoint(1,point(0.3,0.3,0,""));
  topopoints.addpoint(1,point(0.6,0.8,0,""));
  topopoints.addpoint(1,point(0.8,0.6,0,""));
  for (i=0;i<3;i++)
    plist.push_back(topopoints.points[i+1]+offset);
  qinx.sizefit(plist);
  printf("side=%f x=%f y=%f\n",qinx.side,qinx.x,qinx.y);
  assert(qinx.side==1);
  topopoints.maketin();
  topopoints.maketriangles();
  printf("%d triangle, should be 1\n",(int)topopoints.triangles.size());
  assert(topopoints.triangles.size()==1);
  qinx.clear();
  topopoints.clear();
  plist.clear();
  psopen("qindex.ps");
  psprolog();
  startpage();
  setscale(1,-7,31,23);
  aster(100);
  topopoints.maketin();
  enlarge(pow(2,(rng.usrandom()-32767.5)/65536));
  for (i=0;i<100;i++)
  {
    dot(topopoints.points[i+1]+offset);
    plist.push_back(topopoints.points[i+1]+offset);
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
  topopoints.maketin();
  topopoints.maketriangles();
  for (i=ntri=0;i<topopoints.edges.size();i++)
  {
    ntri+=topopoints.edges[i].tria!=NULL;
    ntri+=topopoints.edges[i].trib!=NULL;
    line(topopoints.edges[i],i,false,true);
    setcolor(0.6,0.4,0);
    if (topopoints.edges[i].tria)
      line2p(topopoints.edges[i].midpoint(),topopoints.edges[i].tria->centroid());
    setcolor(0,0.4,0.6);
    if (topopoints.edges[i].trib)
      line2p(topopoints.edges[i].midpoint(),topopoints.edges[i].trib->centroid());
  }
  printf("%d edges ntri=%d\n",i,ntri);
  assert(ntri>i/2);
  setcolor(1,0,0);
  for (i=0;i<topopoints.triangles.size();i++)
  {
    assert(topopoints.triangles[i].area()>0);
    //printf("tri %d area %f\n",i,topopoints.triangles[i].area());
    dot(topopoints.triangles[i].centroid());
  }
  printf("%d triangles\n",i);
  assert(ntri==i*3); // ntri is the number of sides of edges which are linked to a triangle
  endpage();
  ptri=&topopoints.triangles[0];
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
    plist.push_back(topopoints.points[i+1]);
  qinx.sizefit(plist);
  qinx.split(plist);
  qinx.draw();
  qinx.settri(&topopoints.triangles[0]);
  for (i=ntri=0;i<topopoints.edges.size();i++)
    line(topopoints.edges[i],i,false);
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
  for (i=topopoints.points.begin();i!=topopoints.points.end();i++)
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
  for (n=0,i=topopoints.points.begin();i!=topopoints.points.end();i++,n++)
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
  topopoints.clear();
  setsurface(HYPAR);
  aster(100);
  topopoints.maketin();
  topopoints.makegrad(0.15);
  grad63=topopoints.points[63].gradient;
  enlarge(2); // affects x and y but not z, so gradient should be halved
  topopoints.makegrad(0.15);
  grad63half=topopoints.points[63].gradient;
  printf("grad63 %f %f grad63half %f %f\n",grad63.east(),grad63.north(),grad63half.east(),grad63half.north());
  assert(grad63==grad63half*2);
  enlarge(0.5);
  psopen("gradient.ps");
  psprolog();
  for (corr=0;corr<=1;corr+=0.1)
  {
    startpage();
    topopoints.makegrad(corr);
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
  topopoints.clear();
  setsurface(HYPAR);
  aster(100);
  topopoints.maketin();
  topopoints.makegrad(0.);
  topopoints.maketriangles();
  topopoints.setgradient();
  topopoints.makeqindex();
  rasterdraw(topopoints,xy(0,0),30,30,30,0,3,"raster.ppm");
  topopoints.setgradient(true);
  rasterdraw(topopoints,xy(0,0),30,30,30,0,3,"rasterflat.ppm");
}

void test1tri(string triname,int excrits)
{
  vector<double> xs;
  vector<xyz> slice;
  vector<xy> crits;
  int j,side,cubedir,ptype;
  double vertex,offset;
  string fname,tfname,psfname;
  fstream ofile;
  fname=triname+".ppm";
  tfname=triname+".txt";
  psfname=triname+".ps";
  topopoints.maketriangles();
  topopoints.setgradient();
  topopoints.makeqindex();
  rasterdraw(topopoints,xy(5,0),30,40,30,0,1,fname);
  ofile.open(tfname.c_str(),ios_base::out);
  psopen(psfname.c_str());
  psprolog();
  startpage();
  setscale(-17,-17,17,17);
  for (j=0;j<topopoints.edges.size();j++)
    line(topopoints.edges[j],j,false);
  cubedir=topopoints.triangles[0].findnocubedir();
  ofile<<"Zero cube dir "<<cubedir<<' '<<bintodeg(cubedir)<<"°"<<endl;
  ofile<<"Zero quad offset "<<topopoints.triangles[0].flatoffset()<<endl;
  for (j=30;j>=-30;j--)
  {
    offset=j/20.;
    xs=topopoints.triangles[0].xsect(cubedir,offset);
    vertex=paravertex(xs);
    ofile<<fixed<<setprecision(3)<<setw(7)<<offset<<' '<<setw(7)<<deriv3(xs)<<' '<<setw(7)<<vertex;
    if (vertex<=1.5 && vertex>=-1.5)
      ofile<<string(rint((vertex+1.5)*20),' ')<<'*';
    ofile<<endl;
  }
  line2p(topopoints.triangles[0].spcoord(1.5,-1.5),topopoints.triangles[0].spcoord(-1.5,-1.5));
  line2p(topopoints.triangles[0].spcoord(-1.5,-1.5),topopoints.triangles[0].spcoord(-1.5,1.5));
  for (side=0;side<2;side++)
  {
    ofile<<"Side "<<side<<endl;
    slice=topopoints.triangles[0].slices(side);
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
	line2p(topopoints.triangles[0].spcoord(slice[j-1].east(),slice[j-1].north()),
		topopoints.triangles[0].spcoord(slice[j].east(),slice[j].north()));
      }
    }
    crits=topopoints.triangles[0].criticalpts_side(side);
    for (j=0;j<crits.size();j++)
    {
      ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
      dot(crits[j]);
    }
  }
  crits=topopoints.triangles[0].criticalpts_axis();
  ofile<<endl;
  for (j=0;j<crits.size();j++)
  {
    ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
    dot(crits[j]);
  }
  topopoints.triangles[0].findcriticalpts();
  crits=topopoints.triangles[0].critpoints;
  for (j=0;j<crits.size();j++)
  {
    ptype=topopoints.triangles[0].pointtype(crits[j]);
    cout<<crits[j].east()<<','<<crits[j].north()<<" type="<<ptype<<endl;
    ofile<<crits[j].east()<<','<<crits[j].north()<<" type="<<ptype<<endl;
    assert(ptype!=PT_SLOPE && ptype!=PT_GRASS);
  }
  endpage();
  psclose();
  cout<<fname<<endl;
  if (crits.size()!=excrits && excrits>=0)
    cout<<crits.size()<<" critical points found, "<<excrits<<" expected"<<endl;
  assert(crits.size()==excrits || excrits<0);
}

void test1grad()
{
  xyz grad3;
  xy pt,grad2;
  map<int,triangle>::iterator i;
  int j;
  vector<double> xsect,ysect;
  topopoints.maketriangles();
  topopoints.setgradient();
  topopoints.makeqindex();
  for (i=topopoints.triangles.begin();i!=topopoints.triangles.end();i++)
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
  unsigned char bytes[9];
  string fname;
  xy tilt;
  topopoints.clear();
  regpolygon(3);
  enlarge(10);
  topopoints.maketin();
  fname="tri";
  for (j=0;j<9;j++)
  {
    bytes[j]=rng.ucrandom();
    fname+=hexdig[bytes[j]>>4];
    fname+=hexdig[bytes[j]&15];
  }
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev((bytes[j]-127.5)/100);
    topopoints.points[j+1].gradient=xy((bytes[j+3]-127.5)/1000,(bytes[j+6]-127.5)/1000);
  }
  test1tri(fname,-1);
  fname=fname.substr(0,9)+"flat";
  topopoints.setgradient(true);
  for (j=0;j<3;j++)
    topopoints.points[j+1].gradient=xy(topopoints.elevation(xy(1,0))-topopoints.elevation(xy(0,0)),topopoints.elevation(xy(0,1))-topopoints.elevation(xy(0,0)));
  test1tri(fname,0);
  fname="monkeysaddle";
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(0);
    topopoints.points[j+1].gradient=turn90(xy(topopoints.points[j+1]))/10;
  }
  test1tri(fname,1);
  fname="tilted";
  tilt=(xy(topopoints.points[1])-xy(topopoints.points[3]))/1000;
  /* This triangle has two saddle points on a horizontal line in the no-cube direction.
   * The triangle has three equally spaced nocubedirs; the program picks the one
   * passing through points[2].
   */
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(dot(tilt,xy(topopoints.points[j+1])));
    topopoints.points[j+1].gradient=turn90(xy(topopoints.points[j+1]))/10+tilt;
  }
  test1tri(fname,2);
  fname="tolted";
  tilt=(xy(topopoints.points[3])-xy(topopoints.points[2]))/1000;
  /* This is the same as above, except for tilting in a different direction
   * perpendicular to a different no-cube direction.
   */
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(dot(tilt,xy(topopoints.points[j+1])));
    topopoints.points[j+1].gradient=turn90(xy(topopoints.points[j+1]))/10+tilt;
  }
  test1tri(fname,2);
  fname="table";
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(M_PI);
    topopoints.points[j+1].gradient=xy(0,0);
  }
  test1tri(fname,0);
  i=topopoints.triangles[0].pointtype(xy(0,0));
  fname="floor";
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(0);
    topopoints.points[j+1].gradient=xy(0,0);
  }
  test1tri(fname,0);
}

void testgrad()
{
  int j;
  topopoints.clear();
  setsurface(FLATSLOPE);
  regpolygon(3);
  topopoints.maketin();
  topopoints.makegrad(0);
  cout<<"testgrad"<<endl;
  test1grad();
  for (j=0;j<3;j++)
  {
    topopoints.points[j+1].setelev(M_PI);
    topopoints.points[j+1].gradient=xy(0,0);
  }
  test1grad();
  topopoints.clear();
  setsurface(HYPAR);
  aster(20);
  topopoints.maketin();
  topopoints.makegrad(0);
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
  topopoints.clear();
  setsurface(HYPAR);
  aster(3);
  topopoints.maketin();
  topopoints.makegrad(0.);
  topopoints.maketriangles();
  topopoints.setgradient();
  topopoints.makeqindex();
  stltri=stltriangle(topopoints.points[1],topopoints.points[3],topopoints.points[3]);
}

void testdirbound()
{
  double bound;
  int i;
  topopoints.clear();
  aster(100);
  bound=topopoints.dirbound(degtobin(0));
  for (i=1;i<=100;i++)
    if (bound==topopoints.points[i].east())
      printf("westernmost point is %d\n",i);
  assert(bound==topopoints.points[94].east());
  bound=topopoints.dirbound(degtobin(90));
  for (i=1;i<=100;i++)
    if (bound==topopoints.points[i].north())
      printf("southernmost point is %d\n",i);
  assert(bound==topopoints.points[96].north());
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
  p.insert(xy(0,0));
  p.insert(xy(3,0));
  p.insert(xy(3,4));
  p.setlengths();
  assert(p.length()==12);
  p.open();
  assert(p.length()==7);
  p.close();
  p.setlengths();
  assert(p.length()==12);
  p.setdelta(0,439875013);
  p.setdelta(1,633866811);
  p.setdelta(2,1073741824);
  p.setlengths();
  assert(fabs(p.length()-M_PI*5)<0.0005);
  cout<<p.getarc(0).center().north()<<endl;
  cout<<p.length()<<endl;
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
  int i,j,numdist;
  char buf[32];
  map<double,double> dists,ests;
  xy spipts[21],bezpts[21],lastpt,thispt;
  bezier3d a(xyz(0,0,0),xyz(1,0,0),xyz(2,3,0),xyz(3,9,27)),
  b(xyz(3,9,27),xyz(4,15,54),xyz(7,11,13),xyz(2,3,5)),c;
  xyz pt,pt1;
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
    for (clothance=-3;clothance<3.1;clothance+=(clothance>-0.38 && clothance<0.37)?0.125:0.375)
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
    for (clothance=-3;clothance<3.1;clothance+=(clothance>-0.38 && clothance<0.37)?0.125:0.375)
      circle(xy(curvature,clothance/3),sqrt(ests[clothance*M_PI+curvature]));
  setcolor(0,0,0);
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-3;clothance<3.1;clothance+=(clothance>-0.38 && clothance<0.37)?0.125:0.375)
    {
      circle(xy(curvature,clothance/3),sqrt(dists[clothance*M_PI+curvature]));
      if (dists[clothance*M_PI+curvature]>ests[clothance*M_PI+curvature])
	cout<<"estimate too small crv="<<curvature<<" clo="<<clothance<<endl;
      assert(dists[clothance*M_PI+curvature]<=ests[clothance*M_PI+curvature]);
    }
  endpage();
  pstrailer();
  psclose();
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

int main(int argc, char *argv[])
{
  testarea3();
  testintersection();
  testtriangle();
  testcopytopopoints();
  testinvalidintersectionlozenge();
  testinvalidintersectionaster();
  testmaketin123();
  testmaketinaster();
  testmaketinbigaster();
  testmaketinstraightrow();
  testmaketinlongandthin();
  testmaketinlozenge();
  testmaketinring();
  testmaketinellipse();
  testvcurve();
  testintegertrig();
  testsegment();
  testarc();
  testspiral();
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
  printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
  //closure_i();
  return EXIT_SUCCESS;
}
