/******************************************************/
/*                                                    */
/* bezitest.cpp - test program                        */
/*                                                    */
/******************************************************/
/* Copyright 2012,2013,2014,2015,2016 Pierre Abbat.
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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <csignal>
#include <cfloat>
#include <cstring>
#include <unistd.h>
#include "config.h"
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
#include "projection.h"
#include "color.h"
#include "document.h"
#include "relprime.h"
#include "contour.h"
#include "absorient.h"
#include "hlattice.h"
#include "histogram.h"
#include "geoid.h"
#include "refinegeoid.h"
#include "binio.h"
#include "sourcegeoid.h"
#include "bicubic.h"
#include "matrix.h"

#define psoutput false
// affects only maketin

#define CBRT2 1.2599210498948731647592197537

#define tassert(x) testfail|=(!(x))
// so that tests still work in non-debug builds

using namespace std;

char hexdig[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
bool slowmanysum=false;
bool testfail=false;
document doc;
vector<string> args;

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
  tassert(totsinerror+totcoserror+totciserror<2e-29);
  //On Linux, the total error is 2e-38 and the M_PIl makes a big difference.
  //On DragonFly BSD, the total error is 1.7e-29 and M_PIl is absent.
  tassert(bintodeg(0)==0);
  tassert(fabs(bintodeg(0x15555555)-60)<0.0000001);
  tassert(fabs(bintomin(0x08000000)==1350));
  tassert(fabs(bintosec(0x12345678)-184320)<0.001);
  tassert(fabs(bintogon(0x1999999a)-80)<0.0000001);
  tassert(fabs(bintorad(0x4f1bbcdd)-3.88322208)<0.00000001);
  for (i=-2147400000;i<2147400000;i+=rng.usrandom()+18000)
  {
    cout<<setw(11)<<i<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs<<bs;
    cout.flush();
    tassert(degtobin(bintodeg(i))==i);
    tassert(mintobin(bintomin(i))==i);
    tassert(sectobin(bintosec(i))==i);
    tassert(gontobin(bintogon(i))==i);
    tassert(radtobin(bintorad(i))==i);
  }
  tassert(sectobin(1295999.9999)==-2147483648);
  tassert(sectobin(1296000.0001)==-2147483648);
  tassert(sectobin(-1295999.9999)==-2147483648);
  tassert(sectobin(-1296000.0001)==-2147483648);
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
    cout<<"intersection is "<<ldecimal(inters.east())<<','<<ldecimal(inters.north())<<", should be "<<ldecimal(inte.east())<<','<<ldecimal(inte.north())<<endl;
    printf("intersection type is %d, should be %d\n",itype,type);
  }
  tassert(noint || inters==inte);
  tassert(itype==type);
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

void test1in(xy p,xy a,xy b,xy c,int windnum)
{
  int wind;
  wind=in3(p,a,b,c);
  if (wind!=windnum)
    cout<<"Triangle ("<<a.east()<<','<<a.north()<<"),("<<
      b.east()<<','<<b.north()<<"),("<<
      c.east()<<','<<c.north()<<"): ("<<
      p.east()<<','<<p.north()<<")'s winding number is "<<wind<<
      ", should be "<<windnum<<endl;
  tassert(wind==windnum);
  wind=in3(p,c,b,a);
  if (windnum<10 && windnum>-10)
    windnum=-windnum;
  if (wind!=windnum)
    cout<<"Triangle ("<<c.east()<<','<<c.north()<<"),("<<
      b.east()<<','<<b.north()<<"),("<<
      a.east()<<','<<a.north()<<"): ("<<
      p.east()<<','<<p.north()<<")'s winding number is "<<wind<<
      ", should be "<<windnum<<endl;
  tassert(wind==windnum);
}

void testin()
{
  xy a(0,0),b(4,0),c(0,3),d(4/3.,1),e(4,3),f(5,0),g(7,-1),h(8,-3),
     i(3,-5),j(0,-6),k(-2,-2),l(-4,0),m(-4,-3),n(-4,6),o(-3,7),p(0,8),q(2,1.5);
  test1in(d,a,b,c,2);
  test1in(e,a,b,c,0);
  test1in(f,a,b,c,0);
  test1in(g,a,b,c,0);
  test1in(h,a,b,c,0);
  test1in(i,a,b,c,0);
  test1in(j,a,b,c,0);
  test1in(k,a,b,c,0);
  test1in(l,a,b,c,0);
  test1in(m,a,b,c,0);
  test1in(n,a,b,c,0);
  test1in(o,a,b,c,0);
  test1in(p,a,b,c,0);
  test1in(q,a,b,c,1);
  test1in(a,a,b,c,IN_AT_CORNER);
  test1in(b,a,b,c,IN_AT_CORNER);
  test1in(c,a,b,c,IN_AT_CORNER);
  test1in(b,c,h,n,0);
}

void knowndet(matrix &mat)
/* Sets mat to a triangular matrix with ones on the diagonal, which is known
 * to have determinant 1, then permutes the rows and columns so that Gaussian
 * elimination will mess up the entries.
 *
 * The number of rows should be 40 at most. More than that, and it will not
 * be shuffled well.
 */
{
  int i,j,ran,rr,rc,cc,flipcount,size;
  mat.setidentity();
  size=mat.getrows();
  for (i=0;i<size;i++)
    for (j=0;j<i;j++)
      mat[i][j]=(rng.ucrandom()*2-255)/BYTERMS;
  for (flipcount=0;flipcount<2*size || (flipcount&1);)
  { // If flipcount were odd, the determinant would be -1 instead of 1.
    ran=rng.usrandom();
    rr=ran%size;
    ran/=size;
    rc=ran%size;
    ran/=size;
    cc=ran%size;
    if (rr!=rc)
    {
      flipcount++;
      mat.swaprows(rr,rc);
    }
    if (rc!=cc)
    {
      flipcount++;
      mat.swapcolumns(rc,cc);
    }
  }
}

void dumpknowndet(matrix &mat)
{
  int i,j,byte;
  for (i=0;i<mat.getrows();i++)
    for (j=0;j<mat.getcolumns();j++)
    {
      if (mat[i][j]==0)
	cout<<"z0";
      else if (mat[i][j]==1)
	cout<<"z1";
      else
      {
	byte=rint(mat[i][j]*BYTERMS/2+127.5);
	cout<<hexdig[byte>>4]<<hexdig[byte&15];
      }
    }
  cout<<endl;
}

void loadknowndet(matrix &mat,string dump)
{
  int i,j,byte;
  string item;
  for (i=0;i<mat.getrows();i++)
    for (j=0;j<mat.getcolumns();j++)
    {
      item=dump.substr(0,2);
      dump.erase(0,2);
      if (item[0]=='z')
	mat[i][j]=item[1]-'0';
      else
      {
	byte=stoi(item,0,16);
	mat[i][j]=(byte*2-255)/BYTERMS;
      }
    }
  cout<<endl;
}

void testmatrix()
{
  int i,j,chk2,chk3,chk4;
  matrix m1(3,4),m2(4,3),m3(4,3),m4(4,3);
  matrix t1(37,41),t2(41,43),t3(43,37),p1,p2,p3;
  matrix hil(8,8),lih(8,8),hilprod;
  matrix kd(7,7);
  double tr1,tr2,tr3,de1,de2,de3;
  double toler=8e-13;
  double kde;
  manysum lihsum;
  m1[2][0]=5;
  m1[1][3]=7;
  tassert(m1[2][0]==5);
  m2[2][0]=9;
  m2[1][4]=6;
  tassert(m2[2][0]==9);
  for (i=0;i<4;i++)
    for (j=0;j<3;j++)
    {
      m2[i][j]=rng.ucrandom();
      m3[i][j]=rng.ucrandom();
    }
  for (chk2=chk3=i=0;i<4;i++)
    for (j=0;j<3;j++)
    {
      chk2=(50*chk2+(int)m2[i][j])%83;
      chk3=(50*chk3+(int)m3[i][j])%83;
    }
  m4=m2+m3;
  for (chk4=i=0;i<4;i++)
    for (j=0;j<3;j++)
      chk4=(50*chk4+(int)m4[i][j])%83;
  tassert(chk4==(chk2+chk3)%83);
  m4=m2-m3;
  for (chk4=i=0;i<4;i++)
    for (j=0;j<3;j++)
      chk4=(50*chk4+(int)m4[i][j]+332)%83;
  tassert(chk4==(chk2-chk3+83)%83);
  t1.randomize_c();
  t2.randomize_c();
  t3.randomize_c();
  p1=t1*t2*t3;
  p2=t2*t3*t1;
  p3=t3*t1*t2;
  tr1=p1.trace();
  tr2=p2.trace();
  tr3=p3.trace();
  de1=p1.determinant();
  de2=p2.determinant();
  de3=p3.determinant();
  cout<<"trace1 "<<ldecimal(tr1)
      <<" trace2 "<<ldecimal(tr2)
      <<" trace3 "<<ldecimal(tr3)<<endl;
  tassert(fabs(tr1-tr2)<toler && fabs(tr2-tr3)<toler && fabs(tr3-tr1)<toler);
  tassert(tr1!=0);
  cout<<"det1 "<<de1
      <<" det2 "<<de2
      <<" det3 "<<de3<<endl;
  tassert(fabs(de1)>1e83 && fabs(de2)<1e45 && fabs(de3)<1e23);
  // de2 and de3 would actually be 0 with exact arithmetic.
  for (i=0;i<8;i++)
    for (j=0;j<8;j++)
      hil[i][j]=1./(i+j+1);
  lih=invert(hil);
  for (i=0;i<8;i++)
    for (j=0;j<i;j++)
      lihsum+=fabs(lih[i][j]-lih[j][i]);
  cout<<"Total asymmetry of inverse of Hilbert matrix is "<<lihsum.total()<<endl;
  hilprod=hil*lih;
  lihsum.clear();
  for (i=0;i<8;i++)
    for (j=0;j<8;j++)
      lihsum+=fabs(hilprod[i][j]-(i==j));
  cout<<"Total error of Hilbert matrix * inverse is "<<lihsum.total()<<endl;
  tassert(lihsum.total()<2e-5 && lihsum.total()>1e-15);
  knowndet(kd);
  //loadknowndet(kd,"z0z0z0z1c9dd28z0z1z03c46c35cz0z0z0z0z1z0z0aa74z169f635e3z0z0z0z003z0z1z0z0z0z0fcz146z160z000f50091");
  kd.dump();
  dumpknowndet(kd);
  kde=kd.determinant();
  cout<<"Determinant of shuffled matrix is "<<ldecimal(kde)<<endl;
  tassert(fabs(kde-1)<5e-15);
}

void testcopytopopoints()
{
  criteria crit;
  criterion crit1;
  doc.pl[0].clear();
  doc.copytopopoints(crit);
  tassert(doc.pl[1].points.size()==0);
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
  tassert(doc.pl[1].points.size()==3);
}

void checkimpos(int itype,xy a,xy c,xy b,xy d)
{
  if (itype==IMPOS)
    fprintf(stderr,"Impossible intersection type\n(%e,%e)=(%e,%e) × (%e,%e)=(%e,%e)\n(%a,%a)=(%a,%a) × (%a,%a)=(%a,%a)\n",
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north(),
            a.east(),a.north(),c.east(),c.north(),b.east(),b.north(),d.east(),d.north());
  tassert(itype!=IMPOS);
}

void testinvalidintersectionlozenge()
{
  int i,j,k,l,m,itype;
  char rightanswers[9][9][9][9];
  cout<<"Checking for invalid intersections (lozenge)";
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
	    checkimpos(itype,doc.pl[1].points[j],doc.pl[1].points[k],
		       doc.pl[1].points[l],doc.pl[1].points[m]);
	    /*if (itype!=rightanswers[j-1][k-1][l-1][m-1])
	      printf("Iter %i, %d %d %d %d, %d should be %d\n",i,j,k,l,m,itype,rightanswers[j-1][k-1][l-1][m-1]);*/
	  }
  }
  cout<<endl;
}

void testinvalidintersectionaster()
{
  int i,j,k,l,m,itype,nmisses;
  char rightanswers[9][9][9][9];
  double shift;
  cout<<"Checking for invalid intersections (aster)";
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
    tassert(sa!=sb && sb!=sc && sc !=sa && sa!=NULL && sb!=NULL && sc!=NULL);
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
  tassert(i==notri);
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
  tassert(i==notri);
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
  tassert(i==0);
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
  tassert(doc.pl[1].edges.size()==284);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  tassert(fabs(totallength-600.689)<0.001);
}

void testmaketinbigaster()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  aster(doc,5972);
  doc.pl[1].maketin(psoutput?"bigaster.ps":"",true);
  //tassert(doc.pl[1].edges.size()==284);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  //dumppointsvalence();
  //tassert(fabs(totallength-600.689)<0.001);
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
  tassert(i==flattri);
}

void testmaketinlongandthin()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  longandthin(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"longandthin.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("longandthin %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  tassert(fabs(totallength-123.499)<0.001);
}

void testmaketinlozenge()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  lozenge(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"lozenge.ps":"");
  tassert(doc.pl[1].edges.size()==299);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("lozenge %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  tassert(fabs(totallength-2111.8775)<0.001);
}

void testmaketinring()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  ring(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"ring.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("ring edges total length %f\n",totallength);
  //Don't tassert the total length. There are over 10^56 (2^189) right answers to that.
}

void testmaketinellipse()
{
  double totallength;
  int i;
  doc.pl[1].clear();
  ellipse(doc,100);
  doc.pl[1].maketin(psoutput?"ellipse.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  for (totallength=i=0;i<doc.pl[1].edges.size();i++)
    totallength+=doc.pl[1].edges[i].length();
  printf("ellipse edges total length %f\n",totallength);
  tassert(fabs(totallength-1329.4675)<0.001);
}

void testrelprime()
{
  tassert(relprime(987)==610);
  tassert(relprime(610)==377);
  tassert(relprime(377)==233);
  tassert(relprime(233)==144);
  tassert(relprime(144)==89);
  tassert(relprime(89)==55);
  tassert(relprime(55)==34);
  tassert(relprime(100000)==61803);
  tassert(relprime(100)==61);
  tassert(relprime(0)==1);
  tassert(relprime(1)==1);
  tassert(relprime(2)==1);
  tassert(relprime(3)==2);
  tassert(relprime(4)==3);
  tassert(relprime(5)==3);
  tassert(relprime(6)==5);
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
  tassert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  tassert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  tassert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
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
  tassert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  tassert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  tassert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
}

void testvcurve()
{
  double result,b1,c1,d1a2,b2,c2,epsilon;
  int i;
  vector<double> extrema,xs;
  tassert(vcurve(0,0,0,0,0)==0);
  tassert(vcurve(0,1,2,3,0.5)==1.5);
  tassert(vcurve(0,4,4,0,0.5)==3);
  tassert(vcurve(0,1,-1,0,0.5)==0);
  tassert(vcurve(0,0,0,64,0)==0);
  tassert(vcurve(0,0,0,64,0.25)==1);
  tassert(vcurve(0,0,0,64,0.5)==8);
  tassert(vcurve(0,0,0,64,0.75)==27);
  tassert(vcurve(0,0,0,64,1)==64);
  tassert(vslope(0,0,0,0,0)==0);
  tassert(vslope(0,1,2,3,0.5)==3);
  tassert(vslope(0,1,2,3,0.25)==3);
  tassert(vslope(0,4,4,0,0.5)==0);
  tassert(vslope(0,4,4,0,0)==12);
  tassert(vslope(0,0,0,64,0)==0);
  tassert(vslope(0,0,0,64,0.25)==12);
  tassert(vslope(0,0,0,64,0.5)==48);
  tassert(vslope(0,0,0,64,0.75)==108);
  tassert(vslope(0,0,0,64,1)==192);
  tassert(vaccel(0,0,0,0,0)==0);
  tassert(vaccel(0,1,2,3,0.5)==0);
  tassert(vaccel(0,1,2,3,0.25)==0);
  tassert(vaccel(0,4,4,0,0.5)==-24);
  tassert(vaccel(0,4,4,0,0)==-24);
  tassert(vaccel(0,0,0,64,0)==0);
  tassert(vaccel(0,0,0,64,0.25)==96);
  tassert(vaccel(0,0,0,64,0.5)==192);
  tassert(vaccel(0,0,0,64,0.75)==288);
  tassert(vaccel(0,0,0,64,1)==384);
  tassert(fabs(vlength(0,1,2,3,7.2)-7.8)<1e-6);
  result=vlength(0,10,0,20,200);
  printf("vertical curve length=%f\n",result);
  tassert(result>201 && result<204.427);
  extrema=vextrema(0,1,2,3);
  tassert(extrema.size()==0);
  extrema=vextrema(0,4,4,0);
  tassert(extrema.size()==1);
  printf("0,4,4,0: extrema[0]=%f\n",extrema[0]);
  tassert(extrema[0]==0.5);
  extrema=vextrema(2,4,0,2);
  tassert(extrema.size()==2);
  printf("2,4,0,2: extrema[0]=%f, extrema[1]=%f\n",extrema[0],extrema[1]);
  tassert(fabs(vslope(2,4,0,2,extrema[0]))<0.001);
  extrema=vextrema(0,4,3,0);
  tassert(extrema.size()==1);
  printf("0,4,3,0: extrema[0]=%f\n",extrema[0]);
  tassert(fabs(vslope(0,4,3,0,extrema[0]))<0.001);
  extrema=vextrema(M_PI,M_PI,M_E,M_E);
  tassert(extrema.size()==2);
  printf("π,π,e,e: extrema[0]=%f,%f\n",extrema[0],extrema[1]);
  tassert(extrema[0]==0 && extrema[1]==1);
  extrema=vextrema(M_PI,M_PI,M_E,3);
  tassert(extrema.size()==2);
  printf("π,π,e,3: extrema[0]=%f,%f\n",extrema[0],extrema[1]);
  tassert(extrema[0]==0);
  for (epsilon=1/128.;epsilon>1e-15;epsilon*=0.5)
  {
    extrema=vextrema(0,1-epsilon,1+epsilon,0);
    tassert(extrema.size()==1);
    printf("0,1-ε,1+ε,0 ε=%.3e: extrema[0]-0.5=%f*ε\n",epsilon,(extrema[0]-0.5)/epsilon);
    tassert(fabs((extrema[0]-0.5)/epsilon-0.25)<0.001);
    xs.clear();
    for (i=-3;i<4;i+=2)
      xs.push_back(vcurve(0,1-epsilon,1+epsilon,0,(1-i*epsilon)/2));
    //printf("vertex=%f*ε deriv3=%f\n",paravertex(xs),deriv3(xs));
    //tassert(extrema[0]==0.5);
  }
  vsplit(1,2,2,1,4./8,b1,c1,d1a2,b2,c2);
  tassert(vcurve(1,2,2,1,3./8)==vcurve(1,b1,c1,d1a2,3./4));
  tassert(vcurve(1,2,2,1,5./8)==vcurve(d1a2,b2,c2,1,1./4));
  vsplit(0,1,2,0,3./8,b1,c1,d1a2,b2,c2);
  tassert(vcurve(0,1,2,0,15./64)==vcurve(0,b1,c1,d1a2,5./8));
}

void testsegment()
{
  int i;
  double cept;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta;
  segment a(beg,end),b,c;
  tassert(a.length()==500);
  tassert(a.chordlength()==500);
  tassert(a.chordbearing()==316933406);
  a.setslope(START,0.3+a.avgslope());
  a.setslope(END,-0.1+a.avgslope());
  tassert(fabs(a.elev(1)-3.3)<0.05);
  tassert(fabs(a.slope(250)+0.042)<0.001);
  cept=a.contourcept(5);
  cout<<"a crosses 5 at "<<cept<<"; a.elev()="<<a.elev(cept)<<endl;
  tassert(fabs(a.elev(cept)-5)<1e-6);
  cept=a.contourcept(2);
  tassert(std::isnan(cept));
  sta=a.station(200);
  tassert(sta==xyz(120,160,31));
  tassert(std::isinf(a.radius(0)));
  tassert(a.curvature(0)==0);
  tassert(!isfinite(a.center().east()));
  a.split(200,b,c);
  tassert(dist(b.station(123),a.station(123))<0.001);
  tassert(dist(c.station(200),a.station(400))<0.001);
  a.setslope(START,0);
  extrema=a.vextrema(true);
  cout<<extrema.size()<<" extrema"<<endl;
  tassert(extrema.size()==2);
  extrema=a.vextrema(false);
  tassert(extrema.size()==1);
}

void testarc()
{
  int i;
  double xx;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),beg1(0,-15,0),end1(0,15,0),sta,sta1,sta2;
  xy ctr;
  arc a(beg,end),b,c;
  tassert(fabs(a.length()-500)<0.001);
  tassert(a.chordlength()==500);
  a.setdelta(degtobin(60));
  tassert(fabs(a.length()-523.599)<0.001);
  tassert(a.chordlength()==500);
  a.setslope(START,0.3+a.avgslope());
  a.setslope(END,-0.1+a.avgslope());
  //printf("slope(250) %f\n",a.slope(250));
  //printf("slope(261.8) %f\n",a.slope(261.8));
  tassert(fabs(a.elev(1)-3.3)<0.05);
  tassert(fabs(a.slope(261.8)+0.042)<0.001);
  sta=a.station(200);
  //printf("sta.x=%.17f sta.y=%.17f sta.z=%.17f \n",sta.east(),sta.north(),sta.elev());
  tassert(dist(sta,xyz(163.553,112.7825,32.167))<0.001);
  //printf("arc radius %f\n",a.radius(1));
  tassert(fabs(a.radius(0)-500)<0.001);
  tassert(fabs(a.curvature(0)-0.002)<0.000001);
  //printf("arc center %f,%f\n",a.center().east(),a.center().north());
  ctr=a.center();
  //printf("distance %f\n",dist(xy(sta),ctr));
  tassert(fabs(dist(xy(sta),ctr)-500)<0.001);
  tassert(fabs(ctr.east()+196.410)<0.001);
  tassert(fabs(ctr.north()-459.8075)<0.001);
  a.split(200,b,c);
  sta=a.station(200);
  //printf("a.station %f,%f,%f\n",sta.east(),sta.north(),sta.elev());
  sta=b.station(200);
  printf("b.station %f,%f,%f %f\n",sta.east(),sta.north(),sta.elev(),b.length());
  tassert(dist(b.station(123),a.station(123))<0.001);
  tassert(dist(c.station(200),a.station(400))<0.001);
  a.setslope(START,0);
  a.setslope(END,0);
  extrema=a.vextrema(true);
  tassert(extrema.size()==2);
  for (i=0;i<extrema.size();i++)
    cout<<"extrema["<<i<<"]="<<extrema[i]<<endl;
  tassert(extrema[1]>523 && extrema[1]<524);
  extrema=a.vextrema(false);
  tassert(extrema.size()==0);
  sta=xyz(150,200,5);
  b=arc(beg,sta,end);
  sta2=b.station(250);
  cout<<"arc3 "<<sta2.elev()<<endl;
  tassert(sta2.elev()==5);
  sta=xyz(150,200,10);
  b=arc(beg,sta,end);
  sta2=b.station(250);
  cout<<"arc3 "<<sta2.elev()<<endl;
  tassert(sta2.elev()==10);
  sta=xyz(200,150,10);
  b=arc(beg,sta,end);
  sta2=b.station(252.905);
  cout<<"arc3 "<<sta2.east()<<' '<<sta2.north()<<' '<<sta2.elev()<<endl;
  //cout<<dist(sta,sta2)<<endl;
  tassert(dist(sta,sta2)<0.001);
  a=arc(beg1,end1,3);
  sta=a.station(10);
  sta1=a.station(15);
  sta2=a.station(20);
  cout<<"arc4 "<<sta.east()<<' '<<sta.north()<<endl;
  cout<<"arc4 "<<sta1.east()<<' '<<sta1.north()<<endl;
  cout<<"arc4 "<<sta2.east()<<' '<<sta2.north()<<endl;
  xx=(sta.east()+sta1.east()+sta2.east())/25;
  tassert(xx>3.6e-9 && xx<3.7e-9);
  tassert(rint(sta.east()/xx)==8);
  tassert(rint(sta1.east()/xx)==9);
  tassert(rint(sta2.east()/xx)==8);
  a.setcurvature(0.01,0.01);
  cout<<"setcurvature: radius="<<a.radius(0)<<endl;
  tassert(abs(a.radius(0)-100)<0.0001);
}

void testspiralarc()
{
  int i,j,nfail;
  double bear[3],len;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta;
  xy ctr;
  spiralarc a(beg,end),b(beg,0.001,0.001,end),c,arch[10];
  bezier3d a3d;
  psopen("spiralarc.ps");
  psprolog();
  tassert(fabs(a.length()-500)<0.001);
  tassert(a.chordlength()==500);
  cout<<b.length()<<' '<<b.curvature(200)<<endl;
  tassert(fabs(b.length()-505.361)<0.001);
  tassert(fabs(b.curvature(200)-0.001)<0.000001);
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
  tassert(fabs(bear[2]-bear[0]-26)<1e-5);
  tassert(fabs(bear[0]+bear[2]-2*bintodeg(a.chordbearing())-8)<1e-5);
  cout<<"curvature at beg "<<a.curvature(0)<<endl;
  cout<<"curvature at end "<<a.curvature(len)<<endl;
  a.split(200,b,c);
  sta=a.station(123);
  printf("a.station %f,%f,%f\n",sta.east(),sta.north(),sta.elev());
  sta=b.station(123);
  printf("b.station %f,%f,%f %f\n",sta.east(),sta.north(),sta.elev(),b.length());
  tassert(dist(b.station(123),a.station(123))<0.001);
  tassert(dist(c.station(200),a.station(400))<0.001);
  for (i=-20,nfail=0;i<=20;i++)
  {
    for (j=-20;j<=20;j++)
    {
      a.setcurvature(i/1000.,j/1000.);
      if (a.valid())
      {
	tassert(fabs(a.curvature(0)-i/1000.)<1e-6);
	tassert(fabs(a.curvature(a.length())-j/1000.)<1e-6);
	cout<<'.';
      }
      else
      {
	nfail++;
	cout<<' ';
      }
    }
    cout<<endl;
  }
  cout<<"setcurvature: "<<nfail<<" failures"<<endl;
  tassert(nfail>656 && nfail<1066);
  startpage();
  setscale(-10,-10,10,10,degtobin(0));
  // Make something that resembles an Archimedean spiral
  //arch[0]=spiralarc(xyz(-0.5,0,0),2.,2/3.,xyz(1.5,0,0));
  arch[0]=spiralarc(xyz(-0.5,0,0),-DEG90,2.,2/3.,M_PI,0);
  for (i=1;i<10;i++)
    arch[i]=spiralarc(arch[i-1].getend(),arch[i-1].endbearing(),1/(i+0.5),1/(i+1.5),M_PI*(i+1),0);
  for (i=0;i<10;i++)
    a3d+=arch[i].approx3d(0.01);
  spline(a3d);
  for (i=0;i<-10;i++)
  {
    if (i&1)
      setcolor(1,0,0);
    else
      setcolor(0,0,1);
    spline(arch[i].approx3d(0.01));
  }
  cout<<"Archimedes-like spiral ended on "<<arch[9].getend().getx()<<','<<arch[9].getend().gety()<<endl;
  tassert(dist(arch[9].getend(),xy(-0.752,-10.588))<0.001);
  endpage();
  pstrailer();
  psclose();
}

void spiralmicroscope(spiralarc a,double aalong,spiralarc b,double balong,string fname,int scale=1)
{
  int i,alim,blim;
  xy point;
  double apow2,bpow2,ainc,binc;
  double minx=INFINITY,miny=INFINITY,maxx=-INFINITY,maxy=-INFINITY;
  vector<xy> apoints,bpoints;
  fname+=".ps";
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
    point=(a.station(aalong+ainc*i*scale));
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
    point=(b.station(balong+binc*i*scale));
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
  psopen(fname.c_str());
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

void testcogospiral1(spiralarc a,double a0,double a1,spiralarc b,double b0,double b1,bool extend,xy inter,string fname)
{
  int i;
  xy intpoint; // (7,11)
  vector<alosta> intlist;
  intlist=intersection1(a,a0,a1,b,b0,b1,extend);
  cout<<"testcogospiral: "<<intlist.size()<<" alostas"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlist.size();i++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlist[i].along<<' '<<ldecimal(intlist[i].station.east())<<' '<<ldecimal(intlist[i].station.north())<<endl;
    intpoint+=intlist[i].station;
  }
  intpoint/=i;
  if (inter.isfinite())
    tassert(dist(intpoint,inter)<1e-5);
  if (fname.length() && intlist.size())
    spiralmicroscope(a,intlist[0].along,b,intlist[1].along,"spiralmicro");
}

void testcogospiral()
{
  int i;
  xy intpoint; // (7,11)
  vector<alosta> intlist;
  xyz beg0(-1193,-489,0),end0(0xc07,0x50b,0), // slope 5/12
      beg1(-722,983,0),end1(382,-489,0), // slope -4/3
      beg2(-101,1,0),end2(99,1,0),beg3(-99,-1,0),end3(101,-1,0),
      beg4(-5,0,0),end4(5,0,0),beg5(-4,3,0),end5(4,3,0),
      beg6(5,0,0),end6(0,5,0),beg7(5,2,0),end7(4,9,0);
  segment a(beg0,end0),b(beg1,end1);
  spiralarc c(beg2,end2),d(beg3,end3),e(beg4,end4),f(beg5,end5),g(beg6,end6),h(beg7,end7);
  c.setdelta(0,DEG30);
  d.setdelta(0,-DEG30);
  e.setdelta(-DEG60,0);
  f.setdelta(DEG90,0); // e and f are 0.0034 away from touching
  g.setdelta(DEG90,0);
  h.setdelta(-DEG90,0); // g and h are tangent at (3,4)
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
  tassert(dist(intpoint,xy(7,11))<1e-5);
  testcogospiral1(spiralarc(a),0,a.length(),spiralarc(b),0,b.length(),false,xy(7,11),"straightmicro");
  /* The distances along the curves are in the hundreds, but the midpoints
   * are closer to the origin, so the two intersection points do not exactly
   * coincide. They aren't exactly (7,11) either, because the bearings have
   * been rounded to the nearest 1657th of a second.
   */
  testcogospiral1(c,50,c.length()-50,d,50,c.length()-50,false,xy(7.65883,-0.129029),"spiralmicro");
  testcogospiral1(e,0,1,f,0,1,false,xy(NAN,NAN),"");
  testcogospiral1(g,0,0.1,h,0,0.1,false,xy(NAN,NAN),"");
  /* These are tangent, so it can't find the intersection. As represented in the
   * computer, they are about 2e-9 apart.
   */
  //spiralmicroscope(g,3.2175055439642193,h,1.4189705460416392281,"tangentmicro",0x669);
  spiralmicroscope(g,3.2175384147219286,h,1.419003418926355,"tangentmicro",0x669);
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
    tassert(minquick>15);
  }
  pstrailer();
  psclose();
}

void testspiral()
{
  xy a,b,c,limitpoint;
  int i,j,bearing,lastbearing,curvebearing,diff,badcount;
  double t;
  float segalo[]={-5.96875,-5.65625,-5.3125,-4.875,-4.5,-4,-3.5,-2.828125,-2,0,
    2,2.828125,3.5,4,4.5,4.875,5.3125,5.65625,5.96875};
  vector<xy> spoints;
  vector<int> sbearings;
  bezier3d a3d;
  spiralarc sarc;
  a=cornu(0);
  tassert(a==xy(0,0));
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
   * and gets 14 bad bearings.
   * Running under Valgrind, the program says there are 10 bad bearings.
   */
  printf("%d bad bearings out of 118\n",badcount);
  tassert(badcount<=15);
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
	tassert(dist(c,b)>0.049 && dist(c,b)<=0.05);
	//cout<<dist(c,b)<<' ';
      }
      c=b;
    }
    //cout<<endl;
  }
  endpage();
  //startpage();
  /* Compare cornu(t,curvature,clothance) with cornu(t).
   * The code would draw a set of zero-length lines along a spiral.
   * As these would result in a blank page, it's commented out.
   */
  a=cornu(sqrt(M_PI*2));
  for (i=-20;i<21;i++)
  {
    b=cornu(i/20.,0,2);
    c=cornu(i/20.);
    //cout<<i<<' '<<dist(b,c)<<endl;
    tassert(dist(b,c)<1e-12); // it's less than 6e-17 on 64-bit
    b=cornu(i/20.,sqrt(M_PI*8),2);
    c=cornu(i/20.+sqrt(M_PI*2))-a;
    //cout<<i<<' '<<dist(b,c)<<endl;
    tassert(dist(b,c)<1e-12); // it's less than 1.1e-15 on 64-bit
    //line2p(b,c);
    b=cornu(i/20.,1,0);
    c=xy(0,1);
    //cout<<i<<' '<<dist(b,c)-1<<endl;
    tassert(fabs(dist(b,c)-1)<1e-12); // it's 0 or -1.11e-16 on 64-bit
  }
  //endpage();
  startpage();
  setscale(-1,-1,1,1,0);
  spoints.clear();
  for (i=0;i<sizeof(segalo)/sizeof(segalo[0]);i++)
  {
    spoints.push_back(cornu(segalo[i]));
    sbearings.push_back(ispiralbearing(segalo[i],0,2));
  }
  for (i=0;i<sizeof(segalo)/sizeof(segalo[0])-1;i++)
  {
    sarc=spiralarc(xyz(spoints[i],0),xyz(spoints[i+1],0));
    sarc.setdelta(sbearings[i+1]-sbearings[i],sbearings[i+1]+sbearings[i]-2*dir(spoints[i],spoints[i+1]));
    a3d+=sarc.approx3d(0.01);
  }
  spline(a3d);
  for (bearing=i=0,lastbearing=1;i<100 && bearing!=lastbearing;i++)
  {
    t=bintorad(bearing);
    a=cornu(-sqrt(t));
    b=cornu(sqrt(t+M_PI/2));
    lastbearing=bearing;
    bearing=dir(a,b);
  }
  dot(limitpoint);
  dot(-limitpoint);
  setcolor(0,0,1);
  line2p(a,b);
  endpage();
  pstrailer();
  psclose();
  tassert(bearing==162105696);
  printf("Maximum useful t of spiral is %f\n",sqrt(t+M_PI/2));
}

void testarea3()
{
  int i,j,itype;
  xy a(0,0),b(4,0),c(0,3),d(4,4),e;
  tassert(area3(c,a,b)==6);
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
  tassert(elev==0);
  // Now make a constant surface at elevation 1.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(1);
  doc.pl[0].points[3].setelev(1);
  for (i=0;i<7;i++)
    tri.ctrl[i]=1;
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  tassert(elev==1);
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
  tassert(abs(elev*3-1)<1e-7);
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
  tassert(abs(elevd-elev*3+elevg*3-eleva)<1e-7);
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
  tassert(abs(elevd-elev*3+elevg*3-eleva)<1e-7);
  tassert(abs(elev-1)<1e-7);
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
  tassert(qinx.side==1);
  doc.pl[1].maketin();
  doc.pl[1].maketriangles();
  printf("%d triangle, should be 1\n",(int)doc.pl[1].triangles.size());
  tassert(doc.pl[1].triangles.size()==1);
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
  tassert(qs>=79 && qs<=133);
  qinx.draw();
  endpage();
  startpage();
  hilbertpath=qinx.traverse();
  tassert(hilbertpath.size()==qs);
  setscale(1,-7,31,23);
  qinx.draw();
  setcolor(0,0,1);
  for (i=1,pathlength=0;i<hilbertpath.size();i++)
  {
    line2p(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
    pathlength+=dist(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
  }
  printf("pathlength %f\n",pathlength);
  tassert(pathlength>100 && pathlength<400);
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
  tassert(ntri>i/2);
  setcolor(1,0,0);
  for (i=0;i<doc.pl[1].triangles.size();i++)
  {
    tassert(doc.pl[1].triangles[i].area()>0);
    //printf("tri %d area %f\n",i,doc.pl[1].triangles[i].area());
    dot(doc.pl[1].triangles[i].centroid());
  }
  printf("%d triangles\n",i);
  tassert(ntri==i*3); // ntri is the number of sides of edges which are linked to a triangle
  endpage();
  ptri=&doc.pl[1].triangles[0];
  ptri=ptri->findt(bone1);
  tassert(ptri->in(bone1));
  tassert(!ptri->in(bone2));
  ptri=ptri->findt(bone2);
  tassert(ptri->in(bone2));
  tassert(!ptri->in(bone1));
  tassert(ptri->findt(bone3,true));
  tassert(!ptri->findt(bone3,false));
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
  tassert(pathlength>50 && pathlength<250);
  endpage();
  ptri=qinx.findt(bone1);
  tassert(ptri->in(bone1));
  tassert(!ptri->in(bone2));
  ptri=qinx.findt(bone2);
  tassert(ptri->in(bone2));
  tassert(!ptri->in(bone1));
  tassert(qinx.findt(bone3,true));
  tassert(!qinx.findt(bone3,false));
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
  tassert(grad63==grad63half*2);
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
  segment clipped;
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
    tassert(ptype!=PT_SLOPE && ptype!=PT_GRASS);
    // On the Raspberry Pi, doing slope, this fails with ptype==PT_GRASS for reasons not yet known.
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
  tassert(size0==size2);
  cout<<size1-size0<<" monotonic segments in perimeter"<<endl;
  lh=doc.pl[1].triangles[0].lohi();
  cout<<"lohi: "<<setprecision(7)<<lh[0]<<' '<<lh[1]<<' '<<lh[2]<<' '<<lh[3]<<endl;
  for (j=0;j<doc.pl[1].triangles[0].subdiv.size();j++)
    spline(doc.pl[1].triangles[0].subdiv[j].approx3d(1));
  clipped=doc.pl[1].triangles[0].dirclip(xy(1,2),AT34);
  setcolor(1,0,1);
  spline(clipped.approx3d(1));
  endpage();
  psclose();
  cout<<fname<<endl;
  if (crits.size()!=excrits && excrits>=0)
    cout<<crits.size()<<" critical points found, "<<excrits<<" expected"<<endl;
  tassert(crits.size()==excrits || excrits<0);
  testpointedg();
}

void test1grad()
/* Test the method triangle::gradient, which is in bezier.cpp .
 * Compares the output of triangle::gradient with the gradient measured
 * by sampling four points.
 */
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
    tassert(dist(xy(deriv1(xsect),deriv1(ysect)),grad2)<1e-6);
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
 * tri93cdd9cfae9a662e49 also has three critical points inside the triangle.
 * tri93db9159a54b09a0fc has two critical points in the triangle, both saddles,
 * two critical points on two sides, and one critical point on the third side.
 * tri45653a366286ff0747 also has two saddles in the triangle.
 * tri083194be44b7dace28 has a contour that does not touch the perimeter and
 * a contour at the same elevation that does.
 * So do tri2c7ae0064f1de9f5b2 and trie1fbf94c3a1f855e9c.
 * tri714b40f2e8ee8c56b4 has three critical points clearly inside the triangle
 * and one just outside the border.
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
  tassert(deriv3(con)==0);
  tassert(deriv3(lin)==0);
  tassert(deriv3(quad)==0);
  tassert(deriv3(cub)==6);
  tassert(deriv2(con)==0);
  tassert(deriv2(lin)==0);
  tassert(deriv2(quad)==2);
  tassert(deriv2(cub)==0);
  tassert(deriv1(con)==0);
  tassert(deriv1(lin)==1);
  tassert(deriv1(quad)==0);
  tassert(deriv1(cub)==0);
  tassert(deriv0(con)==1);
  tassert(deriv0(lin)==0);
  tassert(deriv0(quad)==0);
  tassert(deriv0(cub)==0);
  cout<<"Zeroth derivative of constant "<<deriv0(con)<<endl;
  cout<<"First derivative of line "<<deriv1(lin)<<endl;
  cout<<"Second derivative of square "<<deriv2(quad)<<endl;
  cout<<"Third derivative of cube "<<deriv3(cub)<<endl;
  para1.push_back(4);
  para1.push_back(1);
  para1.push_back(0);
  para1.push_back(1);
  cout<<"Vertex "<<paravertex(para1)<<endl;
  tassert(paravertex(para1)==0.5);
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
  xyz beg(-3,4,0),end(3,-4,0);
  spiralarc s(beg,end);
  doc.pl[1].clear();
  aster(doc,100);
  bound=doc.pl[1].dirbound(degtobin(0));
  for (i=1;i<=100;i++)
    if (bound==doc.pl[1].points[i].east())
      printf("westernmost point is %d\n",i);
  tassert(bound==doc.pl[1].points[94].east());
  bound=doc.pl[1].dirbound(degtobin(90));
  for (i=1;i<=100;i++)
    if (bound==doc.pl[1].points[i].north())
      printf("southernmost point is %d\n",i);
  tassert(bound==doc.pl[1].points[96].north());
  s.setdelta(DEG180,0);
  bound=s.dirbound(0);
  cout<<"westernmost bound of semicircle is "<<ldecimal(bound)<<endl;
  tassert(fabs(bound+5)<1e-8);
  bound=s.dirbound(DEG90);
  cout<<"southernmost bound of semicircle is "<<ldecimal(bound)<<endl;
  tassert(fabs(bound+5)<1e-8);
  s.setdelta(0,DEG180);
  bound=s.dirbound(0);
  cout<<"westernmost bound of spiral is "<<ldecimal(bound)<<endl;
  tassert(fabs(bound+3)<1e-8);
  bound=s.dirbound(DEG90);
  cout<<"southernmost bound of spiral is "<<ldecimal(bound)<<endl;
  tassert(fabs(bound+4.29208)<1e-6);
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
  tassert(btreversetable[13]==37296);
  /* 13 is 00001101 (2) and 00111 (3).
   * 37296%256=176
   * 37296%243=117
   * 176 is 10110000 (2); 117 is 11100 (3).
   */
  for (i=0;i<62208;i++)
    tassert(btreversetable[btreversetable[i]]==i);
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
  polyspiral r;
  cout<<"testpolyline"<<endl;
  r.smooth(); // sets the curvy flag
  bendlimit=DEG180+7;
  p.insert(xy(0,0));
  q.insert(xy(0,0));
  r.insert(xy(0,0));
  p.insert(xy(3,0));
  q.insert(xy(3,0));
  r.insert(xy(3,0));
  cout<<p.length()<<' '<<r.length()<<endl;
  tassert(p.length()==6);
  tassert(fabs(r.length()-3*M_PI)<1e-6);
  p.insert(xy(3,4));
  q.insert(xy(3,4));
  r.insert(xy(3,4));
  cout<<p.length()<<endl;
  tassert(p.length()==12);
  q.setlengths();
  tassert(q.length()==12);
  tassert(q.area()==6);
  tassert(fabs(r.length()-5*M_PI)<1e-6);
  q.open();
  tassert(q.length()==7);
  tassert(std::isnan(q.area()));
  q.close();
  q.setlengths();
  tassert(q.length()==12);
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
  tassert(fabs(q.length()-M_PI*5)<0.0005);
  tassert(fabs(q.area()-M_PI*6.25)<0.0005);
  cout<<q.getarc(0).center().north()<<endl;
  cout<<q.length()<<endl;
  bendlimit=DEG120;
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
  tassert(a.size()==1);
  pt=a.station(0.4);
  pt1=xyz(1.2,1.44,1.728);
  tassert(dist(pt,pt1)<1e-6);
  pt=a.station(1);
  pt1=xyz(3,9,27);
  tassert(dist(pt,pt1)<1e-6);
  c=a+b;
  pt=c.station(1.5);
  pt1=b.station(.5);
  cout<<pt.east()<<' '<<pt.north()<<' '<<pt.elev()<<endl;
  tassert(pt==pt1);
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
      tassert(dists[clothance*M_PI+curvature]<=ests[clothance*M_PI+curvature]);
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
  cout<<ngood<<" good spirals"<<endl;
  cout<<nclose<<" with 1/3 point close"<<endl;
  tassert(ngood>=107);
  tassert(nclose>=30);
  startpage();
  setscale(-30,-70,30,70,DEG90);
  for (i=-18,nclose=0;i<19;i+=2)
  {
    switch (i%3)
    {
      case 0:
	setcolor(1,0,0);
	break;
      case 1:
      case -2:
	setcolor(0,0.5,0);
	break;
      case -1:
      case 2:
	setcolor(0,0,1);
	break;
    }
    spiralarc0.setcurvature(i/1e3,i/1e3);
    c=spiralarc0.approx3d(1);
    cout<<i<<"/1000 C curvature 1 m approx "<<c.size();
    c=spiralarc0.approx3d(0.001);
    cout<<" splines; 1 mm approx "<<c.size()<<" splines"<<endl;
    pt=spiralarc0.station(arc0.length()/3);
    pt1=c.station(c.size()/3.);
    //cout<<"distance "<<dist(pt,pt1)<<endl;
    nclose+=(dist(pt,pt1)<1);
    spline(c);
  }
  endpage();
  startpage();
  setscale(-30,-70,30,70,DEG90);
  for (i=-100,nclose=0;i<101;i+=5)
  {
    switch (i%3)
    {
      case 0:
	setcolor(1,0,0);
	break;
      case 1:
      case -2:
	setcolor(0,0.5,0);
	break;
      case -1:
      case 2:
	setcolor(0,0,1);
	break;
    }
    spiralarc0.setcurvature(i/1e3,-i/1e3);
    c=spiralarc0.approx3d(1);
    cout<<i<<"/1000 S curvature 1 m approx "<<c.size();
    c=spiralarc0.approx3d(0.001);
    cout<<" splines; 1 mm approx "<<c.size()<<" splines"<<endl;
    pt=spiralarc0.station(arc0.length()/3);
    pt1=c.station(c.size()/3.);
    //cout<<"distance "<<dist(pt,pt1)<<endl;
    nclose+=(dist(pt,pt1)<1);
    if (spiralarc0.valid())
      spline(c);
  }
  setcolor(0,0,0);
  endpage();
  pstrailer();
  psclose();
}

void testangleconvcorner(string anglestr,xyz &totxyz)
{
  xyz corner;
  latlong ll;
  ll=parselatlong(anglestr,DEGREE);
  corner=Sphere.geoc(ll,0);
  tassert(fabs(corner.getx())>3678296 && fabs(corner.gety())>3678296 && fabs(corner.getz())>3678296);
  totxyz+=corner;
}

void testangleconv()
{
  string strang,straz,strbear;
  int angle;
  double distance;
  xyz totxyz;
  latlong ll,wrangell0,wrangell1;
  strang=bintoangle(0,DEGREE+SEXAG2);
  cout<<strang<<endl;
  tassert(strang=="0°00′00″");
  strang=bintoangle(degtobin(90),DEGREE+SEXAG2);
  cout<<strang<<endl;
  tassert(strang=="90°00′00″");
  strang=bintoangle(degtobin(-80),DEGREE+SEXAG2);
  cout<<strang<<endl;
  tassert(strang=="-80°00′00″");
  strang=bintoangle(atan2i(2,1),DEGREE+SEXAG2);
  cout<<strang<<endl;
  strang=bintoangle(atan2i(2,1),DEGREE+SEXAG2P2);
  cout<<strang<<endl;
  strang=bintoangle(atan2i(2,1),GON+DEC4);
  cout<<strang<<endl;
  angle=parseiangle("63°26′05.82″",DEGREE);
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
  ll=parselatlong("126°W 55°N",DEGREE);
  cout<<formatlatlong(ll,DEGREE)<<endl;
  wrangell0=parselatlong("143°52'11.5\"W 61°56'51\"N",DEGREE);
  cout<<formatlatlong(wrangell0,DEGREE+DEC5)<<endl;
  wrangell1=parselatlong("143.86986°W 61.9475°N",DEGREE);
  cout<<formatlatlong(wrangell1,DEGREE+SEXAG2P2)<<endl;
  distance=dist(WGS84.geoc(wrangell0,0),WGS84.geoc(wrangell1,0));
  cout<<"Distance between DMS and decimal: "<<distance<<endl;
  tassert(distance<1);
  /* Add up the eight points where faces of volleyball coordinates meet
   * (45°,135°E,W 35.26439°,35°15'52"N,S). The sum should be close to zero.
   * The ones in the northern hemisphere have the latitude in DMS;
   * the ones with longitude 135 have longitude first.
   */
  testangleconvcorner("35°15'52\"N45°E",totxyz);
  testangleconvcorner("135°E35°15'52\"N",totxyz);
  testangleconvcorner("135°W35°15'52\"N",totxyz);
  testangleconvcorner("35°15'52\"N45°W",totxyz);
  testangleconvcorner("35.26439°S45°E",totxyz);
  testangleconvcorner("135°E35.26439°S",totxyz);
  testangleconvcorner("135°W35.26439°S",totxyz);
  testangleconvcorner("35.26439°S45°W",totxyz);
  cout<<totxyz.length()<<endl;
  tassert(totxyz.length()<30);
}

void testcsvline()
{
  vector<string> words;
  string line;
  words=parsecsvline(line="");
  tassert(words.size()==0);
  tassert(makecsvline(words)==line);
  words=parsecsvline(line="\"\"");
  tassert(words.size()==1);
  //cout<<makecsvline(words)<<endl;
  tassert(makecsvline(words)==line);
  words=parsecsvline(line="\"pote\"\"mkin\"");
  tassert(words[0].length()==9);
  tassert(makecsvline(words)==line);
  words=parsecsvline(line="\"3,4\",\"5,12\"");
  tassert(words.size()==2);
  tassert(makecsvline(words)==line);
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
  tassert(ldecimal(0)=="0");
  tassert(ldecimal(1)=="1");
  tassert(ldecimal(-1)=="-1");
  tassert(ldecimal(1.7320508)=="1.7320508");
  tassert(ldecimal(-0.00064516)=="-.00064516");
  tassert(ldecimal(3628800)=="3628800");
  tassert(ldecimal(1296000)=="1296e3");
  tassert(ldecimal(0.000016387064)=="1.6387064e-5");
}

void testellipsoid()
{
  double rad;
  xyz sealevel,kmhigh,noffset,soffset,diff,benin,bengal,howland,galapagos,npole,spole;
  ellipsoid test1(8026957,0,0.5),test2(8026957,4013478.5,0);
  tassert(test1.geteqr()==test2.geteqr());
  tassert(test1.getpor()==test2.getpor());
  sealevel=test1.geoc(degtobin(45),0,0);
  kmhigh=test1.geoc(degtobin(45),0,65536000);
  diff=kmhigh-sealevel;
  cout<<diff.east()<<' '<<diff.north()<<' '<<diff.elev()<<endl;
  tassert(abs(diff.east()-707.107)<0.001);
  tassert(abs(diff.elev()-707.107)<0.001);
  cout<<sealevel.east()<<' '<<sealevel.north()<<' '<<sealevel.elev()<<' '<<ldecimal(sealevel.east()/sealevel.elev())<<endl;
  tassert(fabs(sealevel.east()/sealevel.elev()-4)<1e-9);
  noffset=test1.geoc(degtobin(45)+1,0,0);
  soffset=test1.geoc(degtobin(45)-1,0,0);
  diff=noffset-soffset;
  cout<<diff.east()<<' '<<diff.north()<<' '<<diff.elev()<<endl;
  tassert(abs(diff.east()+diff.elev())<1e-6);
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
  tassert(benin.getx()>6370999);
  tassert(howland.getx()<-6370999);
  tassert(bengal.gety()>6370999);
  tassert(galapagos.gety()<-6370999);
  tassert(npole.getz()>6370999);
  tassert(spole.getz()<-6370999);
  rad=GRS80.radiusAtLatitude(latlong(0,0),0);
  cout<<"Radius in prime at equator: "<<ldecimal(rad)<<endl;
  tassert(fabs(rad-6378137)<0.5);
  rad=GRS80.radiusAtLatitude(latlong(0,0),DEG90);
  cout<<"Radius in meridian at equator: "<<ldecimal(rad)<<endl;
  tassert(fabs(rad-6335439)<0.5);
  rad=GRS80.radiusAtLatitude(latlong(degtorad(41+18./60+15.0132/3600),2.5),degtobin(-52-14./60-36./3600));
  // Elementary Surveying, 11th ed., example 19.2, page 548
  cout<<"Radius in azimuth 142°14'36\" at 41°18'15\": "<<ldecimal(rad)<<endl;
  tassert(fabs(rad-6372309.4)<0.5);
}

void test1projection(string projName,Projection &proj,latlong ll,xy grid)
{
  latlong ll1=proj.gridToLatlong(grid);
  xy grid1=proj.latlongToGrid(ll);
  xyz gridGeoc=proj.ellip->geoc(ll1,0);
  xyz llGeoc=proj.ellip->geoc(ll,0);
  cout<<setprecision(10);
  cout<<projName<<" Latitude "<<radtodeg(ll1.lat)<<" Longitude "<<radtodeg(ll1.lon)<<
  " Northing "<<grid1.north()<<" Easting "<<grid1.east()<<endl;
  tassert(dist(grid,grid1)<1.5);
  tassert(dist(gridGeoc,llGeoc)<1.75);
}

array<latlong,2> randomPointPair()
/* Pick a point on the sphere according to the spherical asteraceous pattern.
 * Then pick two points about a meter apart. The distance between them is
 * 1±3e-9 m on the sphere.
 */
{
  int r1,r2;
  latlong midpoint;
  double latoff,lonoff;
  array<latlong,2> ret;
  r1=rng.usrandom();
  r2=rng.usrandom();
  r1=(r1<<8)+(r2>>8);
  r2&=255;
  midpoint.lat=asin((2*r1+1)/16777216.-1);
  midpoint.lon=M_1PHI*(2*r1+1)/2.;
  midpoint.lon-=rint(midpoint.lon);
  midpoint.lon*=2*M_PI;
  latoff=sin(M_PI*r2/256)/12742e3;
  lonoff=cos(M_PI*r2/256)/12742e3/cos(midpoint.lat);
  ret[0].lat=midpoint.lat-latoff;
  ret[0].lon=midpoint.lon-lonoff;
  ret[1].lat=midpoint.lat+latoff;
  ret[1].lon=midpoint.lon+lonoff;
  return ret;
}

void testprojscale(string projName,Projection &proj)
/* Tests the accuracy of a projection's scale. Implicitly tests conformality.
 * Picks random pairs of points 1 meter apart and checks that the distance
 * between the two points on the map is close to the map scale. Allows a few
 * bad pairs, as the two points may straddle a cut in the map.
 */
{
  array<latlong,2> pointpair;
  latlong midpoint;
  array<xyz,2> xyzpair;
  array<xy,2> xypair;
  int i,nbad;
  double scale;
  for (i=nbad=0;i<16777216 && nbad>=trunc(sqrt(i)/16);i++)
  {
    pointpair=randomPointPair();
    midpoint.lat=(pointpair[0].lat+pointpair[1].lat)/2;
    midpoint.lon=(pointpair[0].lon+pointpair[1].lon)/2;
    xyzpair[0]=proj.ellip->geoc(pointpair[0],0);
    xyzpair[1]=proj.ellip->geoc(pointpair[1],0);
    scale=proj.scaleFactor(midpoint);
    xypair[0]=proj.latlongToGrid(pointpair[0]);
    xypair[1]=proj.latlongToGrid(pointpair[1]);
    if (fabs(dist(xypair[0],xypair[1])/scale/dist(xyzpair[0],xyzpair[1])-1)>1e-6)
    {
      nbad++;
      if (nbad<256)
	cout<<radtodeg(midpoint.lat)<<"° "<<radtodeg(midpoint.lon)<<"° computed scale "<<
	scale<<" actual scale "<<dist(xypair[0],xypair[1])/dist(xyzpair[0],xyzpair[1])<<endl;
      }
  }
  cout<<projName<<" scale is ";
  if (nbad>=trunc(sqrt(i)/16))
    cout<<"bad"<<endl;
  else
    cout<<"good"<<endl;
}

void testprojection()
{
  LambertConicSphere sphereMercator;
  latlong ll;
  xy grid;
  cout<<"projection"<<endl;
  ll.lat=0;
  ll.lon=0;
  grid=xy(0,0);
  test1projection("sphereMercator",sphereMercator,ll,grid);
  ll.lon=degtorad(1);
  grid=xy(111195,0);
  test1projection("sphereMercator",sphereMercator,ll,grid);
  ll.lat=degtorad(1);
  grid=xy(111195,111201);
  test1projection("sphereMercator",sphereMercator,ll,grid);
  ll.lat=degtorad(89);
  ll.lon=degtorad(-45);
  grid=xy(-5003772,30207133);
  test1projection("sphereMercator",sphereMercator,ll,grid);
  testprojscale("sphereMercator",sphereMercator);
}

void spotcheckcolor(int col0,int col1)
{
  int col2;
  col2=printingcolor(col0,4);
  if (col2!=col1)
    cout<<hex<<col0<<" gives "<<col2<<", should be "<<col1<<endl<<dec;
  tassert(col1==col2);
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
    tassert(hist[i]==0);
  // fliphue is the identity function on the gray axis.
  for (i=0;i<256;i++)
  {
    cint=i*0x010101;
    cint1=printingcolor(cint,4);
    tassert(cint==cint1);
  }
  for (i=0;i<1000;i++)
  {
    cint=(rng.ucrandom()<<16)+rng.usrandom();
    cint1=cint^16777215;
    csht=colorshort(cint);
    csht1=colorshort(cint1);
    tassert(csht+csht1==63999);
    csht=rng.usrandom();
    csht1=colorshort(colorint(csht));
    tassert(csht==csht1);
    if (csht<64000)
    {
      csht1=63999-csht;
      cint=colorint(csht);
      cint1=colorint(csht1);
      tassert(cint+cint1==16777215);
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
  ofstream ofile("contour.bez");
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
  checkedgediscrepancies(doc.pl[1]);
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
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  startpage();
  setscale(-10,-10,10,10,0);
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"contour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  //psclose();
  smoothcontours(doc.pl[1],conterval,false);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  pstrailer();
  psclose();
  doc.writeXml(ofile);
}

void testfoldcontour()
/* This is a test of one triangle from Independence Park in which the contours
 * bend through angles of at least 135° and are drawn badly. The triangle
 * is on the northeast side. There is one very thin triangle between it and the
 * outside.
 */
{
  int i,j;
  double conterval;
  ofstream ofile("foldcontour.bez");
  psopen("foldcontour.ps");
  psprolog();
  startpage();
  setscale(194,-143,221,182,-DEG60);
  doc.pl[1].clear();
  doc.pl[1].addpoint(1,point(-56.185204978391994,267.41484378968016,206.0516647193294,"BC")); // 956
  doc.pl[1].addpoint(2,point(13.2558628396946,217.37694386590738,208.42972517145031,"TP L")); // 1112
  doc.pl[1].addpoint(3,point(234.44101498596137,119.28953213107889,211.83558242316485,"EP")); // 430
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].points[1].gradient=xy(.023188005886758664,-.04617177625035849);
  doc.pl[1].points[2].gradient=xy(.01926493749884871,-.035692258139347684);
  doc.pl[1].points[3].gradient=xy(.012156892166327406,-.024249907326683827);
  doc.pl[1].maketriangles();
  /*doc.pl[1].triangles[0].ctrl[0]=207.35851092221836;
  doc.pl[1].triangles[0].ctrl[0]=210.57774846809454;
  doc.pl[1].triangles[0].ctrl[0]=207.38847734738403;
  doc.pl[1].triangles[0].ctrl[0]=209.6010286337486;
  doc.pl[1].triangles[0].ctrl[0]=209.46053685740776;
  doc.pl[1].triangles[0].ctrl[0]=211.01708495532736;
  doc.pl[1].triangles[0].ctrl[0]=210.1464041938586;*/
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  startpage();
  setscale(194,-143,221,182,-DEG60);
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  pstrailer();
  psclose();
  doc.writeXml(ofile);
}

void testtracingstop()
/* This is a test of one triangle from Independence Park in which the tracing
 * of the contour of elevation 205.6 starts at the side and gets lost in a loop
 * in the middle. Neither it nor any of its three neighbors are acicular.
 */
{
  int i,j;
  double conterval;
  ofstream ofile("tracingstop.bez");
  psopen("tracingstop.ps");
  psprolog();
  startpage();
  setscale(144,51,147,54,0);
  doc.pl[1].clear();
  doc.pl[1].addpoint(1,point(146.51216865633614,53.34791973582469,205.6513304546609,"CENTER SW")); // 697
  doc.pl[1].addpoint(2,point(144.90511658618925,51.606410972832236,205.68529743459484,"SW")); // 681
  doc.pl[1].addpoint(3,point(145.5184577088803,51.65092405382893,205.6918171196342,"SW/STEP")); // 564
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].points[1].gradient=xy(.02316198872560679,.04076481613085247);
  doc.pl[1].points[2].gradient=xy(-.10223642909422531,-.1520591866033225);
  doc.pl[1].points[3].gradient=xy(-.21781342189999459,-.21611388062984482);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  startpage();
  setscale(144,51,147,54,0);
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  pstrailer();
  psclose();
  doc.writeXml(ofile);
}

void testzigzagcontour()
/* This is a test of one triangle from Sandymush (Burnt Chimney job 3608)
 * in which the contours are drawn with erroneous zigzags and cross.
 * When isolated, this triangle is drawn correctly.
 */
{
  int i,j;
  double conterval;
  ofstream ofile("zigzagcontour.bez");
  psopen("zigzagcontour.ps");
  psprolog();
  startpage();
  setscale(15111,14793,15346,15108,0);
  doc.pl[1].clear();
  doc.pl[1].addpoint(1,point(15345.127559055116,15064.447223774447,281.5871780543561,"VRS DEL")); // 1009
  doc.pl[1].addpoint(2,point(15202.258582677165,15107.048442976886,280.3079298958598,"OL PK397")); // 398
  doc.pl[1].addpoint(3,point(15111.2489001778,14793.022799085598,280.59313182626363,"DEL 2004")); // 1
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].points[1].gradient=xy(.006568946045497389,.003201194208355324);
  doc.pl[1].points[2].gradient=xy(-.000821970759633599,.007101097679719867);
  doc.pl[1].points[3].gradient=xy(.0025853345946618457,-3.132437738730802e-5);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"zigzagcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  startpage();
  setscale(15111,14793,15346,15108,0);
  setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"zigzagcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    spline(doc.pl[1].contours[i].approx3d(1));
  }
  endpage();
  pstrailer();
  psclose();
  doc.writeXml(ofile);
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
  tassert(m==0);
  m=minquad(0,0,1,1,2,4);
  cout<<"m="<<m<<endl;
  tassert(m==0);
  m=minquad(0,0,2,4,4,16);
  cout<<"m="<<m<<endl;
  tassert(m==0);
  m=minquad(0,0,1,1,3,9);
  cout<<"m="<<m<<endl;
  tassert(m==0);
  m=minquad(-8,64,13,169,21,441);
  cout<<"m="<<m<<endl;
  tassert(m==0);
}

void testroscat()
{
  xy xy0(32,27),xy1(16,15),xy2(46,58),xy3(58,73),xy4(84,12),xy5(79,47),
     org(0,0),piv0(27,27),piv1(12,12);
  point pt0(32,27,99,""),pt1(16,15,99,""),pt2(46,58,99,""),pt3(58,73,99,"");
  xy0.roscat(piv0,AT34,1,piv1);
  tassert(dist(xy0,xy1)<1e-6);
  cout<<"roscat dist "<<dist(xy0,xy1)<<endl;
  xy2.roscat(org,0,CBRT2,org);
  tassert(dist(xy2,xy3)<1e-1);
  cout<<"roscat dist "<<dist(xy2,xy3)<<endl;
  xy4.roscat(org,AT0512,13/12.,org);
  tassert(dist(xy4,xy5)<1e-6);
  cout<<"roscat dist "<<dist(xy4,xy5)<<endl;
  pt0.roscat(piv0,AT34,1,piv1);
  tassert(dist(pt0,pt1)<1e-6);
  cout<<"roscat dist "<<dist(pt0,pt1)<<endl;
  pt2.roscat(org,0,CBRT2,org);
  tassert(dist(pt2,pt3)<1e-1);
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
  tassert(fabs(ssd-1000)<1e-9);
}

void test1bicubic(xy sw)
{
  xy se,nw,ne;
  double vertex;
  se=sw+xy(1,0);
  nw=sw+xy(0,1);
  ne=sw+xy(1,1);
  vertex=bicubic(sqr(sw.length()),2*sw,sqr(se.length()),2*se,sqr(nw.length()),2*nw,sqr(ne.length()),2*ne,-sw.getx(),-sw.gety());
  cout<<vertex<<endl;
  tassert(fabs(vertex)<1e-12);
}

void testbicubic()
{
  double bicube;
  cout<<"bicubic"<<endl;
  test1bicubic(xy(-0.5,-0.5));
  test1bicubic(xy(-0.5,-0.3));
  test1bicubic(xy(-0.5,0));
  test1bicubic(xy(-0.4,-0.3));
  bicube=bicubic(0,xy(0,0),0,xy(0,0),0,xy(0,0),1,xy(3,3),2./3,5./7);
  cout<<bicube<<endl;
  /* This gives -800/9621, not 1000/9621. I don't know what's correct, but
   * suspect that 1000/9621 is wrong because the inner control points don't
   * have full freedom.
   */
}

void testhistogram()
{
  histogram histo0(-1,1),histo1(-0.1,0.1),histo2(-10,10);
  histobar bar;
  int i,bartot;
  double x;
  for (i=0;i<1000;i++)
  {
    x=sin((double)i);
    histo0<<x;
    histo1<<x;
    histo2<<x;
  }
  for (i=bartot=0;i<histo0.nbars();i++)
  {
    bar=histo0.getbar(i);
    bartot+=bar.count;
    cout<<bar.start<<endl<<setw(6)<<bar.count<<' '<<bar.count/(bar.end-bar.start)<<endl;
    tassert(bar.start<bar.end);
  }
  cout<<bar.end<<endl;
  tassert(histo0.gettotal()==bartot);
  for (i=bartot=0;i<histo1.nbars();i++)
  {
    bar=histo1.getbar(i);
    bartot+=bar.count;
    cout<<bar.start<<endl<<setw(6)<<bar.count<<' '<<bar.count/(bar.end-bar.start)<<endl;
    tassert(bar.start<bar.end);
  }
  cout<<bar.end<<endl;
  tassert(histo1.gettotal()==bartot);
  for (i=bartot=0;i<histo2.nbars();i++)
  {
    bar=histo2.getbar(i);
    bartot+=bar.count;
    cout<<bar.start<<endl<<setw(6)<<bar.count<<' '<<bar.count/(bar.end-bar.start)<<endl;
    tassert(bar.start<bar.end);
  }
  cout<<bar.end<<endl;
  tassert(histo2.gettotal()==bartot);
}

void testsmooth5()
{
  int i,nsmooth=0;
  for (i=17711;i<=28657;i++)
  {
    if (smooth5(i))
    {
      nsmooth++;
      cout<<i<<' ';
    }
  }
  cout<<endl<<nsmooth<<" smooth numbers"<<endl;
  tassert(nsmooth==26);
}

void testquadhash()
{
  int i,j,l,lastang=-1,hash;
  int n,lastn=300;
  map<double,int> langles;
  map<double,int>::iterator k;
  map<int,double> hashmap;
  vector<int> btangles;
  double all256[16][16],some256[16][16];
  double x,y,ofs,s,c,sum;
  for (i=1;i<16;i++)
    for (j=1;j<=i;j++)
      if (gcd(j,i)==1)
      {
	//cout<<j<<"/"<<i<<endl;
	langles[j/(double)i]=atan2i(j,i);
      }
  langles[0]=0;
  for (k=langles.begin();k!=langles.end();k++)
  {
    //cout<<k->first<<' '<<k->second<<endl;
    if (lastang>=0)
    {
      //cout<<(lastang+k->second)/2<<endl;
      btangles.push_back((lastang+k->second)/2);
    }
    lastang=k->second;
  }
  for (i=71;i>=0;i--)
    btangles.push_back(DEG90-btangles[i]);
  for (i=0;i<432;i++)
    btangles.push_back(btangles[i]+DEG90);
  // The total number of between angles is 576.
  cout<<btangles.size()<<endl;
  for (i=0;i<16;i++)
    for (j=0;j<16;j++)
      all256[i][j]=sin(i-7.5+(j-7.5)*M_1PHI+M_PI/4);
  for (l=0;l<576;l++)
  {
    s=sin(btangles[l]);
    c=cos(btangles[l]);
    for (ofs=0;ofs<10.6;ofs+=0.0234375)
    {
      sum=n=0;
      for (i=0;i<16;i++)
      {
	y=i-7.5;
	for (j=0;j<16;j++)
	{
	  x=j-7.5;
	  if (c*x+s*y>ofs)
	    some256[i][j]=NAN;
	  else
	  {
	    sum+=some256[i][j]=all256[i][j];
	    n++;
	  }
	}
      }
      if (n-lastn>1)
	cout<<"Skipped in direction "<<l<<endl;
      lastn=n;
      hash=quadhash(some256);
      if (hashmap[hash]!=0 && hashmap[hash]!=sum)
      {
	cout<<"Hash collision: "<<hash<<' '<<l<<' '<<ofs<<endl;
      }
      hashmap[hash]=sum;
    }
  }
  cout<<hashmap.size()<<" different hashes"<<endl;
  tassert(hashmap.size()==20173);
}

void testgeoid()
{
  vball v;
  int lat,lon,olat,olon,i,j,k;
  double x,y,sum,qpoints[16][16],u0,u1;
  //vector<double> anga,apxa;
  xyz dir;
  geoquad gq,*pgq;
  geoheader hdr;
  fstream file;
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
      tassert(olat==lat && olon==lon);
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
  cout<<"Testing correction..."<<dec<<endl;
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
  gq.clear();
  pgq=&gq;
  for (i=0;i<24;i++)
  {
    cout<<setw(2)<<i<<setprecision(10)
    <<setw(18)<<pgq->apxarea()/sqr(pgq->scale)
    <<setw(18)<<pgq->angarea()/sqr(pgq->scale)
    <<setw(18)<<pgq->area()/sqr(pgq->scale)
    <<setw(18)<<setprecision(6)<<(pgq->apxarea()-pgq->angarea())/sqr(pgq->scale)
    <<endl;
    pgq->subdivide();
    pgq=pgq->sub[i&3];
  }
  geo.resize(1);
  geo[0].settest();
  for (i=0;i<5;i++)
    for (j=0;j<5;j++)
    {
      tassert(geo[0].eslope[5*i+j]==89232+16384*j);
      tassert(geo[0].nslope[5*i+j]==91784-8192*i);
    }
  cube.scale=1/65536.;
  hdr.logScale=-16;
  hdr.planet=BOL_EARTH;
  hdr.dataType=BOL_UNDULATION;
  hdr.encoding=BOL_VARLENGTH;
  hdr.ncomponents=1;
  hdr.tolerance=0.1;
  hdr.sublimit=1000;
  hdr.spacing=1e5;
  hdr.namesFormats.push_back("test");
  hdr.namesFormats.push_back("test");
  for (i=0;i<6;i++)
  {
    interroquad(cube.faces[i],3e5);
    refine(cube.faces[i],cube.scale,hdr.tolerance,hdr.sublimit,hdr.spacing);
  }
  outProgress();
  cout<<endl;
  file.open("test.bol",ios::out|ios::binary);
  hdr.hash=cube.hash();
  hdr.writeBinary(file);
  cube.writeBinary(file);
  cube.clear();
  file.close();
  file.open("test.bol",ios::in|ios::binary);
  hdr.readBinary(file);
  cube.readBinary(file);
  file.close();
  /* Compare the geoquad approximation, which is almost exact since the original
   * is quadratic, with the original geolattice. The geoquads have been written
   * to a file and read back.
   */
  for (i=-12000000;i<=12000000;i+=1000000) // The test pattern extends from -2° to 2°.
    for (j=-12000000;j<=12000000;j+=1000000) // 12000000 is just over 2°.
    {
      u0=geo[0].elev(i,j);
      u1=cube.undulation(i,j);
      if (std::isnan(u0))
	tassert(std::isnan(u1));
      else
	tassert(fabs(u1-u0)<0.001);
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
    tassert(i==j);
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0x40000000<<setw(9)<<j;
    tassert(i+0x40000000==j);
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0x80000000<<setw(9)<<j;
    tassert(i+0x80000000==j || j==(int)0x80000000);
    if (j==(int)0x80000000)
      nancount++;
    j=readgeint(geintf);
    //cout<<setw(9)<<hex<<i+0xc0000000<<setw(9)<<j<<endl;
    tassert(i+0xc0000000==j);
  }
  cout<<dec<<nancount<<" NANs"<<endl;
  tassert(nancount==175);
  cout<<"done."<<endl;
}

void testhlattice()
{
  hlattice latt(31); // 2977 points
  hvec a,b;
  a=nthhvec(1488,31,2977);
  b=latt.nthhvec(1488);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(1457,31,2977);
  b=latt.nthhvec(1457);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(1456,31,2977);
  b=latt.nthhvec(1456);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(1024,31,2977);
  b=latt.nthhvec(1024);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(2048,31,2977);
  b=latt.nthhvec(2048);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(0,31,2977);
  b=latt.nthhvec(0);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
  a=nthhvec(2976,31,2977);
  b=latt.nthhvec(2976);
  cout<<a.getx()<<' '<<a.gety()<<' '<<b.getx()<<' '<<b.gety()<<endl;
  tassert(a==b);
}

bool shoulddo(string testname)
{
  int i;
  bool ret;
  if (testfail)
  {
    cout<<"failed before "<<testname<<endl;
    //sleep(2);
  }
  ret=args.size()==0;
  for (i=0;i<args.size();i++)
    if (testname==args[i])
      ret=true;
  return ret;
}

int main(int argc, char *argv[])
{
  int i;
  for (i=1;i<argc;i++)
    args.push_back(argv[i]);
  doc.pl.resize(2);
  if (shoulddo("area3"))
    testarea3();
  if (shoulddo("relprime"))
    testrelprime();
  if (shoulddo("intersection"))
    testintersection();
  if (shoulddo("in"))
    testin();
  if (shoulddo("triangle"))
    testtriangle();
  if (shoulddo("matrix"))
    testmatrix();
  if (shoulddo("copytopopoints"))
    testcopytopopoints();
  if (shoulddo("invalidintersectionlozenge"))
    testinvalidintersectionlozenge();
  if (shoulddo("invalidintersectionaster"))
    testinvalidintersectionaster();
  if (shoulddo("maketin123"))
    testmaketin123();
  if (shoulddo("maketindouble"))
    testmaketindouble();
  if (shoulddo("maketinaster"))
    testmaketinaster();
  if (shoulddo("maketinbigaster"))
    testmaketinbigaster();
  if (shoulddo("maketinstraightrow"))
    testmaketinstraightrow();
  if (shoulddo("maketinlongandthin"))
    testmaketinlongandthin();
  if (shoulddo("maketinlozenge"))
    testmaketinlozenge();
  if (shoulddo("maketinring"))
    testmaketinring();
  if (shoulddo("maketinellipse"))
    testmaketinellipse();
  if (shoulddo("manysum"))
    testmanysum();
  if (shoulddo("vcurve"))
    testvcurve();
  if (shoulddo("integertrig"))
    testintegertrig();
  if (shoulddo("minquad"))
    testminquad();
  if (shoulddo("segment"))
    testsegment();
  if (shoulddo("arc"))
    testarc();
  if (shoulddo("spiral"))
    testspiral();
  if (shoulddo("spiralarc"))
    testspiralarc();
  if (shoulddo("cogospiral"))
    testcogospiral();
  if (shoulddo("closest"))
    testclosest();
  if (shoulddo("qindex"))
    testqindex();
  if (shoulddo("makegrad"))
    testmakegrad();
  if (shoulddo("derivs"))
    testderivs();
  if (shoulddo("trianglecontours"))
    trianglecontours();
#ifndef NDEBUG
  if (shoulddo("parabinter"))
    testparabinter();
#endif
  if (shoulddo("rasterdraw"))
    testrasterdraw();
  if (shoulddo("dirbound"))
    testdirbound();
  if (shoulddo("stl"))
    teststl();
  if (shoulddo("halton"))
    testhalton();
  if (shoulddo("polyline"))
    testpolyline();
  if (shoulddo("bezier3d"))
    testbezier3d();
  if (shoulddo("angleconv"))
    testangleconv();
  if (shoulddo("grad"))
    testgrad();
  if (shoulddo("csvline"))
    testcsvline();
  if (shoulddo("ldecimal"))
    testldecimal();
  if (shoulddo("ellipsoid"))
    testellipsoid();
  if (shoulddo("projection"))
    testprojection();
  if (shoulddo("color"))
    testcolor();
  if (shoulddo("contour"))
    testcontour();
  if (shoulddo("foldcontour"))
    testfoldcontour();
  if (shoulddo("zigzagcontour"))
    testzigzagcontour();
  if (shoulddo("tracingstop"))
    testtracingstop();
  if (shoulddo("roscat"))
    testroscat();
  if (shoulddo("absorient"))
    testabsorient();
  if (shoulddo("hlattice"))
    testhlattice();
  if (shoulddo("bicubic"))
    testbicubic();
  if (shoulddo("histogram"))
    testhistogram();
  if (shoulddo("smooth5"))
    testsmooth5();
  if (shoulddo("quadhash"))
    testquadhash();
  if (shoulddo("geoid"))
    testgeoid();
  if (shoulddo("geint"))
    testgeint();
  //clampcubic();
  //splitcubic();
  //printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
  //cornustats();
  //closure_i();
  cout<<"\nTest "<<(testfail?"failed":"passed")<<endl;
  return testfail;
}
