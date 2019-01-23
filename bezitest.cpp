/******************************************************/
/*                                                    */
/* bezitest.cpp - test program                        */
/*                                                    */
/******************************************************/
/* Copyright 2012-2019 Pierre Abbat.
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
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <cfloat>
#include <cstring>
#include <unistd.h>
#include <QTime>
#include "config.h"
#include "point.h"
#include "cogo.h"
#include "globals.h"
#include "bezier.h"
#include "rootfind.h"
#include "test.h"
#include "tin.h"
#include "dxf.h"
#include "measure.h"
#include "pnezd.h"
#include "csv.h"
#include "angle.h"
#include "pointlist.h"
#include "vcurve.h"
#include "circle.h"
#include "segment.h"
#include "minquad.h"
#include "arc.h"
#include "spiral.h"
#include "except.h"
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
#include "hnum.h"
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
#include "geoidboundary.h"
#include "refinegeoid.h"
#include "binio.h"
#include "sourcegeoid.h"
#include "bicubic.h"
#include "matrix.h"
#include "quaternion.h"
#include "kml.h"
#include "zoom.h"
#include "manyarc.h"
#include "leastsquares.h"
#include "smooth5.h"

#define psoutput true
// affects only maketin

#define CBRT2 1.2599210498948731647592197537

#define tassert(x) testfail|=(!(x))
//#define tassert(x) if (!(x)) {testfail=true; sleep(10);}
// so that tests still work in non-debug builds

using namespace std;

char hexdig[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
bool slowmanysum=false;
bool testfail=false;
document doc;
vector<string> args;

using namespace std;

void outsizeof(string typeName,int size)
{
  cout<<"size of "<<typeName<<" is "<<size<<endl;
}

void testsizeof()
// This is not run as part of "make test".
{
  outsizeof("bezier3d",sizeof(bezier3d));
  outsizeof("drawobj",sizeof(drawobj));
  outsizeof("Circle",sizeof(Circle));
  outsizeof("segment",sizeof(segment));
  outsizeof("arc",sizeof(arc));
  outsizeof("spiralarc",sizeof(spiralarc));
  outsizeof("xy",sizeof(xy));
  outsizeof("xyz",sizeof(xyz));
  outsizeof("point",sizeof(point));
  outsizeof("edge",sizeof(edge));
  outsizeof("triangle",sizeof(triangle));
  outsizeof("qindex",sizeof(qindex));
  /* A large TIN has 3 edges per point, 2 triangles per point,
   * and 4/9 to 4/3 qindex per point. On x86_64, this amounts to
   * point	160	160	if descriptions are blank
   * edge	72	216
   * triangle	232	464
   * qindex	56	25-75
   * Total		865-915 bytes.
   * On the ARM7l (Raspberry Pi), it is
   * point	120	120
   * edge	48	144
   * triangle	184	368
   * qindex	40	18-53
   * Total		650-685 bytes
   */
  outsizeof("polyline",sizeof(polyline));
  outsizeof("polyarc",sizeof(polyarc));
  outsizeof("polyspiral",sizeof(polyspiral));
  outsizeof("geoquad",sizeof(geoquad));
}

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

void testbbintersection(xy a,xy b,xy c)
{
  int aBear,bBear;
  xy inters;
  aBear=dir(a,c);
  bBear=dir(b,c);
  inters=intersection(a,aBear,b,bBear);
  if ((bBear-aBear)&(DEG180-1))
  {
    tassert(dist(c,inters)<1e-6);
    cout<<"c="<<ldecimal(c.getx())<<','<<ldecimal(c.gety());
    cout<<" inters="<<ldecimal(inters.getx())<<','<<ldecimal(inters.gety())<<endl;
  }
  else
  {
    tassert(inters.isnan());
    cout<<"a, b, and c are collinear\n";
  }
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
  testbbintersection(a,b,c); // These are at 60° angles.
  testbbintersection(b,a,c);
  testbbintersection(b,c,a);
  testbbintersection(c,b,a);
  testbbintersection(c,a,b);
  testbbintersection(a,c,b);
  testbbintersection(a,o,d); // These are collinear.
  testbbintersection(a,d,o);
  testbbintersection(b,o,e);
  testbbintersection(b,e,o);
  testbbintersection(c,o,f);
  testbbintersection(c,f,o);
  testbbintersection(a,o,o); // This one it says is collinear,
  testbbintersection(b,o,o); // but not these two, because
  testbbintersection(c,o,o); // the bearing of o from o is 0.
}

void test1in(xy p,xy a,xy b,xy c,double windnum)
{
  double wind;
  wind=in3(p,a,b,c);
  if (wind!=windnum && (windnum!=IN_AT_CORNER || wind<=-1 || wind==0 || wind>=1))
    cout<<"Triangle ("<<a.east()<<','<<a.north()<<"),("<<
      b.east()<<','<<b.north()<<"),("<<
      c.east()<<','<<c.north()<<"): ("<<
      p.east()<<','<<p.north()<<")'s winding number is "<<wind<<
      ", should be "<<windnum<<endl;
  tassert(wind==windnum || windnum==IN_AT_CORNER);
  wind=in3(p,c,b,a);
  if (windnum<10 && windnum>-10)
    windnum=-windnum;
  if (wind!=windnum && (windnum!=IN_AT_CORNER || wind<=-1 || wind==0 || wind>=1))
    cout<<"Triangle ("<<c.east()<<','<<c.north()<<"),("<<
      b.east()<<','<<b.north()<<"),("<<
      a.east()<<','<<a.north()<<"): ("<<
      p.east()<<','<<p.north()<<")'s winding number is "<<wind<<
      ", should be "<<windnum<<endl;
  tassert(wind==windnum || windnum==IN_AT_CORNER);
}

latlong randomPoint()
/* Pick a point on the sphere according to the spherical asteraceous pattern.
 * This is used for testing usrandom.
 */
{
  int r1;
  latlong ret;
  r1=rng.usrandom();
  ret.lat=asin((2*r1+1)/65536.-1);
  ret.lon=M_1PHI*(2*r1+1)/2.;
  ret.lon-=rint(ret.lon);
  ret.lon*=2*M_PI;
  return ret;
}

void testrandom()
{
  int hist[256],i;
  int done=0,max,min,maxstep=0;
  manysum xsum,ysum,zsum;
  double distsq;
  latlong ll;
  memset(hist,0,sizeof(hist));
  while (!done)
  {
    hist[rng.ucrandom()]++;
    for (max=i=0,min=16777777;i<256;i++)
    {
      if (hist[i]>max)
	max=hist[i];
      if (hist[i]<min)
	min=hist[i];
    }
    if (max>16777215)
      done=-1;
    if (max-min>1.1*pow(max,1/3.) && max-min<0.9*pow(max,2/3.))
      done=1;
    if (max-maxstep>=16384)
    {
      maxstep=max;
      //cout<<max<<' '<<min<<endl;
    }
  }
  tassert(done==1);
  cout<<"Random test: max "<<max<<" min "<<min<<endl;
  for (i=done=0;i<16777216 && !done;i++)
  {
    ll=randomPoint();
    xsum+=cos(ll.lat)*cos(ll.lon);
    ysum+=cos(ll.lat)*sin(ll.lon);
    zsum+=sin(ll.lat);
    distsq=sqr(xsum.total())+sqr(ysum.total())+sqr(zsum.total());
    if (i%65536==65535)
      cout<<i<<' '<<distsq/(i+1)<<endl;
    if (i>1024)
    {
      if (fabs(distsq/(i+1)-1)<0.0005)
	done=1;
      if (distsq<1)
	done=-1;
      if (distsq>sqr(i+1)/3)
	done=-2;
    }
  }
  cout<<i<<' '<<distsq/(i+1)<<endl;
  tassert(done==1);
}

void testin()
{
  xy a(0,0),b(4,0),c(0,3),d(4/3.,1),e(4,3),f(5,0),g(7,-1),h(8,-3),
     i(3,-5),j(0,-6),k(-2,-2),l(-4,0),m(-4,-3),n(-4,6),o(-3,7),p(0,8),q(2,1.5);
  test1in(d,a,b,c,1);
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
  test1in(q,a,b,c,0.5);
  test1in(a,a,b,c,0.25);
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
  int i,j,ran,rr,rc,cc,flipcount,size,giveup;
  mat.setidentity();
  size=mat.getrows();
  for (i=0;i<size;i++)
    for (j=0;j<i;j++)
      mat[i][j]=(rng.ucrandom()*2-255)/BYTERMS;
  for (flipcount=giveup=0;(flipcount<2*size || (flipcount&1)) && giveup<10000;giveup++)
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
  matrix t1t,t2t,t3t,p1t,p2t,p3t;
  matrix hil(8,8),lih(8,8),hilprod;
  matrix kd(7,7);
  matrix r0,c0,p11;
  matrix rs1(3,4),rs2,rs3,rs4;
  vector<double> rv,cv;
  double tr1,tr2,tr3,de1,de2,de3,tr1t,tr2t,tr3t;
  double toler=1.2e-12;
  double kde;
  double lo,hi;
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
  lo=INFINITY;
  hi=0;
  for (i=0;i<1;i++)
  {
    t1.randomize_c();
    t2.randomize_c();
    t3.randomize_c();
    t1t=t1.transpose();
    t2t=t2.transpose();
    t3t=t3.transpose();
    p1=t1*t2*t3;
    p2=t2*t3*t1;
    p3=t3*t1*t2;
    p1t=t3t*t2t*t1t;
    p2t=t1t*t3t*t2t;
    p3t=t2t*t1t*t3t;
    tr1=p1.trace();
    tr2=p2.trace();
    tr3=p3.trace();
    tr1t=p1t.trace();
    tr2t=p2t.trace();
    tr3t=p3t.trace();
    de1=p1.determinant();
    de2=p2.determinant();
    de3=p3.determinant();
    cout<<"trace1 "<<ldecimal(tr1)
	<<" trace2 "<<ldecimal(tr2)
	<<" trace3 "<<ldecimal(tr3)<<endl;
    tassert(fabs(tr1-tr2)<toler && fabs(tr2-tr3)<toler && fabs(tr3-tr1)<toler);
    tassert(fabs(tr1-tr1t)<toler && fabs(tr2-tr2t)<toler && fabs(tr3-tr3t)<toler);
    tassert(tr1!=0);
    cout<<"det1 "<<de1
	<<" det2 "<<de2
	<<" det3 "<<de3<<endl;
    tassert(fabs(de1)>1e80 && fabs(de2)<1e60 && fabs(de3)<1e52);
    // de2 and de3 would actually be 0 with exact arithmetic.
    if (fabs(de2)>hi)
      hi=fabs(de2);
    if (fabs(de2)<lo)
      lo=fabs(de2);
  }
  cout<<"Lowest det2 "<<lo<<" Highest det2 "<<hi<<endl;
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
  for (i=0;i<1;i++)
  {
    knowndet(kd);
    //loadknowndet(kd,"z0z0z0z1c9dd28z0z1z03c46c35cz0z0z0z0z1z0z0aa74z169f635e3z0z0z0z003z0z1z0z0z0z0fcz146z160z000f50091");
    //loadknowndet(kd,"a6z1fc7ce056d6d1z0z0z0z0z1b49ez0z0z1z02f53z1z0z0z0z0z0z0e2z0z097z1230488z0z19b25484e1ez0z0z0z0z0z1");
    kd.dump();
    dumpknowndet(kd);
    kde=kd.determinant();
    cout<<"Determinant of shuffled matrix is "<<ldecimal(kde)<<" diff "<<kde-1<<endl;
    tassert(fabs(kde-1)<4e-12);
  }
  for (i=0;i<11;i++)
  {
    rv.push_back((i*i*i)%11);
    cv.push_back((i*3+7)%11);
  }
  r0=rowvector(rv);
  c0=columnvector(cv);
  p1=r0*c0;
  p11=c0*r0;
  tassert(p1.trace()==253);
  tassert(p11.trace()==253);
  tassert(p1.determinant()==253);
  tassert(p11.determinant()==0);
  for (i=0;i<3;i++)
    for (j=0;j<4;j++)
      rs1[i][j]=(j+1.)/(i+1)-(i^j);
  rs2=rs1;
  rs2.resize(4,3);
  rs3=rs1*rs2;
  rs4=rs2*rs1;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      tassert(rs1[i][j]==rs2[i][j]);
  cout<<"det rs3="<<ldecimal(rs3.determinant())<<endl;
  tassert(fabs(rs3.determinant()*9-100)<1e-12);
  tassert(rs4.determinant()==0);
  rs4[3][3]=1;
  tassert(fabs(rs4.determinant()*9-100)<1e-12);
}

void testquaternion()
{
  Quaternion q0(0,0,0,0),q1(1,0,0,0),qr2(0,1,0,0),qr3(0.5,0.5,0.5,0.5);
  Quaternion qr5(M_1PHI/2+0.5,0.5,M_1PHI/2,0),qsum(1.5,0.5,0.5,0.5);
  Quaternion qprod(-0.5,0,-M_1PHI/2,M_1PHI/2+0.5);
  Quaternion qrste(-1/14.,5/14.,7/14.,11/14.);
  xyz vec0(0,0.6,0.8),vec2,vec3,vec5;
  xyz vec2r(0,-0.6,-0.8),vec3r(0.8,0,0.6),vec5r(0.3*M_1PHI+0.4,0.3-0.4/M_1PHI,0.3/M_1PHI+0.4*M_1PHI);
  xyz vecste(0,0,-192);
  vball v;
  /* qr2, qr3, and qr5 generate the group of order 120 which is the group
   * of rotations of an icosahedron (or dodecahedron) times 2.
   * The matrix of qr5 is
   * 1-1/2φ² 1/2φ     1/2              0.809017  0.309017  0.500000
   * 1/2φ    1/2      -1/2φ-1/2        0.309017  0.500000 -0.809017
   * -1/2    1/2φ+1/2 1/2-1/2φ²       -0.500000  0.809017  0.309017
   */
  Quaternion res;
  int i,j;
  res=q1+qr3;
  tassert(res==qsum);
  tassert(res!=q0);
  res=qr2*qr3*qr5; // 1/3 rotation about (0,1/φ,-φ)
  for (j=0;j<4;j++)
    cout<<setprecision(6)<<res.getcomp(j)<<' ';
  cout<<endl;
  tassert((qprod-res).norm()<1e-12);
  res=q1;
  for (i=0;i<10;i++)
  {
    res=res*qr5;
    for (j=0;j<4;j++)
      cout<<setprecision(6)<<res.getcomp(j)<<' ';
    cout<<endl;
  }
  vec2=qr2.rotate(vec0);
  vec3=qr3.rotate(vec0);
  vec5=qr5.rotate(vec0);
  cout<<vec2.getx()<<' '<<vec2.gety()<<' '<<vec2.getz()<<endl;
  cout<<vec3.getx()<<' '<<vec3.gety()<<' '<<vec3.getz()<<endl;
  cout<<vec5.getx()<<' '<<vec5.gety()<<' '<<vec5.getz()<<endl;
  tassert((vec2-vec2r).length()<1e-15);
  tassert((vec3-vec3r).length()<1e-15);
  tassert((vec5-vec5r).length()<1e-15);
  vec2=qrste.rotate(vecste);
  cout<<vec2.getx()<<' '<<vec2.gety()<<' '<<vec2.getz()<<endl;
  v=encodedir(vec2);
  cout<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  cout<<ldecimal(radtodeg(vec2.lat()))<<' '<<ldecimal(radtodeg(vec2.lon()))<<endl;
}

void testcopytopopoints()
{
  //criteria crit;
  criterion crit1;
  doc.makepointlist(1);
  doc.pl[0].clear();
  doc.copytopopoints(1,0);
  tassert(doc.pl[1].points.size()==0);
  doc.pl[0].addpoint(1,point(0,0,0,"eip"));
  doc.pl[0].addpoint(1,point(25,0,0,"eip"));
  doc.pl[0].addpoint(1,point(25,40,0,"eip"));
  doc.pl[0].addpoint(1,point(0,40,0,"eip"));
  doc.pl[0].addpoint(1,point(5,10,1,"house"));
  doc.pl[0].addpoint(1,point(20,10,1,"house"));
  doc.pl[0].addpoint(1,point(20,20,1,"house"));
  doc.pl[0].addpoint(1,point(5,20,1,"house"));
  doc.pl[0].addpoint(1,point(3,-5,-1.5,"pipe"));
  doc.pl[0].addpoint(1,point(3,41,-2,"pipe"));
  doc.pl[0].addpoint(1,point(2,22,0.3,"topo"));
  doc.pl[0].addpoint(1,point(23,22,0.4,"topo"));
  doc.pl[0].addpoint(1,point(12.5,38,0.5,"topo"));
  crit1.str="topo";
  crit1.istopo=true;
  doc.pl[1].crit.push_back(crit1);
  doc.copytopopoints(1,0);
  tassert(doc.pl[1].points.size()==3);
  doc.pl[1].crit[0].str="";
  doc.pl[1].crit[0].lo=5;
  doc.pl[1].crit[0].hi=8;
  doc.copytopopoints(1,0);
  tassert(doc.pl[1].points.size()==4);
  doc.pl[1].crit[0].lo=0;
  doc.pl[1].crit[0].hi=0;
  doc.pl[1].crit[0].elo=0;
  doc.pl[1].crit[0].ehi=5;
  doc.copytopopoints(1,0);
  tassert(doc.pl[1].points.size()==11);
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
  doc.makepointlist(1);
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
  doc.makepointlist(1);
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
  doc.makepointlist(1);
  doc.pl[1].clear();
  aster(doc,1);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  tassert(i==notri);
  doc.pl[1].clear();
  aster(doc,2);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  tassert(i==notri);
  doc.pl[1].clear();
  aster(doc,3);
  i=0;
  try
  {
    doc.pl[1].maketin();
  }
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  tassert(i==0);
}

void testmaketindouble()
{
  int i;
  doc.makepointlist(1);
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
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  cout<<"maketin threw "<<i<<endl;
}

void testmaketinaster()
{
  double totallength;
  int i,edgerand;
  edge *line;
  doc.makepointlist(1);
  doc.pl[1].clear();
  aster(doc,100);
  doc.pl[1].maketin(psoutput?"aster.ps":"",false);
  tassert(doc.pl[1].edges.size()==284);
  totallength=doc.pl[1].totalEdgeLength();
  tassert(fabs(totallength-600.689)<0.001);
  doc.pl[1].maketriangles();
  tassert(doc.pl[1].checkTinConsistency());
  edgerand=rng.uirandom();
  for (i=0;i<32;i++)
    if ((edgerand&(1<<i)) && doc.pl[1].edges[i].isFlippable())
    {
      doc.pl[1].edges[i].flip(&doc.pl[1]);
      doc.pl[1].edges[i].flip(&doc.pl[1]);
    }
  tassert(doc.pl[1].checkTinConsistency());
  line=doc.pl[1].points[1].line;
  for (i=0;i<5;i++)
  {
    cout<<"Edge bearing "<<bintodeg(line->bearing(&doc.pl[1].points[1]))<<'\n';
    line=line->next(&doc.pl[1].points[1]);
  }
}

void testmaketinbigaster()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  aster(doc,5972);
  doc.pl[1].maketin(psoutput?"bigaster.ps":"",true);
  totallength=doc.pl[1].totalEdgeLength();
  tassert(fabs((totallength/doc.pl[1].points.size()-5.9)*sqrt(doc.pl[1].points.size()))<10);
  cout<<"Total edge length "<<totallength<<endl;
  // 2282 13772.1
  // 5972 35683.1
  // 7011 42028.2
  // 8231 49318.1
  // 9663 57767.4
  // 11344 67181
  // 13318 78859.7
  // 100000 590335
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
  doc.makepointlist(1);
  doc.pl[1].clear();
  straightrow(doc,100);
  rotate(doc,30);
  try
  {
    doc.pl[1].maketin();
  }
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  tassert(i==flattri);
  doc.pl[1].clear();
  for (i=0;i<7;i++)
    doc.pl[1].addpoint(i+1,point(i,2*i,sin((double)i),"test"),false);
  try
  {
    doc.pl[1].maketin();
  }
  catch(BeziExcept e)
  {
    i=e.getNumber();
  }
  tassert(i==flattri);
}

void testmaketinlongandthin()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  longandthin(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"longandthin.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  totallength=doc.pl[1].totalEdgeLength();
  printf("longandthin %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  tassert(fabs(totallength-123.499)<0.001);
}

void testmaketinlozenge()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  lozenge(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"lozenge.ps":"");
  tassert(doc.pl[1].edges.size()==299);
  totallength=doc.pl[1].totalEdgeLength();
  printf("lozenge %ld edges total length %f\n",doc.pl[1].edges.size(),totallength);
  tassert(fabs(totallength-2111.8775)<0.001);
}

void testmaketinring()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  ring(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"ring.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  totallength=doc.pl[1].totalEdgeLength();
  printf("ring edges total length %f\n",totallength);
  //Don't tassert the total length. There are over 10^56 (2^189) right answers to that.
}

void testmaketinwheel()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  wheelwindow(doc,100);
  rotate(doc,30);
  doc.pl[1].maketin(psoutput?"wheel.ps":"");
  tassert(doc.pl[1].edges.size()==500);
  totallength=doc.pl[1].totalEdgeLength();
  printf("wheel edges total length %f\n",totallength);
  tassert(fabs(totallength-1217.2716)<0.001);
}

void testmaketinellipse()
{
  double totallength;
  int i;
  doc.makepointlist(1);
  doc.pl[1].clear();
  ellipse(doc,100);
  doc.pl[1].maketin(psoutput?"ellipse.ps":"");
  tassert(doc.pl[1].edges.size()==197);
  totallength=doc.pl[1].totalEdgeLength();
  printf("ellipse edges total length %f\n",totallength);
  tassert(fabs(totallength-1329.4675)<0.001);
}

void test1tripolygon(int points,int petals,PostScript &ps)
{
  int i;
  vector<point *> poly,hull;
  manysum area;
  polyline pl;
  doc.makepointlist(1);
  doc.pl[1].clear();
  ps.startpage();
  ps.setscale(-2,-2,2,2,0);
  ps.setcolor(0,0,1);
  ps.startline();
  for (i=0;i<points;i++)
  {
    doc.pl[1].addpoint(i+1,point(cossin(2*M_PI*i/points)*(1+cos(2*M_PI*i*petals/points)),0,""));
    poly.push_back(&doc.pl[1].points[i+1]);
    pl.insert(doc.pl[1].points[i+1]);
    ps.lineto(*poly.back());
  }
  ps.endline(true);
  doc.pl[1].triangulatePolygon(poly);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].triangles.size();i++)
  {
    ps.line2p(*doc.pl[1].triangles[i].a,*doc.pl[1].triangles[i].b);
    ps.line2p(*doc.pl[1].triangles[i].b,*doc.pl[1].triangles[i].c);
    ps.line2p(*doc.pl[1].triangles[i].c,*doc.pl[1].triangles[i].a);
    area+=doc.pl[1].triangles[i].sarea;
  }
  hull=doc.pl[1].fromInt1loop(doc.pl[1].convexHull());
  ps.setcolor(1,0,1);
  ps.startline();
  for (i=0;i<hull.size();i++)
    ps.lineto(*hull[i]);
  ps.endline(true);
  ps.endpage();
  cout<<points<<" points, "<<petals<<" petals, "<<doc.pl[1].triangles.size()<<" triangles, ";
  cout<<hull.size()<<" points in convex hull\n";
  cout<<"Area "<<area.total()<<", should be "<<pl.area()<<endl;
  tassert(points==doc.pl[1].triangles.size()+2);
  tassert(fabs(area.total()-pl.area())<1e-12);
  doc.pl[1].makeEdges();
  tassert(doc.pl[1].checkTinConsistency());
}

#define EHF 89
/* 89, normally, or 1597, to test filling in big loops.
 * It should be a Fibonacci number not divisible by 2 or 3.
 */
void testehcycloid(PostScript &ps)
/* Makes a TIN out of 178 triangles between an epicycloid and a hypocycloid
 * (deltoid) of three cusps.
 */
{
  int i,j;
  array<xyz,EHF> outer,inner;
  vector<array<xyz,3> > faces;
  array<xyz,3> face;
  intloop holes;
  int1loop loop1;
  for (i=0;i<EHF;i++)
  {
    outer[i]=xyz(4*cossin(2*M_PI*i/EHF)-cossin(8*M_PI*i/EHF),sin(10*M_PI*i/EHF)/10);
    inner[i]=xyz(-2*cossin(2*M_PI*i/EHF)-cossin(-4*M_PI*i/EHF),sin(10*M_PI*i/EHF)/10);
  }
  for (i=0;i<EHF;i++)
  {
    face[0]=outer[i];
    face[1]=inner[(i+(EHF+1)/2)%EHF];
    face[2]=inner[(i+EHF/2)%EHF];
    faces.push_back(face);
    face[0]=inner[i];
    face[1]=outer[(i+EHF/2)%EHF];
    face[2]=outer[(i+(EHF+1)/2)%EHF];
    faces.push_back(face);
    //cout<<setw(9)<<hex<<i<<' '<<setw(9)<<inv2adic(i)<<dec<<'\n';
  }
  doc.makepointlist(1);
  doc.pl[1].makeBareTriangles(faces);
  cout<<doc.pl[1].points.size()<<" points "<<doc.pl[1].qinx.size()<<" qindex nodes\n";
  tassert(doc.pl[1].points.size()==2*EHF);
  if (EHF==89)
    tassert(doc.pl[1].qinx.size()==289);
  doc.pl[1].makeEdges();
  holes=doc.pl[1].boundary();
  holes.push_back(doc.pl[1].convexHull());
  holes.consolidate();
  for (i=0;i<holes.size();i++)
    doc.pl[1].triangulatePolygon(doc.pl[1].fromInt1loop(holes[i]));
  doc.pl[1].makeEdges();
  ps.startpage();
  ps.setscale(-5,-5,5,5,0);
  ps.setcolor(0,0,1);
  ps.setPointlist(doc.pl[1]);
  for (i=0;i<doc.pl[1].edges.size();i++)
    ps.line(doc.pl[1].edges[i],i,false);
  for (i=0;i<holes.size();i++)
  {
    loop1=holes[i];
    /*ps.startline();
    for (j=0;j<loop1.size();j++)
      ps.lineto(doc.pl[1].points[loop1[j]]);
    ps.endline(true);*/
  }
  tassert(doc.pl[1].checkTinConsistency());
  cout<<doc.pl[1].edges.size()<<" edges "<<doc.pl[1].convexHull().size()<<" points in convex hull\n";
  /* 493 edges 38 points in convex hull; 493-4*89-38=99
   * Four regions to fill in: 89, 19, 19, 19
   * Edges needed to fill them in: 86+17+17+17
   */
  ps.endpage();
}

void testtripolygon()
{
  PostScript ps;
  ps.open("tripolygon.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  test1tripolygon(89,1,ps);
  test1tripolygon(89,2,ps);
  test1tripolygon(89,3,ps);
  test1tripolygon(89,5,ps);
  test1tripolygon(89,8,ps);
  test1tripolygon(89,13,ps);
  test1tripolygon(89,21,ps);
  test1tripolygon(89,34,ps);
  testehcycloid(ps);
}

void test1break0graph(pointlist &pl,string plname)
{
  int i,j,k,niter,off[4],sz;
  cout<<plname<<endl;
  sz=pl.type0Breaklines.size();
  niter=ceil(sz*log(sz)/log(256));
  //cout<<niter*4<<" possible flips\n";
  off[3]=relprime(sz);
  off[2]=relprime(off[3]);
  off[1]=relprime(off[2]);
  off[0]=0;
  for (i=0;i<niter;i++)
  {
    k=rng.ucrandom();
    for (j=0;j<8;j+=2)
    {
      if ((k>>j)&1)
        pl.type0Breaklines[(i+j/2)%sz].reverse();
      swap(pl.type0Breaklines[(i+j/2)%sz],pl.type0Breaklines[(i+j/2+off[(k>>j)&3])%sz]);
    }
  }
  for (i=0;i<sz;i++)
  {
    if (i)
      cout<<',';
    pl.type0Breaklines[i].writeText(cout);
  }
  cout<<endl;
  pl.joinBreaklines();
  cout<<"-----\n";
  for (i=0;i<pl.type0Breaklines.size();i++)
  {
    pl.type0Breaklines[i].writeText(cout);
    cout<<endl;
  }
}

void testtindxf()
{
  int i,acc,fmt;
  GroupCode a,b(0),c(70),d(11),e(290);
  vector<GroupCode> dxfTxt,dxfBin;
  vector<array<xyz,3> > binFaces,txtFaces;
  xyz pnt;
  for (acc=i=0;i<=1001;i+=13)
  {
    /* The tags tested include 169, which is the end of a block of 8s,
     * and 390, which is the first of a block of 132s.
     */
    fmt=tagFormat(i);
    //cout<<i<<' '<<fmt<<endl;
    acc=(acc*17+fmt)%8191;
  }
  cout<<"Hash of every 13th DXF tag format is "<<acc<<endl;
  tassert(acc==3424);
  b.str="SECTION";
  c.integer=105;
  e.flag=true;
  a=b;
  b=c;
  c=d;
  d=e;
  e=GroupCode();
  tassert(a.str=="SECTION");
  tassert(b.integer==105);
  dxfTxt=readDxfGroups("tinytin-txt.dxf");
  if (dxfTxt.size()==0)
    dxfTxt=readDxfGroups("../tinytin-txt.dxf");
  dxfBin=readDxfGroups("tinytin-bin.dxf");
  if (dxfBin.size()==0)
    dxfBin=readDxfGroups("../tinytin-bin.dxf");
  cout<<"Read "<<dxfTxt.size()<<" groups from tinytin-txt, "<<dxfBin.size()<<" from tinytin-bin\n";
  tassert(dxfTxt.size()==dxfBin.size());
  for (i=0;i<dxfBin.size();i++)
  {
    if (dxfBin[i].tag==0)
      cout<<dxfBin[i].str<<'\n';
    if (dxfBin[i].tag>=10 && dxfBin[i].tag<=39)
    {
      cout<<ldecimal(dxfBin[i].real);
      if (dxfBin[i].tag<30)
	cout<<' ';
      else
	cout<<'\n';
    }
  }
  binFaces=extractTriangles(dxfBin);
  txtFaces=extractTriangles(dxfTxt);
  cout<<binFaces.size()<<" triangles in binary file, "<<txtFaces.size()<<" in text file\n";
  doc.makepointlist(1);
  doc.pl[1].makeBareTriangles(binFaces);
  cout<<doc.pl[1].points.size()<<" points "<<doc.pl[1].qinx.size()<<" qindex nodes\n";
  tassert(doc.pl[1].points.size()==6);
  tassert(doc.pl[1].qinx.size()==5);
  for (i=1;i<=doc.pl[1].points.size();i++)
  {
    pnt=doc.pl[1].points[i];
    cout<<pnt.east()<<','<<pnt.north()<<','<<pnt.elev()<<'\n';
  }
}

void testbreak0()
{
  double leftedge,bottomedge,rightedge,topedge,conterval,totallength;
  pointlist bull,octahedron,frucht,cycle17,chain17;
  int rotation=DEG90,i,j,sum,nopen;
  criterion crit1;
  PostScript ps;
  Breakline0 bl1,bl2,bl3;
  array<int,2> edg;
  doc.makepointlist(1);
  doc.pl[0].clear();
  doc.copytopopoints(1,0);
  tassert(doc.pl[1].points.size()==0);
  crit1.str="";
  crit1.istopo=true;
  doc.pl[1].crit.push_back(crit1);
  // Data are from some project of Travis Pruitt.
  doc.pl[0].addpoint( 17,point(42.093,87.960,271.810,"SW20"));
  doc.pl[0].addpoint( 18,point(46.493,87.947,271.528,"SW20"));
  doc.pl[0].addpoint( 19,point(45.760,86.557,271.511,"SW20XB TCR"));
  doc.pl[0].addpoint( 20,point(42.088,86.580,271.739,"TCR 30 CONCRETE"));
  doc.pl[0].addpoint( 22,point(45.789,86.320,271.322,"P11"));
  doc.pl[0].addpoint( 23,point(45.794,86.365,271.373,"TP11"));
  doc.pl[0].addpoint( 24,point(48.355,86.366,271.192,"TP11"));
  doc.pl[0].addpoint( 25,point(48.355,86.320,271.166,"P11"));
  doc.pl[0].addpoint( 26,point(50.847,86.328,271.006,"P11"));
  doc.pl[0].addpoint( 27,point(50.726,86.386,271.071,"TP11"));
  doc.pl[0].addpoint( 28,point(50.858,86.398,271.116,"TP11"));
  doc.pl[0].addpoint( 29,point(50.902,86.529,271.192,"TCL69"));
  doc.pl[0].addpoint( 30,point(50.212,87.894,271.298,"SW69"));
  doc.pl[0].addpoint( 31,point(56.770,87.911,270.893,"SW69"));
  doc.pl[0].addpoint( 32,point(56.758,86.515,270.846,"TCL69"));
  doc.pl[0].addpoint( 39,point(63.792,86.516,270.411,"TCL69"));
  doc.pl[0].addpoint( 40,point(63.873,87.864,270.451,"SW69"));
  doc.pl[0].addpoint( 41,point(75.374,87.828,269.770,"SW69"));
  doc.pl[0].addpoint( 42,point(75.333,86.449,269.689,"TCL69"));
  doc.pl[0].addpoint( 43,point(82.548,86.418,269.252,"TCL69"));
  doc.pl[0].addpoint( 44,point(82.749,87.772,269.303,"SW69"));
  doc.pl[0].addpoint( 47,point(87.423,86.382,268.947,"TCL69 30 CONCRETE"));
  doc.pl[0].addpoint( 48,point(83.860,86.691,269.190,"WM"));
  doc.pl[0].addpoint( 49,point(83.886,86.954,269.194,""));
  doc.pl[0].addpoint( 50,point(50.832,85.760,271.075,"EP1"));
  doc.pl[0].addpoint( 51,point(45.758,85.818,271.379,"EP1"));
  doc.pl[0].addpoint( 52,point(49.792,89.456,271.394,"EP70"));
  doc.pl[0].addpoint( 53,point(49.876,92.374,272.001,"EP70"));
  doc.pl[0].addpoint( 54,point(50.885,92.320,272.206,"G"));
  doc.pl[0].addpoint( 55,point(48.322,92.259,272.001,"P"));
  doc.pl[0].addpoint( 56,point(48.339,89.467,271.459,"P"));
  doc.pl[0].addpoint( 57,point(48.313,86.899,271.237,"P"));
  doc.pl[0].addpoint( 58,point(53.060,92.227,272.098,"G"));
  doc.pl[0].addpoint( 59,point(56.850,92.421,271.912,"G"));
  doc.pl[0].addpoint( 60,point(56.782,89.427,270.920,"SW70"));
  doc.pl[0].addpoint( 61,point(63.139,89.389,270.525,"SW70"));
  doc.pl[0].addpoint( 62,point(62.410,91.055,271.211,"G"));
  doc.pl[0].addpoint( 63,point(71.086,89.356,270.040,"SW70"));
  doc.pl[0].addpoint( 64,point(70.181,90.998,270.628,"G"));
  doc.pl[0].addpoint( 65,point(75.105,91.009,270.377,"G"));
  doc.pl[0].addpoint( 66,point(79.181,91.104,270.246,"G"));
  doc.pl[0].addpoint( 67,point(80.366,91.013,270.197,"BW30"));
  doc.pl[0].addpoint( 68,point(80.321,89.332,269.487,"BW30 R 0.65 BRICK"));
  doc.pl[0].addpoint( 69,point(80.345,89.321,269.485,"STP31"));
  doc.pl[0].addpoint( 70,point(81.566,90.579,270.005,"BW31"));
  doc.pl[0].addpoint( 71,point(81.533,89.330,269.420,"STP31"));
  doc.pl[0].addpoint( 72,point(80.068,89.313,269.505,"SW70"));
  doc.pl[0].addpoint( 73,point(80.088,91.023,270.134,"TOE70"));
  doc.pl[0].addpoint( 74,point(81.788,89.289,269.404,"SW50"));
  doc.pl[0].addpoint( 75,point(81.833,91.001,270.109,"TOE50"));
  doc.pl[0].addpoint( 76,point(84.429,90.994,270.056,"G"));
  doc.pl[0].addpoint( 77,point(86.767,91.123,269.866,"G"));
  doc.pl[0].addpoint(118,point(83.281,81.680,269.239,"P1"));
  doc.pl[0].addpoint(119,point(72.835,81.736,269.863,"P1"));
  doc.pl[0].addpoint(120,point(61.643,81.829,270.535,"P1"));
  doc.pl[0].addpoint(121,point(51.118,81.881,271.152,"P1"));
  doc.pl[0].addpoint(122,point(42.016,81.912,271.682,"P1"));
  doc.pl[0].addpoint(123,point(41.926,81.914,271.687,"PS1 DOUBLE YELLOW LINE"));
  doc.pl[0].addpoint(124,point(41.993,77.924,271.515,"EP20"));
  doc.pl[0].addpoint(125,point(46.228,77.861,271.252,"EP20"));
  doc.pl[0].addpoint(126,point(42.063,77.312,271.461,"TP20"));
  doc.pl[0].addpoint(127,point(42.051,77.357,271.424,"P20"));
  doc.pl[0].addpoint(128,point(44.804,77.328,271.270,"P20"));
  doc.pl[0].addpoint(129,point(44.801,77.296,271.306,"TP20"));
  doc.pl[0].addpoint(130,point(45.344,77.285,271.298,"TP20"));
  doc.pl[0].addpoint(131,point(46.243,77.249,271.379,"TP20"));
  doc.pl[0].addpoint(132,point(46.220,77.344,271.190,"P20"));
  doc.pl[0].addpoint(133,point(46.259,77.150,271.379,"TCR30"));
  doc.pl[0].addpoint(134,point(45.735,75.773,271.491,"SW30"));
  doc.pl[0].addpoint(135,point(54.018,75.750,270.992,"SW30"));
  doc.pl[0].addpoint(136,point(54.215,77.107,270.910,"TCR30"));
  doc.pl[0].addpoint(137,point(61.062,77.072,270.496,"TCR30"));
  doc.pl[0].addpoint(140,point(60.831,75.715,270.588,"SW30"));
  doc.pl[0].addpoint(141,point(71.361,75.623,269.858,"SW30"));
  doc.pl[0].addpoint(142,point(71.317,76.987,269.824,"TCR30"));
  doc.pl[0].addpoint(143,point(80.194,76.961,269.353,"TCR30"));
  doc.pl[0].addpoint(144,point(80.308,75.588,269.340,"SW30"));
  doc.pl[0].addpoint(145,point(85.448,75.581,269.031,"SW30"));
  doc.pl[0].addpoint(146,point(84.935,76.947,269.049,"SW30"));
  doc.pl[0].addpoint(147,point(84.977,77.682,268.966,"EP30"));
  doc.pl[0].addpoint(154,point(84.956,77.040,269.031,"TP20"));
  doc.pl[0].addpoint(155,point(85.258,77.063,268.945,"TP20"));
  doc.pl[0].addpoint(156,point(85.193,77.120,268.931,"P20"));
  doc.pl[0].addpoint(157,point(84.966,77.140,268.919,"P20"));
  doc.pl[0].addpoint(194,point(85.639,72.427,268.966,"EP23"));
  doc.pl[0].addpoint(195,point(85.842,74.031,269.005,"EP23"));
  doc.pl[0].addpoint(196,point(75.043,74.113,269.699,"SW23"));
  doc.pl[0].addpoint(197,point(64.980,74.183,270.359,"SW23"));
  doc.pl[0].addpoint(198,point(56.561,74.250,270.921,"SW23"));
  doc.pl[0].addpoint(199,point(48.573,74.250,271.326,"SW23"));
  doc.pl[0].addpoint(200,point(45.010,74.292,271.625,"BW20"));
  doc.pl[0].addpoint(201,point(44.951,72.682,272.021,"BW20 L 0.5 CONCRETE"));
  doc.pl[0].addpoint(202,point(45.208,72.673,272.140,"TOE"));
  doc.pl[0].addpoint(203,point(45.219,74.028,271.737,"TOE"));
  doc.pl[0].addpoint(204,point(47.375,74.041,271.580,"TOE"));
  doc.pl[0].addpoint(205,point(48.557,74.072,271.410,"TOE"));
  doc.pl[0].addpoint(206,point(42.289,72.461,272.167,"P"));
  doc.pl[0].addpoint(207,point(46.420,73.085,272.229,"TCM 4"));
  doc.pl[0].addpoint(208,point(47.098,72.614,272.234,"G"));
  doc.pl[0].addpoint(209,point(50.676,72.600,271.518,"G"));
  doc.pl[0].addpoint(210,point(59.436,72.935,270.946,"G"));
  doc.pl[0].addpoint(211,point(68.815,73.038,270.250,"G"));
  doc.pl[0].addpoint(220,point(75.304,72.656,269.885,"G"));
  doc.pl[0].addpoint(221,point(83.154,72.543,269.327,"G"));
  bl1<<29<<32<<39<<42<<43<<47;
  bl2<<133<<136<<137<<142<<143<<146;
  bl3<<19<<20;
  leftedge=doc.pl[0].dirbound(-rotation);
  bottomedge=doc.pl[0].dirbound(-rotation+DEG90);
  rightedge=-doc.pl[0].dirbound(-rotation+DEG180);
  topedge=-doc.pl[0].dirbound(-rotation-DEG90);
  doc.copytopopoints(1,0);
  ps.open("break0.ps");
  ps.setDoc(doc);
  ps.prolog();
  ps.startpage();
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  checkedgediscrepancies(doc.pl[1]);
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  ps.setscale(leftedge,bottomedge,rightedge,topedge,rotation);
  conterval=0.2;
  roughcontours(doc.pl[1],conterval);
  smoothcontours(doc.pl[1],conterval,true,false);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(leftedge,bottomedge,rightedge,topedge,rotation);
  for (i=0;i<doc.pl[1].edges.size();i++)
  {
    ps.line(doc.pl[1].edges[i],i,false);
  }
  totallength=doc.pl[1].totalEdgeLength();
  cout<<"Total length without breaklines "<<totallength<<endl;
  tassert(fabs(totallength-1221.8)<0.1);
  ps.endpage();
  ps.startpage();
  cout<<'\n';
  doc.pl[1].type0Breaklines.push_back(bl1);
  doc.pl[1].type0Breaklines.push_back(bl2);
  doc.pl[1].type0Breaklines.push_back(bl3);
  for (i=0;i<doc.pl[1].type0Breaklines.size();i++)
  {
    doc.pl[1].type0Breaklines[i].writeText(cout);
    cout<<'\n';
  }
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  checkedgediscrepancies(doc.pl[1]);
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  ps.setscale(leftedge,bottomedge,rightedge,topedge,rotation);
  conterval=0.2;
  roughcontours(doc.pl[1],conterval);
  smoothcontours(doc.pl[1],conterval,true,false);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(leftedge,bottomedge,rightedge,topedge,rotation);
  for (i=0;i<doc.pl[1].edges.size();i++)
  {
    ps.line(doc.pl[1].edges[i],i,false);
  }
  totallength=doc.pl[1].totalEdgeLength();
  cout<<"Total length with breaklines "<<totallength<<endl;
  tassert(fabs(totallength-1231.9)<0.1);
  ps.endpage();
  ps.trailer();
  ps.close();
  cout<<'\n';
  /* Now test joining single edges into breaklines.
   * The number of open trails is an invariant of the graph,
   * equal to half the number of odd-degree vertices.
   */
  bull.type0Breaklines.push_back(Breakline0(1,2));
  bull.type0Breaklines.push_back(Breakline0(2,3));
  bull.type0Breaklines.push_back(Breakline0(3,4));
  bull.type0Breaklines.push_back(Breakline0(4,2));
  bull.type0Breaklines.push_back(Breakline0(4,5));
  test1break0graph(bull,"bull");
  for (i=sum=nopen=0;i<bull.type0Breaklines.size();i++)
  {
    sum+=bull.type0Breaklines[i].size();
    nopen+=bull.type0Breaklines[i].isOpen();
  }
  tassert(sum==5);
  tassert(nopen==2);
  tassert(i<4 && i>1);
  for (i=1;i<7;i++)
    for (j=i+1;j<7;j++)
      if (i+j!=7)
        octahedron.type0Breaklines.push_back(Breakline0(i,j));
  test1break0graph(octahedron,"octahedron");
  for (i=sum=nopen=0;i<octahedron.type0Breaklines.size();i++)
  {
    /* Can't assert that the cycle sizes are divisible by 3 or 4. It is
     * possible to get a 5-cycle and a 7-cycle. One can also get four
     * 3-cycles or three 4-cycles.
     */
    tassert(octahedron.type0Breaklines[i].size()>2);
    sum+=octahedron.type0Breaklines[i].size();
    nopen+=octahedron.type0Breaklines[i].isOpen();
  }
  tassert(sum==12);
  tassert(i<5);
  tassert(nopen==0);
  for (i=1;i<12;i++)
    frucht.type0Breaklines.push_back(Breakline0(i,i+1));
  frucht.type0Breaklines.push_back(Breakline0(12,1));
  frucht.type0Breaklines.push_back(Breakline0(1,3));
  frucht.type0Breaklines.push_back(Breakline0(4,6));
  frucht.type0Breaklines.push_back(Breakline0(7,9));
  frucht.type0Breaklines.push_back(Breakline0(2,10));
  frucht.type0Breaklines.push_back(Breakline0(12,5));
  frucht.type0Breaklines.push_back(Breakline0(8,11));
  test1break0graph(frucht,"Frucht");
  for (i=sum=nopen=0;i<frucht.type0Breaklines.size();i++)
  {
    sum+=frucht.type0Breaklines[i].size();
    nopen+=frucht.type0Breaklines[i].isOpen();
  }
  tassert(sum==18);
  tassert(i>5 && i<10); // the most disjoint cycles is 3
  tassert(nopen==6);
  for (i=1;i<17;i++)
    cycle17.type0Breaklines.push_back(Breakline0(i,i+1));
  cycle17.type0Breaklines.push_back(Breakline0(17,1));
  test1break0graph(cycle17,"cycle17");
  for (i=sum=nopen=0;i<cycle17.type0Breaklines.size();i++)
  {
    sum+=cycle17.type0Breaklines[i].size();
    nopen+=cycle17.type0Breaklines[i].isOpen();
    for (j=0;j<cycle17.type0Breaklines[i].size();j++)
    {
      edg=cycle17.type0Breaklines[i][j];
      tassert(edg[0]==j+1);
      tassert((edg[1]-j-2)%17==0);
    }
  }
  tassert(sum==17);
  tassert(i==1);
  tassert(nopen==0);
  for (i=1;i<17;i++)
    chain17.type0Breaklines.push_back(Breakline0(i,i+1));
  test1break0graph(chain17,"chain17");
  for (i=sum=nopen=0;i<chain17.type0Breaklines.size();i++)
  {
    sum+=chain17.type0Breaklines[i].size();
    nopen+=chain17.type0Breaklines[i].isOpen();
    for (j=0;j<chain17.type0Breaklines[i].size();j++)
    {
      edg=chain17.type0Breaklines[i][j];
      tassert(edg[0]==j+1);
      tassert(edg[1]==j+2);
    }
  }
  tassert(sum==16);
  tassert(i==1);
  tassert(nopen==1);
  doc.pl.clear();
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

void testzoom()
{
  int i;
  cout<<"zoom\n";
  for (i=-10;i<=10;i++)
    cout<<setw(3)<<i<<' '<<ldecimal(zoomratio(i))<<endl;
  tassert(fabs(zoomratio(30)-1012)<0.1);
  tassert(largestFit(1000)==29);
  tassert(largestFit(1024)==30);
}

double brentfun0(double x)
{
  return (x+3)*sqr(x-1); // example function in Wikipedia
}

double brentfun1(double x)
{
  return sqr(x)-3; // No exact solution in 8-byte floating point.
}

double brentfun2(double x)
/* From Steven Stage. Brent's method should find the exact root in a finite number of steps.
 * However, in 8-byte floating point, f(2/3.+0.1) is 4.16e-17, not 0.
 */
{
  if (x>2/3.)
    return sqrt(x-2/3.)-0.1;
  else
    return -sqrt(2/3.-x)-0.1;
}

double ibrentfun0(int x)
{
  return (x+3000000)*sqr(x-1000000)/1e18;
}

double ibrentfun1(double x)
{
  return (sqr(x)-3e12)/1e12;
}

void testbrent()
{
  double x,y,res;
  int i;
  brent br,ibr;
  x=6;
  y=M_1PHI;
  res=invquad(5,-1,x,y,7,1);
  tassert(fabs(res-5)<1e-15);
  y=0.5;
  res=invquad(5,-1,x,y,7,1);
  tassert(fabs(res-16/3.)<1e-15);
  cout<<"brent "<<res<<endl;
  for (i=0;i<10;i++)
    res=invquad(tan((double)rng.usrandom()),-5,tan((double)rng.usrandom()),2,tan((double)rng.usrandom()),3);
  x=br.init(-4,brentfun0(-4),4/3.,brentfun0(4/3.));
  //br.setdebug(true);
  cout<<"fun0 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !br.finished();i++)
  {
    y=brentfun0(x);
    cout<<ldecimal(y)<<endl;
    x=br.step(y);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
  tassert(x==-3);
  x=br.init(-1.7,brentfun1(-1.7),1.8,brentfun1(1.8));
  //br.setdebug(true);
  cout<<"fun1 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !br.finished();i++)
  {
    y=brentfun1(x);
    cout<<ldecimal(y)<<endl;
    x=br.step(y);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
  x=br.init(-10,brentfun2(-10),10,brentfun2(10));
  //br.setdebug(true);
  cout<<"fun2 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !br.finished();i++)
  {
    y=brentfun2(x);
    cout<<ldecimal(y)<<endl;
    x=br.step(y);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
  x=ibr.init(-4000000,ibrentfun0(-4000000),1333333,ibrentfun0(1333333),true);
  //ibr.setdebug(true);
  cout<<"ifun0 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !ibr.finished();i++)
  {
    y=ibrentfun0(x);
    cout<<ldecimal(y)<<endl;
    x=ibr.step(y);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
  tassert(x==-3e6);
  x=ibr.init(-1.7e6,ibrentfun1(-1.7e6),1.8e6,ibrentfun1(1.8e6),true);
  //ibr.setdebug(true);
  cout<<"ifun1 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !ibr.finished();i++)
  {
    y=ibrentfun1(x);
    cout<<ldecimal(y)<<endl;
    x=ibr.step(y);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
}

double newtonfun0(double x)
{
  return sqr(x)-3; // No exact solution in 8-byte floating point.
}

double newtonderiv0(double x)
{
  return x*2;
}

void testnewton()
{
  double x,y,z,res;
  int i;
  Newton ne;
  x=ne.init(-1.7,newtonfun0(-1.7),newtonderiv0(-1.7),1.8,newtonfun0(1.8),newtonderiv0(1.8));
  //ne.setdebug(true);
  cout<<"fun0 init "<<ldecimal(x)<<' ';
  for (i=0;i<200 && !ne.finished();i++)
  {
    y=newtonfun0(x);
    z=newtonderiv0(x);
    cout<<ldecimal(y)<<' '<<ldecimal(z)<<endl;
    x=ne.step(y,z);
    cout<<"step "<<ldecimal(x)<<' ';
  }
  cout<<endl;
  tassert(fabs(sqr(x)-3)<1e-15);
}

void testmanysum()
{
  manysum ms,negms;
  int i,j,h;
  double x,naiveforwardsum,forwardsum,pairforwardsum,naivebackwardsum,backwardsum,pairbackwardsum;
  vector<double> summands;
  double odd[32];
  long double oddl[32];
  int pairtime=0;
  QTime starttime;
  cout<<"manysum"<<endl;
  for (i=0;i<32;i++)
    oddl[i]=odd[i]=2*i+1;
  for (i=0;i<32;i++)
  {
    tassert(pairwisesum(odd,i)==i*i);
    tassert(pairwisesum(oddl,i)==i*i);
  }
  ms.clear();
  summands.clear();
  tassert(pairwisesum(summands)==0);
  for (naiveforwardsum=i=0;i>-7;i--)
  {
    x=pow(1000,i);
    for (j=0;j<(slowmanysum?1000000:100000);j++)
    {
      naiveforwardsum+=x;
      ms+=x;
      negms-=x;
      summands.push_back(x);
    }
  }
  ms.prune();
  forwardsum=ms.total();
  tassert(forwardsum==-negms.total());
  starttime.start();
  pairforwardsum=pairwisesum(summands);
  pairtime+=starttime.elapsed();
  ms.clear();
  summands.clear();
  for (naivebackwardsum=0,i=-6;i<1;i++)
  {
    x=pow(1000,i);
    for (j=0;j<(slowmanysum?1000000:100000);j++)
    {
      naivebackwardsum+=x;
      ms+=x;
      summands.push_back(x);
    }
  }
  ms.prune();
  backwardsum=ms.total();
  starttime.start();
  pairbackwardsum=pairwisesum(summands);
  pairtime+=starttime.elapsed();
  cout<<"Forward: "<<ldecimal(naiveforwardsum)<<' '<<ldecimal(forwardsum)<<' '<<ldecimal(pairforwardsum)<<endl;
  cout<<"Backward: "<<ldecimal(naivebackwardsum)<<' '<<ldecimal(backwardsum)<<' '<<ldecimal(pairbackwardsum)<<endl;
  tassert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  tassert(fabs((pairforwardsum-pairbackwardsum)/(pairforwardsum+pairbackwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-pairforwardsum)/(forwardsum+pairforwardsum))<DBL_EPSILON);
  tassert(fabs((backwardsum-pairbackwardsum)/(backwardsum+pairbackwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  tassert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  tassert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
  ms.clear();
  summands.clear();
  h=slowmanysum?1:16;
  for (naiveforwardsum=i=0;i>-0x360000;i-=h)
  {
    x=exp(i/65536.);
    naiveforwardsum+=x;
    ms+=x;
    summands.push_back(x);
  }
  ms.prune();
  forwardsum=ms.total();
  starttime.start();
  pairforwardsum=pairwisesum(summands);
  pairtime+=starttime.elapsed();
  ms.clear();
  summands.clear();
  for (naivebackwardsum=0,i=-0x35ffff&-h;i<1;i+=h)
  {
    x=exp(i/65536.);
    naivebackwardsum+=x;
    ms+=x;
    summands.push_back(x);
  }
  ms.prune();
  backwardsum=ms.total();
  starttime.start();
  pairbackwardsum=pairwisesum(summands);
  pairtime+=starttime.elapsed();
  cout<<"Forward: "<<ldecimal(naiveforwardsum)<<' '<<ldecimal(forwardsum)<<' '<<ldecimal(pairforwardsum)<<endl;
  cout<<"Backward: "<<ldecimal(naivebackwardsum)<<' '<<ldecimal(backwardsum)<<' '<<ldecimal(pairbackwardsum)<<endl;
  tassert(fabs((forwardsum-backwardsum)/(forwardsum+backwardsum))<DBL_EPSILON);
  tassert(fabs((pairforwardsum-pairbackwardsum)/(pairforwardsum+pairbackwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-pairforwardsum)/(forwardsum+pairforwardsum))<DBL_EPSILON);
  tassert(fabs((backwardsum-pairbackwardsum)/(backwardsum+pairbackwardsum))<DBL_EPSILON);
  tassert(fabs((forwardsum-naiveforwardsum)/(forwardsum+naiveforwardsum))<1000000*DBL_EPSILON);
  tassert(fabs((backwardsum-naivebackwardsum)/(backwardsum+naivebackwardsum))<1000*DBL_EPSILON);
  tassert(fabs((naiveforwardsum-naivebackwardsum)/(naiveforwardsum+naivebackwardsum))>30*DBL_EPSILON);
  cout<<"Time in pairwisesum: "<<pairtime<<endl;
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

void testcircle()
{
  Circle unit(xy(0,0),1.);
  Circle triple(xy(0,0),3.);
  Circle xaxis(xy(0,0),0,0);
  Circle yaxis(xy(0,0),DEG90,0);
  Circle c41(xy(205,0),DEG90,1/205.);
  Circle c43(xy(125,169),DEG180-AT34,1/215.);
  //Circle c43(xy(215,0),DEG90+radtobin(0.001),1/215.);
  /* c43's center is (-4,-3), but its midpoint is 43*(3,4) from its center.
   * The closest approach between c41 and c43 is (164,123) to (168,126).
   */
  PostScript ps;
  BoundRect br;
  int i,shortCount=0,longCount=0;
  xyz sta1,sta3;
  vector<segment> lines;
  polyline envelope;
  array<double,2> close2;
  tassert(triple.curvature()==1/3.);
  ps.open("circle.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  ps.startpage();
  ps.setscale(-3,-3,3,3,degtobin(0));
  for (i=0;i<1080;i+=5)
  {
    sta1=unit.station(degtorad(i));
    sta3=triple.station(degtorad(i));
    ps.line2p(sta1,sta3);
    if (dist(sta1,sta3)<3.16227733)
      shortCount++;
    if (dist(sta1,sta3)>3.162278) // The distance at 45°/135° is sqrt(10).
      longCount++;
    lines.push_back(segment(sta1,sta3));
  }
  tassert(shortCount==longCount);
  ps.spline(unit.approx3d(0.1/ps.getscale()));
  ps.spline(triple.approx3d(0.1/ps.getscale()));
  ps.setcolor(1,0,1);
  for (i=0;i<lines.size();i++)
    envelope.insert(intersection(lines[i],lines[(i+1)%lines.size()]));
  ps.spline(envelope.approx3d(0.1/ps.getscale()));
  ps.endpage();
  ps.startpage();
  ps.setscale(-1,-1,1,1,degtobin(0));
  ps.setcolor(0,0,0);
  for (i=8388608-DEG180;i<=DEG180;i+=16777216)
    ps.line2p(xaxis.station(tan(i)),yaxis.station(cot(i)));
  ps.spline(xaxis.approx3d(0.1/ps.getscale()));
  ps.spline(yaxis.approx3d(0.1/ps.getscale()));
  ps.endpage();
  ps.startpage();
  br.clear();
  br.include(&c41);
  br.include(&c43);
  ps.setscale(br);
  //ps.setscale(-215,-215,215,215,degtobin(0));
  for (i=8388608-DEG180;i<=DEG180;i+=16777216)
    tassert(c43.dirbound(i)<-200);
  ps.setcolor(0,0,0);
  ps.spline(xaxis.approx3d(0.1/ps.getscale()));
  ps.spline(yaxis.approx3d(0.1/ps.getscale()));
  ps.spline(c43.approx3d(0.1/ps.getscale()));
  ps.spline(c41.approx3d(0.1/ps.getscale()));
  sta1=c41.station(0);
  sta3=c43.station(0);
  ps.setcolor(0,0,1);
  ps.line2p(sta1,sta3);
  close2=besidement(c41,c43);
  sta1=c41.station(close2[0]);
  sta3=c43.station(close2[1]);
  cout<<"Circles approach within "<<ldecimal(dist(sta1,sta3))<<endl;
  tassert(fabs(dist(sta1,sta3)-5)<1e-7);
  ps.setcolor(0,0,0);
  ps.line2p(sta1,sta3);
  close2=besidement(xaxis,c43);
  sta1=xaxis.station(close2[0]);
  sta3=c43.station(close2[1]);
  cout<<"Circle and x-axis "<<ldecimal(dist(sta1,sta3))<<endl;
  tassert(fabs(dist(sta1,sta3)-218)<2e-7);
  ps.setcolor(0,0,0);
  ps.line2p(sta1,sta3);
  close2=besidement(c43,yaxis);
  sta1=yaxis.station(close2[1]);
  sta3=c43.station(close2[0]);
  cout<<"Circle and y-axis "<<ldecimal(dist(sta1,sta3))<<endl;
  tassert(fabs(dist(sta1,sta3)-211)<2e-7);
  ps.setcolor(0,0,0);
  ps.line2p(sta1,sta3);
  close2=besidement(xaxis,yaxis);
  tassert(std::isinf(close2[0]) && std::isinf(close2[1]));
  ps.setcolor(0,0,0);
  ps.line2p(sta1,sta3);
  ps.endpage();
  ps.close();
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
  tassert(a.diffarea()==0);
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
  tassert(fabs(a.diffarea()-(M_PI*sqr(500)/6-250*500*M_SQRT_3_4))<1e-4);
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
  double bear[3],len,begcur,endcur,arcarea,spiralarea;
  vector<double> extrema;
  xyz beg(0,0,3),end(300,400,7),sta,pi;
  xy ctr;
  spiralarc a(beg,end),b(beg,0.001,0.001,end),c,arch[10];
  spiralarc az0(xyz(189794.012,496960.750,0),1531654601,0,1145.229168e-6,60.96,0); // From a road in Arizona
  arc aarc(beg,end);
  bezier3d a3d;
  PostScript ps;
  BoundRect br;
  ps.open("spiralarc.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
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
  a.setdelta(radtobin(0.01),0);
  aarc.setdelta(radtobin(0.01),0);
  cout<<"spiralarc "<<a.diffarea()<<" arc "<<aarc.diffarea()<<
    ' '<<ldecimal(a.diffarea()/aarc.diffarea())<<endl;
  a.setdelta(DEG60,0);
  aarc.setdelta(DEG60,0);
  cout<<"spiralarc "<<a.diffarea()<<" arc "<<aarc.diffarea()<<
    ' '<<ldecimal(a.diffarea()/aarc.diffarea())<<endl;
  tassert(fabs(a.diffarea()/aarc.diffarea()-1)<1e-10);
  for (i=1;i<4;i++)
  {
    for (j=0;j<5;j++)
    {
      beg=xyz(cornu(-0.5,i*0.01,j*0.01),0);
      end=xyz(cornu(0.5,i*0.01,j*0.01),0);
      begcur=spiralcurvature(-0.5,i*0.01,j*0.01);
      endcur=spiralcurvature(0.5,i*0.01,j*0.01);
      c=spiralarc(beg,begcur,endcur,end);
      tassert(fabs(c.length()-1)<1e-12);
      spiralarea=c.diffarea();
      if (j)
        cout<<ldecimal(sqr(j*0.01)*(i*0.01)/(spiralarea-arcarea))<<' ';
      else
        arcarea=spiralarea;
    }
    cout<<endl;
  }
  ps.startpage();
  ps.setscale(-10,-10,10,10,degtobin(0));
  // Make something that resembles an Archimedean spiral
  //arch[0]=spiralarc(xyz(-0.5,0,0),2.,2/3.,xyz(1.5,0,0));
  arch[0]=spiralarc(xyz(-0.5,0,0),-DEG90,2.,2/3.,M_PI,0);
  for (i=1;i<10;i++)
    arch[i]=spiralarc(arch[i-1].getend(),arch[i-1].endbearing(),1/(i+0.5),1/(i+1.5),M_PI*(i+1),0);
  for (i=0;i<10;i++)
    a3d+=arch[i].approx3d(0.01);
  ps.spline(a3d);
  for (i=0;i<-10;i++)
  {
    if (i&1)
      ps.setcolor(1,0,0);
    else
      ps.setcolor(0,0,1);
    ps.spline(arch[i].approx3d(0.01));
  }
  cout<<"Archimedes-like spiral ended on "<<arch[9].getend().getx()<<','<<arch[9].getend().gety()<<endl;
  tassert(dist(arch[9].getend(),xy(-0.752,-10.588))<0.001);
  ps.endpage();
  ps.startpage();
  pi=xyz(az0.pointOfIntersection(),0);
  br.clear();
  br.include(&az0);
  br.include(pi);
  ps.setscale(br);
  cout<<"Arizona spiral\nEndpoint: "<<az0.getend().getx()<<','<<az0.getend().gety()<<endl;
  tassert(dist(az0.getend(),xy(189780.746,496901.254))<0.0007);
  cout<<"Chord: "<<az0.chordlength()<<'<'<<bintodeg(az0.chordbearing())<<endl;
  cout<<"Point of intersection: "<<ldecimal(pi.getx())<<','<<ldecimal(pi.gety())<<endl;
  tassert(dist(pi,xy(189784.706,496921.187))<0.0007);
  cout<<"Tangents: "<<az0.tangentLength(START)<<", "<<az0.tangentLength(END)<<endl;
  tassert(fabs(az0.tangentLength(START)-40.643)<0.0007);
  tassert(fabs(az0.tangentLength(END)-20.322)<0.0007);
  ps.spline(az0.approx3d(0.001/ps.getscale()));
  ps.line2p(az0.getstart(),pi);
  ps.line2p(pi,az0.getend());
  ps.endpage();
  ps.trailer();
  ps.close();
}

void spiralmicroscope(segment *a,double aalong,segment *b,double balong,string fname,int scale=1)
{
  int i,alim,blim;
  xy point;
  double apow2,bpow2,ainc,binc;
  double minx=INFINITY,miny=INFINITY,maxx=-INFINITY,maxy=-INFINITY;
  vector<xy> apoints,bpoints;
  PostScript ps;
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
    point=(a->station(aalong+ainc*i*scale));
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
    point=(b->station(balong+binc*i*scale));
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
  ps.open(fname);
  ps.prolog();
  ps.startpage();
  ps.setscale(minx,miny,maxx,maxy,0);
  ps.setcolor(1,0,0);
  for (i=0;i<apoints.size();i++)
    ps.dot(apoints[i]);
  ps.setcolor(0,0,1);
  for (i=0;i<bpoints.size();i++)
    ps.dot(bpoints[i]);
  ps.endpage();
  ps.startpage();
  ps.setscale(minx,miny,maxx,maxy,0);
  ps.setcolor(0,0,1);
  for (i=0;i<apoints.size();i++)
  {
    if (i==apoints.size()/2)
      ps.setcolor(0,0.6,0);
    ps.dot(apoints[i]);
    if (i==apoints.size()/2)
      ps.setcolor(0,0,1);
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(minx,miny,maxx,maxy,0);
  ps.setcolor(0,0,1);
  for (i=0;i<bpoints.size();i++)
  {
    if (i==bpoints.size()/2)
      ps.setcolor(0,0.6,0);
    ps.dot(bpoints[i]);
    if (i==bpoints.size()/2)
      ps.setcolor(0,0,1);
  }
  ps.endpage();
  ps.trailer();
  ps.close();
}

void testcogospiral1(segment *a,double a0,double a1,segment *b,double b0,double b1,bool extend,xy inter,PostScript &ps,string fname)
{
  int i,n=0;
  xy intpoint; // (7,11)
  vector<alosta> intlistSecant,intlistTangent;
  BoundRect br;
  br.include(a);
  br.include(b);
  ps.startpage();
  ps.setscale(br);
  ps.setcolor(1,0,0);
  ps.spline(a->approx3d(0.001/ps.getscale()));
  ps.setcolor(0,0,1);
  ps.spline(b->approx3d(0.001/ps.getscale()));
  ps.endpage();
  intlistSecant=intersection1(a,a0,a1,b,b0,b1,extend);
  intlistTangent=intersection1(a,a0,b,b0,extend);
  // It is valid for one method, but not the other, to find an intersection.
  cout<<"testcogospiral: "<<intlistSecant.size()<<" alostas by secant method, ";
  cout<<intlistTangent.size()<<" alostas by tangent method"<<endl;
  intpoint=xy(0,0);
  for (i=0;i<intlistSecant.size();i++,n++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlistSecant[i].along<<' '<<ldecimal(intlistSecant[i].station.east())<<' '<<ldecimal(intlistSecant[i].station.north())<<endl;
    intpoint+=intlistSecant[i].station;
  }
  for (i=0;i<intlistTangent.size();i++,n++)
  {
    cout<<((i&1)?"b: ":"a: ")<<intlistTangent[i].along<<' '<<ldecimal(intlistTangent[i].station.east())<<' '<<ldecimal(intlistTangent[i].station.north())<<endl;
    intpoint+=intlistTangent[i].station;
  }
  intpoint/=n;
  if (inter.isfinite())
    tassert(dist(intpoint,inter)<1e-5);
  if (fname.length() && intlistSecant.size())
    spiralmicroscope(a,intlistSecant[0].along,b,intlistSecant[1].along,fname);
}

spiralarc snip20(spiralarc a,spiralarc &d,spiralarc &c)
/* Snips up to 20% off each end. Computing all intersections is done by
 * picking equally-spaced points along the two curves and computing one
 * intersection for each pair. This makes sure that the equally-spaced
 * points are positioned randomly with respect to each other.
 */
{
  double len,begcut,endcut;
  spiralarc b,e;
  int bcut,ecut;
  bcut=rng.usrandom();
  ecut=rng.usrandom();
  cout<<bcut<<' '<<ecut<<" snip20\n";
  len=a.length();
  endcut=(0.8+ecut/327680.)*len;
  begcut=(0.2-bcut/327680.)*len;
  a.split(endcut,b,c);
  b.split(begcut,d,e);
  return e;
}

void testcogospiral2(spiralarc a,spiralarc b,PostScript &ps,vector<xy> expected,double toler,unsigned nint)
/* Tests all intersections of a and b. They should be within toler of some
 * subsequence of expected, and the number should be a bit set in nint.
 * nint is a bitmask; 5 means that the number of intersections should be 0 or 2.
 */
{
  spiralarc aSnip,aStart,aEnd,bSnip,bStart,bEnd,bNeg;
  BoundRect br;
  vector<array<alosta,2> > inters;
  vector<alosta> beside;
  int i,j;
  aSnip=snip20(a,aStart,aEnd);
  bSnip=snip20(b,bStart,bEnd);
  bNeg=-bSnip;
  br.include(&a);
  br.include(&b);
  ps.startpage();
  ps.setscale(br);
  ps.setcolor(1,0,0);
  ps.spline(aStart.approx3d(0.001/ps.getscale()));
  ps.spline(bStart.approx3d(0.001/ps.getscale()));
  ps.setcolor(0,0,0);
  ps.spline(aSnip.approx3d(0.001/ps.getscale()));
  ps.spline(bSnip.approx3d(0.001/ps.getscale()));
  ps.setcolor(0,0,1);
  ps.spline(aEnd.approx3d(0.001/ps.getscale()));
  ps.spline(bEnd.approx3d(0.001/ps.getscale()));
  inters=intersections(&aSnip,&bSnip);
  beside=besidement1(&aSnip,aSnip.length()/2,&bNeg,bSnip.length()/2);
  tassert((nint>>inters.size())&1);
  if (((nint>>inters.size())&1)==0)
  {
    cout<<inters.size()<<" intersections found, should be ";
    for (i=j=0;i<32;i++)
      if ((nint>>i)&1)
      {
	if (j)
	  cout<<" or ";
	cout<<i;
	j++;
      }
    cout<<endl;
  }
  for (i=j=0;i<inters.size();i++)
  {
    ps.circle(inters[i][0].station,1/ps.getscale());
    for (;j<expected.size() && (dist(expected[j],inters[i][0].station)>toler || dist(expected[j],inters[i][1].station)>toler);j++);
    if (j==expected.size())
      cout<<"Unexpected intersection:\n";
    tassert(j<expected.size());
    j++;
    cout<<ldecimal(inters[i][0].along)<<' '<<ldecimal(inters[i][1].along)
        <<' '<<inters[i][1].bearing-inters[i][0].bearing<<'\n';
    cout<<ldecimal(inters[i][0].station.getx())<<','<<ldecimal(inters[i][0].station.gety())<<' '
        <<ldecimal(inters[i][1].station.getx())<<','<<ldecimal(inters[i][1].station.gety())<<'\n';
  }
  ps.setcolor(0,1,0);
  if (beside.size())
  {
    ps.line2p(beside[0].station,beside[1].station);
    cout<<"beside: bearing diff "<<foldangle(beside[1].bearing-beside[0].bearing)<<endl;
    tassert(abs(foldangle(beside[1].bearing-beside[0].bearing))<2);
  }
  ps.endpage();
}

void testcogospiral()
{
  int i;
  xy intpoint; // (7,11)
  vector<alosta> intlist;
  vector<xy> expected;
  xyz beg0(-1193,-489,0),end0(0xc07,0x50b,0), // slope 5/12
      beg1(-722,983,0),end1(382,-489,0), // slope -4/3
      beg2(-101,1,0),end2(99,1,0),beg3(-99,-1,0),end3(101,-1,0),
      beg4(-5,0,0),end4(5,0,0),beg5(-4,3,0),end5(4,3,0),
      beg6(5,0,0),end6(0,5,0),beg7(5,2,0),end7(4,9,0);
  segment a(beg0,end0),b(beg1,end1);
  spiralarc aspi,bspi;
  spiralarc c(beg2,end2),d(beg3,end3),e(beg4,end4),f(beg5,end5),g(beg6,end6),h(beg7,end7);
  spiralarc o(xyz(0,0,0),3.5,3.,0,-2,0.7);
  spiralarc p(xyz(0,0,0),3.5,-3.,0,-0.7,2);
  spiralarc q(xyz(0,0,0),3.5,3.,degtobin(-3),-2.03,0.9);
  spiralarc r(xyz(0,0,0),3.5,-3.,degtobin(3),-0.9,2.03);
  spiralarc s(xyz(0,0,0),0.5,0.,0,-M_PI,M_PI);
  spiralarc t(xyz(0,0,0),0.5,0.,DEG180,-M_PI,M_PI);
  PostScript ps;
  ps.open("cogospiral.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  ps.setDoc(doc);
  c.setdelta(0,DEG30);
  d.setdelta(0,-DEG30);
  e.setdelta(-DEG60,0);
  f.setdelta(DEG90,0); // e and f are 0.0034 away from touching
  g.setdelta(DEG90,0);
  h.setdelta(-DEG90,0); // g and h are tangent at (3,4)
  testcogospiral1(&a,0,a.length(),&b,0,b.length(),false,xy(7,11),ps,"straightmicro");
  /* The distance along both lines to the intersection point is exactly an integer,
   * so the two points are exactly equal to (7,11).
   */
  aspi=a;
  bspi=b;
  testcogospiral1(&aspi,0,a.length(),&bspi,0,b.length(),false,xy(7,11),ps,"straightmicro");
  /* The distances along the curves are in the hundreds, but the midpoints
   * are closer to the origin, so the two intersection points do not exactly
   * coincide. They aren't exactly (7,11) either, because the bearings have
   * been rounded to the nearest 1657th of a second.
   */
  testcogospiral1(&c,50,c.length()-50,&d,50,c.length()-50,false,xy(7.65883,-0.129029),ps,"spiralmicro");
  testcogospiral1(&e,0,1,&f,0,1,false,xy(NAN,NAN),ps,"");
  testcogospiral1(&g,0,0.1,&h,0,0.1,false,xy(NAN,NAN),ps,"");
  /* These are tangent, so it can't find the intersection. As represented in the
   * computer, they are about 2e-9 apart.
   */
  //spiralmicroscope(g,3.2175055439642193,h,1.4189705460416392281,"tangentmicro",0x669);
  spiralmicroscope(&g,3.2175384147219286,&h,1.419003418926355,"tangentmicro",0x669);
  expected.clear();
  expected.push_back(xy(0,0));
  testcogospiral2(o,p,ps,expected,0.0015,2);
  expected.clear();
  expected.push_back(xy(0,1.193622997));
  expected.push_back(xy(-0.248290956,0.145341333));
  expected.push_back(xy(0,0));
  expected.push_back(xy(0.248290956,0.145341333));
  expected.push_back(xy(0,0.414137715));
  testcogospiral2(q,r,ps,expected,1.4e-8,52);
  expected.clear();
  expected.push_back(xy(0,0));
  expected.push_back(xy(0,0));
  /* When tested without snipping, this produces 3 intersections, which is wrong.
   * Also, the following snip numbers produce two intersections with bearing
   * difference only 108 units (0.652"), which looks like a single intersection:
   * 59807 63087
   * 53498 39253
   * These snip numbers produce intersections at x=±5.067e-1:
   * 24581 17857
   * 18613 4542
   */
  testcogospiral2(s,t,ps,expected,5.1e-5,7);
}

void test1curly(double curvature,double clothance,PostScript &ps,double tCurlyLength,double tTooCurlyLength,double tMaxLength)
{
  double curlyLength,tooCurlyLength,maxLength;
  spiralarc s;
  double lo=0,hi=1,mid;
  BoundRect br;
  while (s.getstart().isfinite() && s.getend().isfinite())
  {
    hi*=2;
    s=spiralarc(xyz(0,0,0),curvature,clothance,0,-hi/2,hi/2);
  }
  while (true)
  {
    mid=(hi+lo)/2;
    if (mid==hi || mid==lo)
      break;
    s=spiralarc(xyz(0,0,0),curvature,clothance,0,-mid/2,mid/2);
    if (s.getstart().isfinite() && s.getend().isfinite())
      lo=mid;
    else
      hi=mid;
  }
  maxLength=lo;
  // Compute curlyLength
  lo=0;
  hi=maxLength;
  while (true)
  {
    mid=(hi+lo)/2;
    if (mid==hi || mid==lo)
      break;
    s=spiralarc(xyz(0,0,0),curvature,clothance,0,-mid/2,mid/2);
    if (s.isCurly())
      hi=mid;
    else
      lo=mid;
  }
  curlyLength=mid;
  // Compute tooCurlyLength
  lo=0;
  hi=maxLength;
  while (true)
  {
    mid=(hi+lo)/2;
    if (mid==hi || mid==lo)
      break;
    s=spiralarc(xyz(0,0,0),curvature,clothance,0,-mid/2,mid/2);
    if (s.isTooCurly())
      hi=mid;
    else
      lo=mid;
  }
  tooCurlyLength=mid;
  br.clear();
  br.include(xy(-0.5,-0.43));
  br.include(xy(0.5,0.43));
  br.include(&s);
  s=spiralarc(xyz(0,0,0),curvature,clothance,0,-maxLength/2,maxLength/2);
  br.include(s.getstart()); // Including the whole curve would take too long
  br.include(s.getend());   // because it has many spiral turns.
  ps.startpage();
  ps.setscale(br);
  ps.spline(s.approx3d(0.001/ps.getscale()));
  //ps.line2p(s.getstart(),s.getend());
  ps.write(xy(-0.5,-0.3),"Curvature "+ldecimal(curvature));
  ps.write(xy(-0.5,-0.325),"Clothance "+ldecimal(clothance));
  ps.write(xy(-0.5,-0.35),"Maximum length "+ldecimal(maxLength));
  ps.endpage();
  ps.startpage();
  ps.setscale(br);
  s=spiralarc(xyz(0,0,0),curvature,clothance,0,-curlyLength/2,curlyLength/2);
  ps.spline(s.approx3d(0.001/ps.getscale()));
  ps.setcolor(0,1,0);
  ps.spline(Circle(s.getstart(),s.startbearing()+DEG90).approx3d(1/ps.getscale()));
  ps.spline(Circle(s.getend(),s.endbearing()+DEG90).approx3d(1/ps.getscale()));
  ps.setcolor(0,0,0);
  ps.write(xy(-0.5,-0.3),"Curvature "+ldecimal(curvature));
  ps.write(xy(-0.5,-0.325),"Clothance "+ldecimal(clothance));
  ps.write(xy(-0.5,-0.35),"Curly length "+ldecimal(curlyLength));
  ps.endpage();
  ps.startpage();
  ps.setscale(br);
  s=spiralarc(xyz(0,0,0),curvature,clothance,0,-tooCurlyLength/2,tooCurlyLength/2);
  ps.spline(s.approx3d(0.001/ps.getscale()));
  ps.setcolor(0,1,0);
  ps.spline(Circle(s.getstart(),s.startbearing()).approx3d(1/ps.getscale()));
  ps.spline(Circle(s.getend(),s.endbearing()).approx3d(1/ps.getscale()));
  ps.setcolor(0,0,0);
  ps.write(xy(-0.5,-0.3),"Curvature "+ldecimal(curvature));
  ps.write(xy(-0.5,-0.325),"Clothance "+ldecimal(clothance));
  ps.write(xy(-0.5,-0.35),"Too curly length "+ldecimal(tooCurlyLength));
  ps.endpage();
  tassert(std::isnan(tMaxLength) || fabs(log(maxLength/tMaxLength))<1e-9);
  tassert(std::isnan(tCurlyLength) || fabs(log(curlyLength/tCurlyLength))<1e-9);
  tassert(std::isnan(tTooCurlyLength) || fabs(log(tooCurlyLength/tTooCurlyLength))<1e-9);
}

void testcurly()
/* This outputs "cornu needs more iterations" as it finds maxLength.
 * maxLength may depend on MAXITER in spiral.cpp.
 */
{
  int i;
  double curvature,clothance;
  spiralarc s;
  PostScript ps;
  ps.open("curly.ps");
  ps.setpaper(papersizes["A4 landscape"],0);
  ps.prolog();
  for (i=0;i<10;i++)
  {
    curvature=(rng.usrandom()-32767.5)/2896.31;
    clothance=(rng.usrandom()-32767.5)/1024;
    test1curly(curvature,clothance,ps,NAN,NAN,NAN);
  }
  test1curly(0,28.25,ps,0.761013024660324,NAN,3.238467135241204);
  test1curly(10,0,ps,M_PI/10,M_PI/5,NAN);
  test1curly(10,0.4,ps,NAN,NAN,NAN);
}

void testcurvefit()
/* Test fitting a 3D curve to a sequence of points, which are somewhat evenly
 * spaced along the centerline of a rural road.
 */
{
  criterion crit1;
  doc.makepointlist(2);
  doc.pl[0].clear();
  doc.pl[1].crit.clear();
  doc.pl[2].crit.clear();
  crit1.str="CL";
  crit1.istopo=true;
  doc.pl[1].crit.push_back(crit1);
  crit1.str="DR";
  doc.pl[2].crit.push_back(crit1);
  /* These coordinates are N.C. grid coordinates. The COW DRs are two places,
   * one across the street from the other, where cows can cross the street
   * when the gates are open. The NEW CLs (except one where I forgot to erase
   * NEW) are places where the latest painted centerlines (all of which are
   * double yellow lines) don't match the lines painted in a previous year.
   */
  doc.pl[0].addpoint(1,point(330043.02478,193167.25356,335.4551,"DR"));
  doc.pl[0].addpoint(2,point(330046.26158,193160.47584,335.18331,"DR"));
  doc.pl[0].addpoint(3,point(330085.99731,193085.00785,332.08863,"DR"));
  doc.pl[0].addpoint(4,point(330083.44072,193093.02286,332.50786,"DR"));
  doc.pl[0].addpoint(5,point(329958.49496,192871.18366,313.52201,"COW DR"));
  doc.pl[0].addpoint(6,point(329955.95815,192863.64198,312.76765,"COW DR"));
  doc.pl[0].addpoint(7,point(329951.91737,192869.4372,313.48706,"COW DR"));
  doc.pl[0].addpoint(8,point(329954.17955,192875.17545,313.96278,"COW DR"));
  doc.pl[0].addpoint(9,point(329951.10681,192827.73858,309.90544,"EP"));
  doc.pl[0].addpoint(10,point(329956.38912,192829.53094,309.69212,"EP"));
  doc.pl[0].addpoint(11,point(329953.59211,192829.01894,309.94432,"CL"));
  doc.pl[0].addpoint(12,point(329951.03268,192844.28448,311.29915,"CL"));
  doc.pl[0].addpoint(13,point(329952.46425,192861.25285,312.80239,"CL"));
  doc.pl[0].addpoint(14,point(329958.36131,192877.62687,314.31337,"CL"));
  doc.pl[0].addpoint(15,point(329967.36296,192892.08981,315.71217,"CL"));
  doc.pl[0].addpoint(16,point(329978.32641,192905.81388,317.02939,"CL"));
  doc.pl[0].addpoint(17,point(329989.83452,192918.27588,318.27136,"CL"));
  doc.pl[0].addpoint(18,point(330002.09472,192930.76048,319.51392,"CL"));
  doc.pl[0].addpoint(19,point(330014.3626,192943.17238,320.73134,"CL"));
  doc.pl[0].addpoint(20,point(330026.68172,192955.41608,321.84994,"CL"));
  doc.pl[0].addpoint(21,point(330039.14558,192967.82724,323.12446,"CL"));
  doc.pl[0].addpoint(22,point(330051.34198,192980.57416,324.22264,"CL"));
  doc.pl[0].addpoint(23,point(330062.0244,192993.29161,325.26457,"NEW CL"));
  doc.pl[0].addpoint(24,point(330071.32296,193007.75264,326.42237,"CL"));
  doc.pl[0].addpoint(25,point(330078.32532,193023.53127,327.71374,"CL"));
  doc.pl[0].addpoint(26,point(330083.31259,193040.11129,328.98621,"NEW CL"));
  doc.pl[0].addpoint(27,point(330085.71121,193056.82687,330.29308,"CL"));
  doc.pl[0].addpoint(28,point(330085.17708,193074.5715,331.34593,"CL"));
  doc.pl[0].addpoint(29,point(330080.97383,193091.95369,332.35184,"NEW CL"));
  doc.pl[0].addpoint(30,point(330075.06052,193106.86078,333.01328,"NEW CL"));
  doc.pl[0].addpoint(31,point(330068.10206,193121.73217,333.6529,"CL"));
  doc.pl[0].addpoint(32,point(330062.36704,193133.57594,334.23896,"CL"));
  doc.pl[0].addpoint(33,point(330054.89422,193148.78755,334.92296,"CL"));
  doc.pl[0].addpoint(34,point(330047.04195,193165.06163,335.48276,"CL"));
  doc.pl[0].addpoint(35,point(330039.426,193181.18002,335.90564,"CL"));
  doc.pl[0].addpoint(36,point(330032.10575,193196.9071,336.4109,"CL"));
  doc.pl[0].addpoint(37,point(330024.80078,193212.69583,336.70285,"CL"));
  doc.pl[0].addpoint(38,point(330017.47856,193228.51048,337.0227,"CL"));
  doc.pl[0].addpoint(39,point(330010.3409,193244.3484,337.29339,"CL"));
  doc.pl[0].addpoint(40,point(330003.6787,193260.15507,337.62253,"CL"));
  doc.pl[0].addpoint(41,point(329997.55018,193276.0551,337.84381,"CL"));
  doc.pl[0].addpoint(42,point(329992.41288,193291.8858,338.03719,"CL"));
  doc.pl[0].addpoint(43,point(329988.44315,193308.6462,338.30774,"CL"));
  doc.pl[0].addpoint(44,point(329986.04724,193325.33593,338.65454,"CL"));
  doc.pl[0].addpoint(45,point(329984.88476,193312.2714,338.455,"DR"));
  doc.pl[0].addpoint(46,point(329990.39617,193289.15558,338.10359,"DR"));
  doc.pl[0].addpoint(47,point(329997.99308,193267.57183,337.78719,"DR"));
  doc.pl[0].addpoint(48,point(329999.62166,193263.35751,337.73799,"DR"));
  doc.pl[0].addpoint(49,point(330039.35264,193188.00773,336.01316,"DR"));
  doc.pl[0].addpoint(50,point(330041.18982,193184.11305,335.90922,"DR"));
  doc.copytopopoints(1,0); // centerline
  doc.copytopopoints(2,0); // driveways
}

void test1manyarc(spiralarc s,PostScript &ps)
{
  int narcs,i,j,ncenters;
  polyarc approx;
  vector<xy> crossings;
  vector<segment> tickmarks;
  arc arc1;
  int crossingsPerArc[4];
  vector<array<alosta,2> > crossings1;
  xy enddiff;
  bool showCenters=false;
  BoundRect br;
  for (i=0;i<4;i++)
    crossingsPerArc[i]=0;
  cout<<"Throw "<<s.sthrow();
  cout<<" Estimated throw "<<(s.curvature(s.length())-s.curvature(0))*sqr(s.length())/24<<'\n';
  for (narcs=2;narcs<9;narcs++)
  {
    approx=manyArc(s,narcs);
    enddiff=approx.getend()-s.getend();
    cout<<narcs<<" arcs, ";
    tassert(enddiff.length()==0);
    cout<<"mean square error "<<meanSquareDistance(approx,s);
    cout<<" linear error "<<maxError(approx,s)<<endl;
    crossings.clear();
    tickmarks.clear();
    for (i=0;i<narcs-1;i++)
    {
      /* If the approximation is good, every arc except the first and last
       * intersects the spiral. The first and last arcs are tangent to the
       * spiral at their ends, so it takes a really long time (over 400 calls
       * to spiralarc::station for every call to intersection1) to find
       * the intersections, which fails half the time because of roundoff.
       * Finding the intersections of the intermediate arcs takes only
       * about 10 calls to spiralarc::station per intersection1.
       */
      arc1=approx.getarc(i);
      if (i)
	crossings1=intersections(&s,&arc1);
      else
	crossings1.clear();
      if (crossings1.size()<4)
      {
	crossingsPerArc[crossings1.size()]++;
	if (i && s.clothance()) // If clothance=0, s and approx coincide, and it may find no intersections.
	  tassert(crossings1.size());
      }
      for (j=0;j<crossings1.size();j++)
	crossings.push_back(crossings1[j][0].station);
      tickmarks.push_back(segment(arc1.getend()+xyz(5*cossin(arc1.endbearing()+DEG90),0),
				  arc1.getend()+xyz(5*cossin(arc1.endbearing()-DEG90),0)));
    }
    br.clear();
    br.include(&s);
    br.include(&approx);
    for (i=0;showCenters && i<approx.size();i++)
      br.include(approx.getarc(i).center());
    ps.startpage();
    ps.setscale(br);
    ps.setcolor(0.8,0.8,0);
    for (i=0;i<crossings.size();i++)
      ps.circle(crossings[i],s.length()/100);
    for (i=0;showCenters && i<approx.size();i++)
    {
      if (i&1)
	ps.setcolor(0.2,0.2,1);
      else
	ps.setcolor(0,0.7,0);
      ps.circle(approx.getarc(i).center(),s.length()/50);
    }
    ncenters=9*narcs-4; // The first and last arcs are about 7/9 as long as the others.
    for (i=0;showCenters && i<ncenters;i++)
    {
      if (((i+2)/9)&1)
	ps.setcolor(1,0.2,1);
      else
	ps.setcolor(0.8,0.7,0);
      ps.circle(s.osculatingCircle((i+0.5)/ncenters*s.length()).center(),s.length()/50);
    }
    ps.setcolor(0,0,1);
    ps.spline(s.approx3d(0.01));
    ps.setcolor(0,0,0);
    ps.spline(approx.approx3d(0.01));
    for (i=0;i<tickmarks.size();i++)
      ps.spline(tickmarks[i].approx3d(0.01));
    ps.endpage();
  }
}

void testmanyarc()
/* Approximating a spiralarc by a smooth sequence of arcs.
 * In the approximation where the difference in curvature times the length is
 * small, this approaches the problem of approximating a cubic by a sequence
 * of quadratics.
 */
{
  segment cubic(xyz(-30,0,-27),27,-27,xyz(30,0,27));
  vector<segment> approx;
  segment diff;
  double abscissa,lastabscissa,ordinate,lastordinate;
  double startslope,endslope;
  double x,length,accel,firstlength,maxerror;
  vector<double> vex;
  spiralarc trans(xyz(0,0,0),0,0.003,xyz(500,0,0));
  spiralarc trans43(xyz(0,0,0),0,0.003,xyz(400,300,0));
  spiralarc trans34(xyz(0,0,0),0,0.003,xyz(300,400,0));
  spiralarc symm(xyz(0,0,0),-0.003,0.003,xyz(500,0,0));
  spiralarc straight(xyz(0,0,0),0,0,xyz(500,0,0));
  int narcs,i,j;
  PostScript ps;
  bezier3d spl;
  Quaternion flip=versor(xyz(1,0,0),-DEG90);
  ps.open("manyarc.ps");
  ps.setpaper(papersizes["A4 landscape"],0);
  ps.prolog();
  ps.setDoc(doc);
  for (narcs=2;narcs<2;narcs++)
  {
    ps.startpage();
    ps.setscale(-30,-27,30,27);
    spl=cubic.approx3d(1);
    spl.rotate(flip);
    approx.clear();
    ps.setcolor(0,0,1);
    ps.spline(spl);
    spl=bezier3d();
    approx=manyQuad(cubic,narcs);
    for (i=0;i<narcs;i++)
      spl+=approx[i].approx3d(1);
    spl.rotate(flip);
    ps.setcolor(1,0,0);
    ps.spline(spl);
    endslope=approx.back().endslope();
    ordinate=approx.back().getend().elev();
    cout<<"endslope is "<<endslope<<", should be "<<cubic.endslope()<<' '<<ldecimal(endslope-cubic.endslope())<<endl;
    cout<<"ordinate is "<<ordinate<<", should be "<<27<<endl;
    cout<<"p="<<ldecimal(manyArcTrim(narcs))<<endl;
    firstlength=approx[0].length();
    diff=segment(xyz(-30,0,0),xyz(firstlength-30,0,cubic.elev(firstlength)-approx[0].getend().elev()));
    diff.setslope(START,0);
    diff.setslope(END,cubic.slope(firstlength)-approx[0].endslope());
    vex=diff.vextrema(false);
    cout<<"Greatest separation at "<<ldecimal(vex[0])<<" is "<<ldecimal(-diff.station(vex[0]).elev())<<endl;
    /* Approximate empirical formula for the largest separation:
     * f(narcs)=sqrt(432)/((narcs-0.230201)**3-narcs*sqrt(3/16)).
     * For a fixed narcs, the separation is proportional to the difference in
     * second derivative (curvature of a spiral) times the square of length.
     * The difference in curvature times the length is invariant when a spiralarc
     * is scaled.
     * For this cubic, the difference in 2nd deriv times square of length is 1296.
     * The difference in curvature times the square of the length divided by 24
     * is the estimate of throw. This cubic's estimated (and exact) throw is 54.
     */
    ps.endpage();
  }
  for (narcs=2;narcs<21;narcs++)
  {
    maxerror=maxErrorCubic(narcs);
    cout<<narcs<<" arcs, p="<<manyArcTrim(narcs)<<", max error is "<<maxerror<<endl;
    tassert(fabs(1/maxerror-(sqrt(6.75)*cub(narcs-0.230201)-narcs*M_SQRT_3_4/2))<0.04);
  }
  cout<<"limit p="<<ldecimal(manyArcTrim(2147483647))<<endl;
  test1manyarc(trans,ps);
  test1manyarc(trans43,ps);
  test1manyarc(trans34,ps);
  test1manyarc(symm,ps);
  test1manyarc(straight,ps);
  ps.close();
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
  PostScript ps;
  doc.makepointlist(1);
  doc.pl[1].clear();
  aster(doc,1000);
  ps.open("closest.ps");
  ps.prolog();
  for (i=0;i<5;i++)
  {
    a.setdelta(d1[i],d2[i]);
    ps.startpage();
    minquick=INFINITY;
    /* minquick is the minimum distance that it calculates quickly and inaccurately.
     * It should be greater than closesofar, which is 15.
     */
    ps.setscale(-32,-32,32,32,0);
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
	    ps.setcolor(0,0,1);
	  else
	    ps.setcolor(1,0,0);
	else
	  ps.setcolor(0,0,0);
	ps.line2p(doc.pl[1].points[j],a.station(close));
      }
    }
    ps.endpage();
    cout<<"Minimum distance that is calculated inaccurately is "<<minquick<<endl;
    tassert(minquick>15);
  }
  ps.trailer();
  ps.close();
}

void testspiral()
{
  xy a,b,c,d,limitpoint;
  int i,j,bearing,bearing2,lastbearing,curvebearing,diff,badcount;
  double t,t2;
  float segalo[]={-5.96875,-5.65625,-5.3125,-4.875,-4.5,-4,-3.5,-2.828125,-2,0,
    2,2.828125,3.5,4,4.5,4.875,5.3125,5.65625,5.96875};
  vector<xy> spoints;
  vector<int> sbearings;
  bezier3d a3d;
  spiralarc sarc;
  PostScript ps;
  a=cornu(0);
  tassert(a==xy(0,0));
  ps.open("spiral.ps");
  ps.prolog();
  ps.startpage();
  ps.setscale(-1,-1,1,1,degtobin(0));
  //widen(10);
  for (i=-120;i<121;i++)
  {
    b=cornu(t=i/20.);
    if (i*i==14400)
    {
      limitpoint=b;
      ps.dot(b);
    }
    else
      if (i>-119)
	ps.line2p(c,b);
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
  ps.setcolor(0,0,1);
  ps.line2p(a,b);
  ps.endpage();
  //b=cornu(1,1,1);
  ps.startpage();
  for (j=-3;j<=3;j++)
  {
    switch ((j+99)%3)
    {
      case 0:
	ps.setcolor(1,0,0);
	break;
      case 1:
	ps.setcolor(0,0.4,0);
	break;
      case 2:
	ps.setcolor(0,0,1);
	break;
    }
    for (i=-20;i<21;i++)
    {
      b=cornu(t=i/20.,2*j,2);
      if (i>-20)
      {
	ps.line2p(c,b);
	tassert(dist(c,b)>0.049 && dist(c,b)<=0.05);
	//cout<<dist(c,b)<<' ';
      }
      c=b;
    }
    //cout<<endl;
  }
  ps.endpage();
  //ps.startpage();
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
    //ps.line2p(b,c);
    b=cornu(i/20.,1,0);
    c=xy(0,1);
    //cout<<i<<' '<<dist(b,c)-1<<endl;
    tassert(fabs(dist(b,c)-1)<1e-12); // it's 0 or -1.11e-16 on 64-bit
  }
  //ps.endpage();
  ps.startpage();
  ps.setscale(-1,-1,1,1,0);
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
  ps.spline(a3d);
  for (bearing=i=0,lastbearing=1;i<100 && bearing!=lastbearing;i++)
  {
    t=bintorad(bearing);
    a=cornu(-sqrt(t));
    b=cornu(sqrt(t+M_PI/2));
    lastbearing=bearing;
    bearing=dir(a,b);
  }
  for (bearing2=i=0,lastbearing=1;i<100 && bearing2!=lastbearing;i++)
  {
    t2=bintorad(bearing2);
    c=cornu(-sqrt(t2));
    d=cornu(sqrt(t2+M_PI));
    lastbearing=bearing2;
    bearing2=dir(c,d);
  }
  ps.dot(limitpoint);
  ps.dot(-limitpoint);
  ps.setcolor(0,0,1);
  ps.line2p(a,b);
  //ps.line2p(c,d);
  ps.endpage();
  ps.trailer();
  ps.close();
  tassert(bearing==162105696);
  tassert(bearing2==229309921);
  cout<<"Barely curly spiralarc is from "<<ldecimal(-sqrt(t))<<" to "<<ldecimal(sqrt(t+M_PI/2))<<endl;
  cout<<"Barely too curly spiralarc is from "<<ldecimal(-sqrt(t2))<<" to "<<ldecimal(sqrt(t2+M_PI))<<endl;
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
  xy h(-5,0),j(-1,0);
  doc.pl[0].clear();
  // Start with an identically 0 surface. The elevation at the center should be 0.
  doc.pl[0].addpoint(1,point(1,0,0,"eip"));
  doc.pl[0].addpoint(1,point(-0.5,M_SQRT_3_4,0,"eip"));
  doc.pl[0].addpoint(1,point(-0.5,-M_SQRT_3_4,0,"eip"));
  tri.a=&doc.pl[0].points[1];
  tri.b=&doc.pl[0].points[2];
  tri.c=&doc.pl[0].points[3];
  tri.flatten(); // sets area, needed for computing elevations
#ifndef FLATTRIANGLE
  for (i=0;i<7;i++)
    tri.ctrl[i]=0;
#endif
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  tassert(elev==0);
  // Now make a constant surface at elevation 1.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(1);
  doc.pl[0].points[3].setelev(1);
#ifndef FLATTRIANGLE
  for (i=0;i<7;i++)
    tri.ctrl[i]=1;
#endif
  elev=tri.elevation(o);
  printf("elevation=%f\n",elev);
  tassert(fabs(elev-1)<1e-15);
  // Now make a linear surface.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(0);
  doc.pl[0].points[3].setelev(0);
#ifndef FLATTRIANGLE
  tri.ctrl[0]=tri.ctrl[1]=2/3.;
  tri.ctrl[2]=tri.ctrl[4]=1/3.;
  tri.ctrl[5]=tri.ctrl[6]=0;
  tri.setcentercp();
#endif
  elev=tri.elevation(o);
#ifndef FLATTRIANGLE
  printf("ctrl[3]=%f elevation=%f\n",tri.ctrl[3],elev);
#endif
  tassert(abs(elev*3-1)<1e-7);
  // Now make a quadratic surface. It is a paraboloid z=r². Check that the cubic component is 0.
  doc.pl[0].points[1].setelev(1);
  doc.pl[0].points[2].setelev(1);
  doc.pl[0].points[3].setelev(1);
#ifndef FLATTRIANGLE
  for (i=0;i<7;i++)
    tri.ctrl[i]=0;
  tri.setcentercp();
#endif
  elev=tri.elevation(o);
  elevd=tri.elevation(d);
  elevg=tri.elevation(g);
  eleva=tri.elevation(a);
#ifndef FLATTRIANGLE
  printf("ctrl[3]=%f elevation=%f %f %f %f\n",tri.ctrl[3],elevd,elev,elevg,eleva);
#endif
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
#ifndef FLATTRIANGLE
  printf("ctrl[3]=%f elevation=%f %f %f %f\n",tri.ctrl[3],elevd,elev,elevg,eleva);
#endif
  tassert(abs(elevd-elev*3+elevg*3-eleva)<1e-7);
  tassert(abs(elev-1)<1e-7);
  tassert(tri.inCircle(o,0.1));
  tassert(tri.inCircle(j,0.51));
  tassert(!tri.inCircle(h,1));
}

void testmeasure()
{
  Measure meas;
  Measurement parsed;
  xy xy0,xy1,xy2;
  string measStr;
  double easting=443615.85705156205; // of point H, an EIR in Independence Park
  double longitude=-1.42977054329272687479; // of OAKLAND, a benchmark
  meas.addUnit(KILOMETER);
  meas.addUnit(METER);
  meas.addUnit(MILLIMETER);
  meas.addUnit(GRAM);
  meas.addUnit(KILOGRAM);
  meas.setDefaultUnit(LENGTH,0.552); // geometric mean of meter and foot
  meas.setDefaultPrecision(LENGTH,1.746e-3); // g.m. of 1 mm and 0.01 ft
  meas.setDefaultPrecision(ANGLE,1.57e-10); // 1 mm in a radian of earth
  meas.setDefaultUnit(MASS,0.6735);
  tassert(meas.findUnit(LENGTH)==METER);
  tassert(meas.findUnit(MASS)==KILOGRAM);
  tassert(meas.findPrecision(LENGTH)==3);
  measStr=meas.formatMeasurementUnit(easting,LENGTH);
  cout<<"In meters, easting is "<<measStr<<endl;
  tassert(measStr=="443615.857 m");
  meas.setFoot(INTERNATIONAL);
  meas.clearUnits();
  meas.addUnit(FOOT);
  meas.addUnit(CHAIN);
  meas.addUnit(POUND);
  meas.addUnit(DEGREE);
  meas.addUnit(GON);
  meas.addUnit(ARCSECOND+DECIMAL+FIXLARGER);
  meas.addUnit(DEGREE_B);
  meas.addUnit(GON_B);
  meas.addUnit(ARCSECOND_B+DECIMAL+FIXLARGER);
  tassert(meas.findUnit(LENGTH)==FOOT);
  tassert(meas.findUnit(MASS)==POUND);
  tassert(meas.findPrecision(LENGTH)==2);
  measStr=meas.formatMeasurementUnit(easting,LENGTH);
  cout<<"In international feet, easting is "<<measStr<<endl;
  tassert(measStr=="1455432.60 ft");
  meas.setFoot(USSURVEY);
  measStr=meas.formatMeasurementUnit(easting,LENGTH);
  cout<<"In US survey feet, easting is "<<measStr<<endl;
  tassert(measStr=="1455429.69 ft");
  xy0=meas.parseXy("30,40 ft");
  xy1=meas.parseXy("30 ft,40 m");
  xy2=meas.parseXy("30,40 m");
  cout<<"xy0-xy1 "<<dist(xy0,xy1)<<endl;
  cout<<"xy1-xy2 "<<dist(xy1,xy2)<<endl;
  cout<<"xy2-xy0 "<<dist(xy2,xy0)<<endl;
  tassert(fabs(dist(xy0,xy1)-4*6.95)<0.1);
  tassert(fabs(dist(xy1,xy2)-3*6.95)<0.1);
  tassert(fabs(dist(xy2,xy0)-5*6.95)<0.1);
  meas.setDefaultUnit(ANGLE,0.017);
  measStr=meas.formatMeasurementUnit(longitude,ANGLE);
  cout<<"In degrees, longitude is "<<measStr<<endl;
  meas.setDefaultUnit(ANGLE,0.016);
  measStr=meas.formatMeasurementUnit(longitude,ANGLE);
  cout<<"In gons, longitude is "<<measStr<<endl;
  meas.setDefaultUnit(ANGLE,5e-6);
  measStr=meas.formatMeasurementUnit(longitude,ANGLE);
  cout<<"In degrees, minutes, and seconds, longitude is "<<measStr<<endl;
  parsed=meas.parseMeasurement("20.61408379530502663289",LENGTH);
  cout<<parsed.magnitude<<" m\n";
  tassert(fabs(parsed.magnitude-2*M_PI)<1e-8);
  parsed=meas.parseMeasurement(measStr,ANGLE);
  cout<<ldecimal(parsed.magnitude)<<" rad\n";
  tassert(fabs(parsed.magnitude-longitude)<1e-9);
  meas.setDefaultUnit(ANGLE_B,1732);
  parsed=meas.parseMeasurement("-81-55-11.3441",ANGLE_B);
  cout<<ldecimal(parsed.magnitude)<<" furmanlets\n";
  // There has to be a better name. The unit is 1/32768 furman.
  tassert(fabs(parsed.magnitude-radtobin(longitude))<1);
}

void testqindex()
{
  qindex qinx;
  int i,j,qs,ntri,size;
  triangle *ptri;
  vector<xy> plist;
  double pathlength;
  vector<qindex*> hilbertpath;
  set<triangle *> intri;
  xy offset(16,8),bone1(3,4),bone2(-3,-4),bone3(49,-64);
  PostScript ps;
  doc.makepointlist(1);
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
  ps.open("qindex.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  ps.setDoc(doc);
  ps.startpage();
  ps.setscale(1,-7,31,23);
  aster(doc,100);
  doc.pl[1].maketin();
  size=rng.usrandom();
  cout<<"Size "<<size<<'\n';
  enlarge(doc,pow(2,(size-32767.5)/65536));
  for (i=0;i<100;i++)
  {
    ps.dot(doc.pl[1].points[i+1]+offset);
    plist.push_back(doc.pl[1].points[i+1]+offset);
  }
  for (j=0;j<10;j++)
    for (i=0;i<100;i++)
      if (rng.ucrandom()<128)
	plist.push_back(doc.pl[1].points[i+1]+offset);
  qinx.sizefit(plist);
  qinx.split(plist);
  printf("%d nodes, ",qs=qinx.size());
  qs--;
  qs=qs*3/4; // convert to number of leaves of the tree (undivided squares in the drawing)
  qs++;
  printf("%d leaves\n",qs);
  tassert(qs>=52 && qs<=108);
  qinx.draw(ps);
  ps.endpage();
  ps.startpage();
  hilbertpath=qinx.traverse();
  tassert(hilbertpath.size()==qs);
  ps.setscale(1,-7,31,23);
  qinx.draw(ps);
  ps.setcolor(0,0,1);
  for (i=1,pathlength=0;i<hilbertpath.size();i++)
  {
    ps.line2p(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
    pathlength+=dist(hilbertpath[i-1]->middle(),hilbertpath[i]->middle());
  }
  printf("pathlength %f\n",pathlength);
  tassert(pathlength>100 && pathlength<400);
  ps.endpage();
  ps.startpage();
  ps.setscale(-15,-15,15,15);
  doc.pl[1].maketin();
  doc.pl[1].maketriangles();
  for (i=ntri=0;i<doc.pl[1].edges.size();i++)
  {
    ntri+=doc.pl[1].edges[i].tria!=NULL;
    ntri+=doc.pl[1].edges[i].trib!=NULL;
    ps.line(doc.pl[1].edges[i],i,false,true);
    ps.setcolor(0.6,0.4,0);
    if (doc.pl[1].edges[i].tria)
      ps.line2p(doc.pl[1].edges[i].midpoint(),doc.pl[1].edges[i].tria->centroid());
    ps.setcolor(0,0.4,0.6);
    if (doc.pl[1].edges[i].trib)
      ps.line2p(doc.pl[1].edges[i].midpoint(),doc.pl[1].edges[i].trib->centroid());
  }
  printf("%d edges ntri=%d\n",i,ntri);
  tassert(ntri>i/2);
  ps.setcolor(1,0,0);
  for (i=0;i<doc.pl[1].triangles.size();i++)
  {
    tassert(doc.pl[1].triangles[i].area()>0);
    //printf("tri %d area %f\n",i,doc.pl[1].triangles[i].area());
    ps.dot(doc.pl[1].triangles[i].centroid());
  }
  printf("%d triangles\n",i);
  tassert(ntri==i*3); // ntri is the number of sides of edges which are linked to a triangle
  ps.endpage();
  ptri=&doc.pl[1].triangles[0];
  ptri=ptri->findt(bone1);
  tassert(ptri->in(bone1));
  tassert(!ptri->in(bone2));
  ptri=ptri->findt(bone2);
  tassert(ptri->in(bone2));
  tassert(!ptri->in(bone1));
  tassert(ptri->findt(bone3,true));
  tassert(!ptri->findt(bone3,false));
  ps.startpage();
  ps.setscale(-15,-15,15,15);
  plist.clear();
  for (i=0;i<100;i++)
    plist.push_back(doc.pl[1].points[i+1]);
  for (j=0;j<10;j++)
    for (i=0;i<100;i++)
      if (rng.ucrandom()<128)
	plist.push_back(doc.pl[1].points[i+1]);
  qinx.sizefit(plist);
  qinx.split(plist);
  qinx.draw(ps);
  qinx.settri(&doc.pl[1].triangles[0]);
  for (i=ntri=0;i<doc.pl[1].edges.size();i++)
    ps.line(doc.pl[1].edges[i],i,false);
  ps.setcolor(1,0,0);
  hilbertpath=qinx.traverse();
  for (i=pathlength=0;i<hilbertpath.size();i++)
  {
    ps.line2p(hilbertpath[i]->tri->centroid(),hilbertpath[i]->middle());
    pathlength+=dist(hilbertpath[i]->tri->centroid(),hilbertpath[i]->middle());
  }
  printf("settri: pathlength=%f\n",pathlength);
  tassert(pathlength>27 && pathlength<210);
  ps.endpage();
  ptri=qinx.findt(bone1);
  tassert(ptri->in(bone1));
  tassert(!ptri->in(bone2));
  ptri=qinx.findt(bone2);
  tassert(ptri->in(bone2));
  tassert(!ptri->in(bone1));
  tassert(qinx.findt(bone3,true));
  tassert(!qinx.findt(bone3,false));
  printf("%d nodes\n",qinx.size());
  intri=qinx.localTriangles(xy(0,0),pow(2,(size-32767.5)/65536)*10,185);
  cout<<intri.size()<<" local triangles\n";
  tassert(intri.size()>40 && intri.size()<=185);
  intri=qinx.localTriangles(xy(0,0),pow(2,(size-32767.5)/65536)*10,40);
  tassert(intri.size()==1 && intri.count(nullptr));
  ps.trailer();
  ps.close();
}

void drawgrad(PostScript &ps,double scale)
{
  ptlist::iterator i;
  for (i=doc.pl[1].points.begin();i!=doc.pl[1].points.end();i++)
  {
    ps.setcolor(0,1,0);
    ps.line2p(i->second,xy(i->second)+testsurfacegrad(i->second)*scale);
    ps.setcolor(0,0,0);
    ps.dot(i->second);
    ps.line2p(i->second,xy(i->second)+i->second.gradient*scale);
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
  PostScript ps;
  doc.makepointlist(1);
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
  ps.open("gradient.ps");
  ps.prolog();
  for (corr=0;corr<=1;corr+=0.1)
  {
    ps.startpage();
    ps.setscale(-25,-25,25,25);
    doc.pl[1].makegrad(corr);
    checkgrad(avgerror,maxerror);
    printf("testmakegrad: corr=%f avgerror=%f maxerror=%f\n",corr,avgerror,maxerror);
    drawgrad(ps,3);
    ps.endpage();
  }
  ps.trailer();
  ps.close();
}

void testrasterdraw()
{
  doc.makepointlist(1);
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
  vector<double> xs;
  array<double,4> lh;
  vector<xyz> slice;
  vector<xy> crits;
  PostScript ps;
  int i,j,side,cubedir,cubedir0,ptype,size0,size1,size2;
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
  ps.open(psfname);
  ps.prolog();
  ps.startpage();
  ps.setDoc(doc);
  ps.setscale(-17,-17,17,17);
  for (j=0;j<doc.pl[1].edges.size();j++)
    ps.line(doc.pl[1].edges[j],j,false);
#ifndef FLATTRIANGLE
  //cubedir0=doc.pl[1].triangles[0].findnocubedir0();
  cubedir=doc.pl[1].triangles[0].findnocubedir();
  //if (cubedir!=cubedir0)
  //  cout<<"Cubedirs don't match "<<cubedir0<<"!="<<cubedir<<endl;
  ofile<<"Zero cube dir "<<cubedir<<' '<<bintodeg(cubedir)<<"°"<<endl;
  ofile<<"Zero quad offset "<<doc.pl[1].triangles[0].flatoffset()<<endl;
#endif
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
#ifndef FLATTRIANGLE
  ps.line2p(doc.pl[1].triangles[0].spcoord(1.5,-1.5),doc.pl[1].triangles[0].spcoord(-1.5,-1.5));
  ps.line2p(doc.pl[1].triangles[0].spcoord(-1.5,-1.5),doc.pl[1].triangles[0].spcoord(-1.5,1.5));
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
	  ps.setcolor(0,.7,0);
	else
	  ps.setcolor(1,0,1);
	ps.line2p(doc.pl[1].triangles[0].spcoord(slice[j-1].east(),slice[j-1].north()),
		doc.pl[1].triangles[0].spcoord(slice[j].east(),slice[j].north()));
      }
    }
    crits=doc.pl[1].triangles[0].criticalpts_side(side);
    for (j=0;j<crits.size();j++)
    {
      ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
      ps.dot(crits[j]);
    }
  }
  crits=doc.pl[1].triangles[0].criticalpts_axis();
#endif
  ofile<<endl;
  for (j=0;j<crits.size();j++)
  {
    ofile<<fixed<<setprecision(3)<<setw(7)<<crits[j].east()<<setw(7)<<crits[j].north()<<endl;
    ps.dot(crits[j]);
  }
  doc.pl[1].triangles[0].findcriticalpts();
#ifndef FLATTRIANGLE
  crits=doc.pl[1].triangles[0].critpoints;
#endif
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
	ps.dot(doc.pl[1].edges[j].critpoint(i));
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
    ps.spline(doc.pl[1].triangles[0].subdiv[j].approx3d(1));
  clipped=doc.pl[1].triangles[0].dirclip(xy(1,2),AT34);
  ps.setcolor(1,0,1);
  ps.spline(clipped.approx3d(1));
  ps.endpage();
  ps.close();
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
  doc.makepointlist(1);
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
  doc.makepointlist(1);
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
  doc.makepointlist(1);
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
  doc.makepointlist(1);
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
  latlong ll;
  xyz pt;
  double toler,expected;
  manysum xsqsum,ysqsum,zsqsum;
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
  for (i=0;i<746496;i++)
  {
    ll=h.onearth();
    pt=Sphere.geoc(ll,0);
    xsqsum+=sqr(pt.getx());
    ysqsum+=sqr(pt.gety());
    zsqsum+=sqr(pt.getz());
  }
  cout<<xsqsum.total()<<' '<<ysqsum.total()<<' '<<zsqsum.total()<<endl;
  expected=EARTHRADSQ*i/3;
  toler=EARTHRADSQ*log(i+3);
  tassert(fabs(xsqsum.total()-expected)<toler);
  tassert(fabs(ysqsum.total()-expected)<toler);
  tassert(fabs(zsqsum.total()-expected)<toler);
}

xy intersection(polyline &p,xy start,xy end)
/* Given start and end, of which one is in p and the other is out,
 * returns a point on p. It can't use Brent's method because p.in
 * gives no clue about distance. It uses bisection.
 */
{
  xy mid;
  double sin,min,ein;
  sin=p.in(start);
  ein=p.in(end);
  min=9;
  while (fabs(min)>=0.75 || fabs(min)<=0.25)
  {
    mid=(start+end)/2;
    min=p.in(mid);
    if (dist(start,mid)==0 || dist(mid,end)==0)
      break;
    if (fabs(sin-min)>fabs(min-ein))
    {
      end=mid;
      ein=min;
    }
    else if (fabs(sin-min)<fabs(min-ein))
    {
      start=mid;
      sin=min;
    }
    else
      break;
  }
  return mid;
}

void testpolyline()
{
  polyline p;
  polyarc q;
  polyspiral r;
  xy a(2,1.333),b(1.5,2),c(1.5000000001,2),d(1.499999999,2);
  xy e(3,0),f(3.5,0.5),g(0,4),mid;
  /* a: near centroid; b: center of circle, midpoint of hypot;
   * c and d: on opposite sites of b; e: corner;
   * f and g: other points on circle
   */
  cout<<"testpolyline"<<endl;
  r.smooth(); // sets the curvy flag
  bendlimit=DEG180+7;
  p.insert(xy(0,0));
  q.insert(xy(0,0));
  r.insert(xy(0,0));
  p.insert(xy(3,0));
  q.insert(xy(3,0));
  r.insert(xy(3,0));
  p.setlengths();
  q.setlengths();
  r.setlengths();
  cout<<p.length()<<' '<<r.length()<<endl;
  tassert(p.length()==6);
  tassert(fabs(r.length()-3*M_PI)<1e-6);
  p.insert(xy(3,4));
  q.insert(xy(3,4));
  r.insert(xy(3,4));
  p.setlengths();
  cout<<p.length()<<endl;
  tassert(p.length()==12);
  q.setlengths();
  tassert(q.length()==12);
  tassert(q.area()==6);
  r.setlengths();
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
  cout<<"testpolyline: area of circle is "<<q.area()<<" (arc), "<<r.area()<<" (spiral)"<<endl;
  tassert(fabs(q.length()-M_PI*5)<0.0005);
  tassert(fabs(q.area()-M_PI*6.25)<0.0005);
  tassert(fabs(r.length()-M_PI*5)<0.0005);
  tassert(fabs(r.area()-M_PI*6.25)<0.0005);
  cout<<q.getarc(0).center().north()<<endl;
  cout<<q.length()<<endl;
  cout<<"p: a "<<p.in(a)<<" b "<<p.in(b)<<" c "<<p.in(c)<<" d "<<p.in(d)
    <<" e "<<p.in(e)<<" f "<<p.in(f)<<" g "<<p.in(g)<<endl;
  tassert(p.in(a)==1);
  tassert(p.in(b)==0.5);
  tassert(p.in(c)==1);
  tassert(p.in(d)==0);
  tassert(p.in(e)==0.25);
  tassert(p.in(f)==0);
  tassert(p.in(g)==0);
  cout<<"q: a "<<q.in(a)<<" b "<<q.in(b)<<" c "<<q.in(c)<<" d "<<q.in(d)
    <<" e "<<q.in(e)<<" f "<<q.in(f)<<" g "<<q.in(g)<<endl;
  tassert(q.in(a)==1);
  tassert(q.in(b)==1);
  tassert(q.in(c)==1);
  tassert(q.in(d)==1);
  tassert(q.in(e)==0.5);
  mid=intersection(q,b,2*f-b);
  cout<<"q x b-f ("<<ldecimal(mid.getx())<<','<<ldecimal(mid.gety())<<')'<<endl;
  tassert(dist(f,mid)<1e-8);
  mid=intersection(q,b,2*g-b);
  cout<<"q x b-g ("<<ldecimal(mid.getx())<<','<<ldecimal(mid.gety())<<')'<<endl;
  tassert(dist(g,mid)<1e-8);
  cout<<"r: a "<<r.in(a)<<" b "<<r.in(b)<<" c "<<r.in(c)<<" d "<<r.in(d)
    <<" e "<<r.in(e)<<" f "<<r.in(f)<<" g "<<r.in(g)<<endl;
  tassert(r.in(a)==1);
  tassert(r.in(b)==1);
  tassert(r.in(c)==1);
  tassert(r.in(d)==1);
  tassert(r.in(e)==0.5);
  mid=intersection(r,b,2*f-b);
  cout<<"r x b-f ("<<ldecimal(mid.getx())<<','<<ldecimal(mid.gety())<<')'<<endl;
  tassert(dist(f,mid)<1e-8);
  mid=intersection(r,b,2*g-b);
  cout<<"r x b-g ("<<ldecimal(mid.getx())<<','<<ldecimal(mid.gety())<<')'<<endl;
  tassert(dist(g,mid)<1e-8);
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
  PostScript ps;
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
  ps.open("bezier3d.ps");
  ps.prolog();
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
    {
      ps.startpage();
      ps.setscale(-0.2,-0.5,0.2,0.5,degtobin(90));
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
      ps.setcolor(0,0,0);
      sprintf(buf,"cur=%5.3f clo=%5.3f",curvature,clothance);
      ps.write(xy(0,0.2),buf);
      ps.setcolor(1,.5,0); // red is the spiral
      for (i=-10;i<10;i++)
        ps.line2p(spipts[i+10],spipts[i+11]);
      ps.setcolor(0,.5,1); // blue is the Bézier I'm approximating it with
      for (i=-10;i<10;i++)
        ps.line2p(bezpts[i+10],bezpts[i+11]);
      ps.setcolor(0,0,0);
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
	ps.line2p(lastpt,thispt);
      }
      avgdist=sqrt(totaldist/numdist);
      dists[clothance*M_PI+curvature]=avgdist;
      ests[clothance*M_PI+curvature]=bez3destimate(startpoint,startbearing,1,endbearing,endpoint);
      ps.setcolor(0,0,0);
      sprintf(buf,"dist=%5.7f",avgdist);
      ps.write(xy(0,-0.2),buf);
      ps.endpage();
      //cout<<avgdist<<endl;
    }
  ps.startpage();
  ps.setscale(-1,-1,1,1,0);
  ps.setcolor(0.5,0.5,1);
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
      ps.circle(xy(curvature,clothance/6),sqrt(ests[clothance*M_PI+curvature]));
  ps.setcolor(0,0,0);
  for (curvature=-1;curvature<1.1;curvature+=0.125)
    for (clothance=-6;clothance<6.1;clothance+=(clothance>-0.76 && clothance<0.74)?0.25:0.75)
    {
      ps.circle(xy(curvature,clothance/6),sqrt(dists[clothance*M_PI+curvature]));
      if (dists[clothance*M_PI+curvature]>ests[clothance*M_PI+curvature])
	cout<<"estimate too small crv="<<curvature<<" clo="<<clothance<<endl;
      tassert(dists[clothance*M_PI+curvature]<=ests[clothance*M_PI+curvature]);
    }
  ps.endpage();
  ps.startpage();
  ps.setscale(-100,-180,100,180,degtobin(0));
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
    ps.spline(c);
  }
  ps.endpage();
  for (i=-300,ngood=0;i<330;i+=60)
  {
    ps.startpage();
    ps.setscale(-100,-180,100,180,degtobin(0));
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
	ps.spline(c);
	ngood++;
      }
    }
    ps.endpage();
  }
  cout<<ngood<<" good spirals"<<endl;
  cout<<nclose<<" with 1/3 point close"<<endl;
  tassert(ngood>=107);
  tassert(nclose>=30);
  ps.startpage();
  ps.setscale(-30,-70,30,70,DEG90);
  for (i=-18,nclose=0;i<19;i+=2)
  {
    switch (i%3)
    {
      case 0:
	ps.setcolor(1,0,0);
	break;
      case 1:
      case -2:
	ps.setcolor(0,0.5,0);
	break;
      case -1:
      case 2:
	ps.setcolor(0,0,1);
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
    ps.spline(c);
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(-30,-70,30,70,DEG90);
  for (i=-100,nclose=0;i<101;i+=5)
  {
    switch (i%3)
    {
      case 0:
	ps.setcolor(1,0,0);
	break;
      case 1:
      case -2:
	ps.setcolor(0,0.5,0);
	break;
      case -1:
      case 2:
	ps.setcolor(0,0,1);
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
      ps.spline(c);
  }
  ps.setcolor(0,0,0);
  ps.endpage();
  ps.trailer();
  ps.close();
}

void testangleconvcorner(string anglestr,xyz &totxyz)
{
  xyz corner;
  latlong ll;
  ll=parselatlong(anglestr,DEGREE);
  tassert(partialLatlong(anglestr));
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
  strang=bintoangle(atan2i(2,1),ARCSECOND+DEC2+FIXLARGER);
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
  tassert(ll.valid()==2);
  ll=parselatlong("12z6°W 5*5°N",DEGREE);
  tassert(ll.valid()==0);
  wrangell0=parselatlong("143°52'11.5\"W 61°56'51\"N",DEGREE);
  cout<<formatlatlong(wrangell0,DEGREE+DEC5)<<endl;
  wrangell1=parselatlong("143.86986°W 61.9475°N",DEGREE);
  cout<<formatlatlong(wrangell1,ARCSECOND+DEC2+FIXLARGER)<<endl;
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
  testangleconvcorner("35°15′52″N45°W",totxyz);
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

void testpnezd()
{
  double a;
  ifstream file;
  string content;
  criterion crit1;
  crit1.istopo=true;
  doc.pl[0].crit.clear();
  doc.pl[0].crit.push_back(crit1);
  doc.pl[0].clear();
  doc.pl[1].clear();
  aster(doc,3);
  doc.copytopopoints(0,1);
  doc.ms.clearUnits();
  doc.ms.addUnit(FOOT);
  //set_length_unit(FOOT+DEC2);
  doc.writepnezd("pnezd.csv");
  file.open("pnezd.csv",ios::in);
  getline(file,content);
  tassert(content.find("2.319")==4);
  cout<<content<<endl;
  getline(file,content);
  tassert(content.find("2.714")==3);
  cout<<content<<endl;
  getline(file,content);
  tassert(content.find(".4535")<40);
  cout<<content<<endl;
  file.close();
  doc.pl[0].clear();
  doc.readpnezd("pnezd.csv");
  a=area3(doc.pl[0].points[1],doc.pl[0].points[2],doc.pl[0].points[3]);
  tassert(fabs(a+1.5034)<1e-3);
  cout<<a<<endl;
  doc.ms.clearUnits();
  doc.ms.addUnit(METER);
  //set_length_unit(METER+DEC3);
  doc.writepnezd("pnezd.csv");
  file.open("pnezd.csv",ios::in);
  getline(file,content);
  tassert(content.find(".7071")==4);
  cout<<content<<endl;
  getline(file,content);
  tassert(content.find(".8273")==3);
  cout<<content<<endl;
  getline(file,content);
  tassert(content.find(".1382")<40);
  cout<<content<<endl;
  file.close();
  doc.pl[0].clear();
  doc.readpnezd("pnezd.csv");
  a=area3(doc.pl[0].points[1],doc.pl[0].points[2],doc.pl[0].points[3]);
  tassert(fabs(a+1.5034)<1e-3);
  cout<<a<<endl;
}

void testldecimal()
{
  double d;
  bool looptests=false;
  cout<<ldecimal(1/3.)<<endl;
  cout<<ldecimal(M_PI)<<endl;
  cout<<ldecimal(-64664./65536,1./131072)<<endl;
  /* This is a number from the Alaska geoid file, -0.9867, which was output
   * as -.98669 when converting to GSF. The bug is fixed. It still outputs
   * a few numbers with an extra digit, but the input numbers are halfway
   * between multiples of 1/65536, so that is not a bug.
   */
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
    for (d=DBL_EPSILON;d<=1;d*=2)
      cout<<ldecimal(M_SQRT_3,d)<<endl;
  }
  cout<<ldecimal(0)<<' '<<ldecimal(INFINITY)<<' '<<ldecimal(NAN)<<' '<<ldecimal(-5.67)<<endl;
  cout<<ldecimal(3628800)<<' '<<ldecimal(1296000)<<' '<<ldecimal(0.000016387064)<<endl;
  tassert(ldecimal(0)=="0");
  tassert(ldecimal(1)=="1");
  tassert(ldecimal(-1)=="-1");
  tassert(isalpha(ldecimal(INFINITY)[0]));
  tassert(isalpha(ldecimal(NAN)[0]));
  tassert(ldecimal(1.7320508)=="1.7320508");
  tassert(ldecimal(1.7320508,0.0005)=="1.732");
  tassert(ldecimal(-0.00064516)=="-.00064516");
  tassert(ldecimal(3628800)=="3628800");
  tassert(ldecimal(1296000)=="1296e3");
  tassert(ldecimal(0.000016387064)=="1.6387064e-5");
  tassert(ldecimal(-64664./65536,1./131072)=="-.9867");
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

void testellipsoidscale(ellipsoid *ellip)
/* Tests the accuracy of an ellipsoid's conformal latitude projection's scale.
 * See testprojscale below.
 */
{
  array<latlong,2> pointpair,projectedpair;
  latlong midpoint,midpointSphere;
  array<xyz,2> xyzpairEllip,xyzpairSphere;
  int i,nbad;
  double scale;
  for (i=nbad=0;i<16777216 && nbad>=trunc(sqrt(i)/16);i++)
  {
    pointpair=randomPointPair();
    midpoint.lat=(pointpair[0].lat+pointpair[1].lat)/2;
    midpoint.lon=(pointpair[0].lon+pointpair[1].lon)/2;
    xyzpairEllip[0]=ellip->geoc(pointpair[0],0);
    xyzpairEllip[1]=ellip->geoc(pointpair[1],0);
    projectedpair[0]=ellip->conformalLatitude(pointpair[0]);
    projectedpair[1]=ellip->conformalLatitude(pointpair[1]);
    midpointSphere=ellip->conformalLatitude(midpoint);
    scale=ellip->scaleFactor(midpoint.lat,midpointSphere.lat);
    xyzpairSphere[0]=ellip->sphere->geoc(projectedpair[0],0);
    xyzpairSphere[1]=ellip->sphere->geoc(projectedpair[1],0);
    if (fabs(dist(xyzpairEllip[0],xyzpairEllip[1])/scale/dist(xyzpairSphere[0],xyzpairSphere[1])-1)>1e-6)
    {
      nbad++;
      if (nbad<256)
	cout<<radtodeg(midpoint.lat)<<"° "<<radtodeg(midpoint.lon)<<"° computed scale "<<
	scale<<" actual scale "<<dist(xyzpairEllip[0],xyzpairEllip[1])/dist(xyzpairSphere[0],xyzpairSphere[1])<<endl;
      }
  }
  cout<<ellip->getName()<<" scale is ";
  if (nbad>=trunc(sqrt(i)/16))
    cout<<"bad"<<endl;
  else
    cout<<"good"<<endl;
  tassert(nbad<trunc(sqrt(i)/16));
}

void testkrugerscale(ellipsoid *ellip)
{
  halton hal;
  xy pnt[2],krugerPnt[2],dekrugerPnt[2],krugerDeriv,dekrugerDeriv;
  int i,j;
  bool good;
  for (i=0;i<1024;i++)
  {
    pnt[0]=pnt[1]=(hal.pnt()-xy(1,1))*1e6;
    krugerDeriv=ellip->krugerizeDeriv(pnt[0]);
    dekrugerDeriv=ellip->dekrugerizeDeriv(pnt[0]);
    pnt[0]+=cossin(i*2097152)/2;
    pnt[1]-=cossin(i*2097152)/2;
    for (j=0;j<2;j++)
    {
      krugerPnt[j]=ellip->krugerize(pnt[j]);
      dekrugerPnt[j]=ellip->dekrugerize(pnt[j]);
    }
    good=dist(krugerPnt[0]-krugerPnt[1],turn(krugerDeriv,i*2097152))<1e-6;
    good&=dist(dekrugerPnt[0]-dekrugerPnt[1],turn(dekrugerDeriv,i*2097152))<1e-6;
    if (!good)
    {
      cout<<"Krüger distance is "<<dist(krugerPnt[0],krugerPnt[1])<<" should be "<<krugerDeriv.length()<<endl;
      cout<<"Dekrüger distance is "<<dist(dekrugerPnt[0],dekrugerPnt[1])<<" should be "<<dekrugerDeriv.length()<<endl;
    }
  }
}

void testellipsoid()
{
  double rad,cenlat,conlat,invconlat,conscale;
  int i,j;
  xyz sealevel,kmhigh,noffset,soffset,diff,benin,bengal,howland,galapagos,npole,spole;
  latlongelev greenhill,greenhill2,athens,athens2;
  xyz gh,ath;
  xy pnt0,pnt1,pnt2;
  ellipsoid test1(8026957,0,0.5,xyz(0,0,0),"test1"),
            test2(8026957,4013478.5,0,xyz(0,0,0),"test2");
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
  for (i=0;i<=0;i++)
  {
    diff=test1.geoc(degtorad(i),0.,0.);
    cenlat=radtodeg(atan2(diff.getz(),diff.getx()));
    cout<<setw(2)<<i<<setw(15)<<cenlat<<setw(15)<<radtodeg(test1.conformalLatitude(degtorad(i)))
        <<setw(15)<<test1.apxConLatDeriv(degtorad(i))<<endl;
  }
  cout<<"test1 eccentricity "<<test1.eccentricity()<<endl;
  for (i=0;i<=90;i++)
  {
    conlat=test1.conformalLatitude(degtorad(i));
    invconlat=test1.inverseConformalLatitude(conlat);
    conscale=test1.scaleFactor(invconlat,conlat);
    cout<<setw(2)<<i<<setw(15)<<radtodeg(conlat)<<setw(15)<<radtodeg(invconlat)
        <<setw(20)<<ldecimal(conscale)<<endl;
    tassert(fabs(invconlat-degtorad(i))<1e-10);
  }
  for (i=DEG90-1024;i<=DEG90;i+=32)
  {
    conlat=test1.conformalLatitude(bintorad(i));
    invconlat=test1.inverseConformalLatitude(conlat);
    conscale=test1.scaleFactor(invconlat,conlat);
    cout<<setw(2)<<i<<setw(15)<<radtobin(conlat)<<setw(15)<<radtobin(invconlat)
        <<setw(20)<<ldecimal(conscale)<<endl;
  }
  testellipsoidscale(&test1);
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
  greenhill.lat=degtorad(35.4);
  greenhill.lon=degtorad(-82.05);
  greenhill.elev=310; // over ellipsoid; geoid is about 30 m below ellipsoid
  athens.lat=degtorad(37.984);
  athens.lon=degtorad(23.728);
  athens.elev=200;
  gh=GRS80.geoc(greenhill);
  greenhill2=GRS80.geod(gh);
  cout<<"Green Hill "<<radtodeg(greenhill2.lat)<<' '<<radtodeg(greenhill2.lon)<<' '<<greenhill2.elev<<endl;
  tassert(fabs(greenhill2.lat-greenhill.lat)<1e-3/EARTHRAD);
  tassert(fabs(greenhill2.lon-greenhill.lon)<1e-3/EARTHRAD);
  tassert(fabs(greenhill2.elev-greenhill.elev)<1e-3);
  greenhill2=GRS80.geod(gh/1000);
  ath=HGRS87.geoc(athens);
  athens2=HGRS87.geod(ath);
  cout<<"Athens "<<radtodeg(athens2.lat)<<' '<<radtodeg(athens2.lon)<<' '<<athens2.elev<<endl;
  tassert(fabs(athens2.lat-athens.lat)<1e-3/EARTHRAD);
  tassert(fabs(athens2.lon-athens.lon)<1e-3/EARTHRAD);
  tassert(fabs(athens2.elev-athens.elev)<1e-3);
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
  pnt0=xy(6e6,5e6);
  pnt1=WGS84.krugerize(pnt0);
  pnt2=WGS84.dekrugerize(pnt1);
  cout<<"Krügerize "<<pnt0.east()<<','<<pnt0.north()
      <<"->"<<pnt1.east()<<','<<pnt1.north()
      <<"->"<<pnt2.east()<<','<<pnt2.north()
      <<" dist "<<dist(pnt0,pnt2)<<endl;
  testkrugerscale(&WGS84);
}

float BeninBoundary[][2]=
{
  {0.898,10.294},{1.355,10.},{1.349,9.566},
  {1.394,9.481},{1.41,9.329},{1.617,9.068},
  {1.631,7.849},{1.642,6.995},{1.586,6.998},
  {1.624,6.596},{1.807,6.284},{1.633,6.236},
  {2.166,6.333},{2.673,6.371},{2.767,6.758},
  {2.729,7.792},{2.786,9.045},{3.052,9.087},
  {3.272,9.659},{3.548,9.879},{3.632,10.417},
  {3.802,10.455},{3.767,10.953},{3.544,11.252},
  {3.592,11.675},{2.836,12.407},{2.389,12.212},
  {2.459,11.977},{2.307,11.697},{2.025,11.434},
  {1.422,11.456},{0.97,11.075},{0.778,10.442}
};

void drawproj(string projName,Projection &proj)
{
  int i,ori=0;
  double minx,maxx,miny,maxy;
  latlong ll;
  PostScript ps;
  polyline outline;
  for (i=0;i<sizeof(BeninBoundary)/sizeof(BeninBoundary[0]);i++)
  {
    ll=latlong(degtorad(BeninBoundary[i][1]),degtorad(BeninBoundary[i][0]));
    outline.insert(proj.latlongToGrid(ll));
  }
  minx=outline.dirbound(-ori);
  miny=outline.dirbound(-ori+DEG90);
  maxx=-outline.dirbound(-ori+DEG180);
  maxy=-outline.dirbound(-ori-DEG90);
  ps.open(projName+".ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  ps.startpage();
  ps.setscale(minx,miny,maxx,maxy,ori);
  ps.spline(outline.approx3d(1));
  ps.endpage();
  ps.trailer();
  ps.close();
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

void testEquidistance(string projName,Projection &proj,latlong begin,latlong end)
/* Draws a clélie from begin to end and checks that the scale of the projection
 * is constant along the clélie.
 */
{
  vector<latlong> globePoints=splitPoints(begin,end);
  int i;
  vector<xy> mapPoints;
  vector<xyz> spacePoints;
  vector<double> scales;
  double minScale=INFINITY,maxScale=0,scale;
  globePoints.push_back(end);
  for (i=0;i<globePoints.size();i++)
  {
    spacePoints.push_back(proj.ellip->geoc(globePoints[i],0));
    mapPoints.push_back(proj.latlongToGrid(globePoints[i]));
  }
  for (i=0;i<mapPoints.size()-1;i++)
  {
    scale=dist(mapPoints[i],mapPoints[i+1])/dist(spacePoints[i],spacePoints[i+1]);
    if (scale>maxScale)
      maxScale=scale;
    if (scale<minScale)
      minScale=scale;
    scales.push_back(scale);
  }
  scale=sqrt(minScale*maxScale);
  cout<<projName<<" scale varies by ±"<<maxScale/scale-1<<endl;
  tassert(maxScale/minScale<1.000001);
}

int badLimit(int i)
{
  return trunc(sqrt(i)/16)+i/5; // 1/5 is about how much of the earth transverse Mercator fails on
}

void testprojscale(string projName,Projection &proj)
/* Tests the accuracy of a projection's scale. Implicitly tests conformality.
 * Picks random pairs of points 1 meter apart and checks that the distance
 * between the two points on the map is close to the map scale. Allows up to 1/5
 * bad pairs, since transverse Mercator loses accuracy far from its meridian.
 */
{
  array<latlong,2> pointpair;
  latlong midpoint,inv;
  array<xyz,2> xyzpair;
  array<xy,2> xypair;
  xyz midxyz,invxyz;
  xy midxy;
  int i,nbadscale,nbadinv;
  double scale,backScale;
  for (i=nbadscale=nbadinv=0;
       i<16777216 && (nbadscale>=badLimit(i) || nbadinv>=badLimit(i));i++)
  {
    pointpair=randomPointPair();
    midpoint.lat=(pointpair[0].lat+pointpair[1].lat)/2;
    midpoint.lon=(pointpair[0].lon+pointpair[1].lon)/2;
    xyzpair[0]=proj.ellip->geoc(pointpair[0],0);
    xyzpair[1]=proj.ellip->geoc(pointpair[1],0);
    midxyz=proj.ellip->geoc(midpoint,0);
    inv=proj.gridToLatlong(proj.latlongToGrid(midpoint));
    invxyz=proj.ellip->geoc(inv,0);
    scale=proj.scaleFactor(midpoint);
    midxy=proj.latlongToGrid(midpoint);
    backScale=proj.scaleFactor(midxy);
    /* For other projections, one of the scaleFactor methods calls the other,
     * but for ellipsoidal transverse Mercator, they are computed separately.
     */
    xypair[0]=proj.latlongToGrid(pointpair[0]);
    xypair[1]=proj.latlongToGrid(pointpair[1]);
    if (fabs(log(scale/backScale))>1e-6)
    {
      cout<<"scale "<<scale<<" backScale "<<backScale<<endl;
      scale=proj.scaleFactor(midpoint);
      backScale=proj.scaleFactor(midxy);
    }
    if (fabs(dist(xypair[0],xypair[1])/scale/dist(xyzpair[0],xyzpair[1])-1)>1e-6
        || fabs(log(scale/backScale))>1e-6)
    {
      nbadscale++;
      if (nbadscale<256)
	cout<<radtodeg(midpoint.lat)<<"° "<<radtodeg(midpoint.lon)<<"° computed scale "<<
	scale<<" actual scale "<<dist(xypair[0],xypair[1])/dist(xyzpair[0],xyzpair[1])<<endl;
    }
    if (dist(midxyz,invxyz)>1e-3)
    {
      nbadinv++;
      if (nbadinv<256)
	cout<<radtodeg(midpoint.lat)<<"° "<<radtodeg(midpoint.lon)<<"° inverse projection "<<
	radtodeg(inv.lat)<<"° "<<radtodeg(inv.lon)<<"°\n";
    }
  }
  cout<<projName<<" scale is ";
  if (nbadscale>=badLimit(i))
    cout<<"bad "<<ldecimal((double)nbadscale/i,0.001)<<endl;
  else
    cout<<"good"<<endl;
  tassert(nbadscale<badLimit(i));
  cout<<projName<<" inverse is ";
  if (nbadinv>=badLimit(i))
    cout<<"bad "<<ldecimal((double)nbadinv/i,0.001)<<endl;
  else
    cout<<"good"<<endl;
  tassert(nbadinv<badLimit(i));
}

void testTransMerc()
{
  int i,angle;
  xyz pt3,pt3n,pt3e;
  xy pt2,pt2n,pt2e;
  double inverr,scaleerr;
  angle=rng.uirandom();
  Quaternion ro=versor(xyz(0,1,0),angle);
  cout<<"TransMerc rotating by "<<angle<<endl;
  for (i=-89;i<90;i++)
  {
    pt3=ro.rotate(xyz(cos(degtorad(i)),sin(degtorad(i)),0));
    pt2=transMerc(pt3);
    inverr=dist(pt3,invTransMerc(pt2,1));
    scaleerr=log(transMercScale(pt2,1)/transMercScale(pt3));
    tassert(fabs(inverr)<1e-12 && fabs(scaleerr)<1e-12);
    if (fabs(inverr)>1e-12 || fabs(scaleerr)>1e-12)
      cout<<i<<"° inverse error "<<inverr<<" scale error "<<scaleerr<<endl;
  }
}

/* 80° 1.9126888
 * 60° 1.56419578
 * 30° 1.13975353
 */
void testprojection()
{
  latlong zll(0,0);
  xy zxy(0,0);
  LambertConicSphere sphereMercator,sphereConic10(0,degtorad(10)),
    sphereConic20(0,degtorad(20)),sphereConic80(0,degtorad(80)),
    sphereConicm80(0,degtorad(-80));
  LambertConicSphere sphereConicBenin(degtorad(8/3.),degtorad(7.5),degtorad(11.5));
  LambertConicSphere sphereConic4590(0,degtorad(45),degtorad(90));
  LambertConicSphere sphereConic4590m(0,degtorad(45),bintorad(DEG90-1));
  // One parallel 90° and the other not is an invalid condition.
  // This is a test to see what the object does with it.
  TransverseMercatorSphere sphereTransverse0,sphereTransverse60W(degtorad(-60),0.9999),
    sphereTransverse90W(degtorad(-90),0.9999);
  LambertConicEllipsoid ellipsoidMercator(&WGS84,0,0),ellipsoidConic10(&WGS84,0,degtorad(10)),
    ellipsoidConic20(&WGS84,0,degtorad(20)),ellipsoidConic80(&WGS84,0,degtorad(80)),
    ellipsoidConicm80(&WGS84,0,degtorad(-80));
  LambertConicEllipsoid ellipsoidConicBenin(&WGS84,degtorad(8/3.),degtorad(7.5),degtorad(11.5),zll,zxy);
  TransverseMercatorEllipsoid ellipsoidTransverse0(&WGS84,0),ellipsoidTransverse90W(&WGS84,degtorad(-90),0.9999);
  StereographicSphere sphereStereoNorthPole;
  latlong ncll(degtorad(33.75),degtorad(-79.));
  xy ncxy(609601.219202438405,0);
  LambertConicEllipsoid NorthCarolina(&GRS80,degtorad(-79),degtorad(103/3.),degtorad(217/6.),ncll,ncxy);
  latlong llOakland(degtorad(35.3415108),degtorad(-81.9198178028));
  xy xyOakland(344240.332,180449.168);
  // Benchmark on Oakland Road overpass over 74A, Spindale, North Carolina.
  double distOldNewOakland;
  latlong gawll(degtorad(30),degtorad(-505/6.));
  xy gawxy(7e5,0);
  TransverseMercatorEllipsoid GeorgiaWest(&GRS80,gawll.lon,0.9999,gawll,gawxy);
  latlong llBV067202(degtorad(33.936322861),degtorad(-84.158358964));
  xy xyBV067202(700768.001,436441.243);
  latlong EWN(degtorad(35.07),degtorad(-77.04));
  // New Bern is far enough from the borders that it should not be in an adjacent state's grid.
  latlong ll196(degtorad(-14.1758035159),degtorad(-120.343248884));
  // See projection.cpp. This point shouldn't be in any state's or country's grid.
  // It will be in a UTM zone, once transverse Mercator is implemented.
  latlong ll;
  xy grid;
  ProjectionList plist,ncplist,pacplist;
  ifstream pfile(string(SHARE_DIR)+"/projections.txt");
  cout<<"projection"<<endl;
  testTransMerc();
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
  testEquidistance("ellipsoidConic20",ellipsoidConic20,latlong(degtobin(20),DEG60),latlong(degtobin(20),-DEG60));
  testEquidistance("ellipsoidTransverse0",ellipsoidTransverse0,latlong(-DEG30,0),latlong(DEG90,0));
  testprojscale("sphereMercator",sphereMercator);
  drawproj("sphereMercator",sphereMercator);
  testprojscale("sphereConic10",sphereConic10);
  drawproj("sphereConic10",sphereConic10);
  testprojscale("sphereConic20",sphereConic20);
  drawproj("sphereConic20",sphereConic20);
  testprojscale("sphereConic80",sphereConic80);
  drawproj("sphereConic80",sphereConic80);
  testprojscale("sphereConic-80",sphereConicm80);
  drawproj("sphereConic-80",sphereConicm80);
  testprojscale("sphereConicBenin",sphereConicBenin);
  drawproj("sphereConicBenin",sphereConicBenin);
  testprojscale("sphereStereoNorthPole",sphereStereoNorthPole);
  drawproj("sphereStereoNorthPole",sphereStereoNorthPole);
  testprojscale("sphereTransverse0",sphereTransverse0);
  drawproj("sphereTransverse0",sphereTransverse0);
  testprojscale("sphereTransverse60W",sphereTransverse60W);
  drawproj("sphereTransverse60W",sphereTransverse60W);
  testprojscale("sphereTransverse90W",sphereTransverse90W);
  drawproj("sphereTransverse90W",sphereTransverse90W);
  testprojscale("ellipsoidMercator",ellipsoidMercator);
  drawproj("ellipsoidMercator",ellipsoidMercator);
  testprojscale("ellipsoidConic10",ellipsoidConic10);
  drawproj("ellipsoidConic10",ellipsoidConic10);
  testprojscale("ellipsoidConic20",ellipsoidConic20);
  drawproj("ellipsoidConic20",ellipsoidConic20);
  testprojscale("ellipsoidConic80",ellipsoidConic80);
  drawproj("ellipsoidConic80",ellipsoidConic80);
  testprojscale("ellipsoidConic-80",ellipsoidConicm80);
  drawproj("ellipsoidConic-80",ellipsoidConicm80);
  testprojscale("ellipsoidConicBenin",ellipsoidConicBenin);
  drawproj("ellipsoidConicBenin",ellipsoidConicBenin);
  testprojscale("ellipsoidTransverse0",ellipsoidTransverse0);
  drawproj("ellipsoidTransverse0",ellipsoidTransverse0);
  testprojscale("ellipsoidTransverse90W",ellipsoidTransverse90W);
  drawproj("ellipsoidTransverse90W",ellipsoidTransverse90W);
  testprojscale("GeorgiaWest",GeorgiaWest);
  drawproj("GeorgiaWest",GeorgiaWest);
  testprojscale("sphereStereoArabianSea",sphereStereoArabianSea);
  drawproj("sphereStereoArabianSea",sphereStereoArabianSea);
  grid=NorthCarolina.latlongToGrid(latlong(degtorad(33.75),degtorad(-79.)));
  cout<<grid.east()<<' '<<grid.north()<<endl;
  grid=NorthCarolina.latlongToGrid(latlong(degtorad(34.75),degtorad(-79.)));
  cout<<grid.east()<<' '<<grid.north()<<endl;
  grid=NorthCarolina.latlongToGrid(llOakland);
  cout<<grid.east()<<' '<<grid.north()<<' '<<dist(grid,xyOakland)<<endl;
  tassert(dist(grid,xyOakland)<0.001);
  grid=GeorgiaWest.latlongToGrid(llBV067202);
  cout<<grid.east()<<' '<<grid.north()<<' '<<dist(grid,xyBV067202)<<endl;
  tassert(dist(grid,xyBV067202)<0.001);
  if (pfile)
  {
    plist.readFile(pfile);
    ncplist=plist.cover(EWN);
    pacplist=plist.cover(ll196);
    cout<<"New Bern is in "<<ncplist.size()<<" projections\n";
    tassert(ncplist.size()==2);
    cout<<"Point 196 is in "<<pacplist.size()<<" projections\n";
    tassert(pacplist.size()==0);
    if (ncplist.size()>=2)
    {
      distOldNewOakland=dist(ncplist[0]->latlongToGrid(llOakland),ncplist[1]->latlongToGrid(llOakland));
      cout<<"Distance from Oakland NAD27 to NAD83 is "<<distOldNewOakland<<endl;
      tassert(fabs(distOldNewOakland-7.868)<0.001);
    }
  }
  else
    cout<<"Projection list is uninstalled. Skipping projection list test.\n";
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

void testlayer()
{
  LayerList layers;
  tassert(layers.findLayer("0")<0);
  tassert(layers.newLayer()==0);
  tassert(layers.findLayer("0")==0);
  tassert(layers.newLayer("drawing")==1);
  tassert(layers.newLayer("easement")==2);
  tassert(layers.newLayer("text")==3);
  tassert(layers.setColor(2,RED));
  tassert(!layers.setColor(256,MAGENTA));
}

void test1contour(string contourName,xyz offset,xy tripoint,double conterval,double expectedLength)
{
  int i,j,excessElevCount=0;
  unsigned tothash=0;
  double prec=0.0000001,along,elevError,maxElevError=0;
  histogram h(-conterval/10,conterval/10);
  manysum totalContourLength;
  triangle *tri;
  segment seg;
  xy crit,sta;
  ofstream ofile(contourName+".bez");
  PostScript ps;
  ps.open(contourName+".ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  ps.startpage();
  ps.setscale(-10-offset.getx(),-10-offset.gety(),10-offset.getx(),10-offset.gety(),0);
  doc.pl[1].maketin();
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  checkedgediscrepancies(doc.pl[1]);
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  tri=doc.pl[1].qinx.findt(tripoint-xy(offset)); // the triangle where the spike occurs
#ifndef FLATTRIANGLE
  for (i=0;tri && i<tri->critpoints.size();i++)
  {
    crit=tri->critpoints[i];
    cout<<"crit "<<i<<' '<<ldecimal(crit.getx(),prec)<<','<<ldecimal(crit.gety(),prec)<<'\n';
  }
#endif
for (i=0;tri && i<tri->subdiv.size();i++)
  {
    seg=tri->subdiv[i];
    cout<<"seg "<<i<<' '<<ldecimal(seg.getstart().getx(),prec)<<','<<ldecimal(seg.getstart().gety(),prec);
    cout<<"->"<<ldecimal(seg.getend().getx(),prec)<<','<<ldecimal(seg.getend().gety(),prec)<<'\n';
    cout<<ldecimal(seg.getstart().getz(),prec)<<' '<<ldecimal(seg.startslope(),prec)
      <<"  "<<ldecimal(seg.endslope(),prec)<<' '<<ldecimal(seg.getend().getz(),prec)<<'\n';
  }
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  rasterdraw(doc.pl[1],-offset,30,30,30,0,10*conterval,contourName+".ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  roughcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
    tothash+=doc.pl[1].contours[i].hash();
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(-10-offset.getx(),-10-offset.gety(),10-offset.getx(),10-offset.gety(),0);
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  rasterdraw(doc.pl[1],-offset,30,30,30,0,10*conterval,contourName+".ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  //psclose();
  smoothcontours(doc.pl[1],conterval,true,false);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    if (std::isnan(doc.pl[1].contours[i].length()))
      cout<<"nan\n";
    tassert(std::isfinite(doc.pl[1].contours[i].length()));
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<' ';
    //cout<<"Contour area: "<<doc.pl[1].contours[i].area()<<endl;
    totalContourLength+=doc.pl[1].contours[i].length();
    ps.spline(doc.pl[1].contours[i].approx3d(1));
    tothash+=doc.pl[1].contours[i].hash();
  }
  ps.endpage();
  ps.setpaper(papersizes["A4 portrait"],1);
  ps.startpage();
  along=rng.expirandom();
  /* Generate random points along contours with a Poisson process and make
   * a histogram of the elevation errors.
   */
  for (i=0;i<doc.pl[1].contours.size();i++)
    if (std::isfinite(doc.pl[1].contours[i].length()))
    {
      while (along<=doc.pl[1].contours[i].length())
      {
        sta=doc.pl[1].contours[i].station(along);
        elevError=doc.pl[1].elevation(sta)-doc.pl[1].contours[i].getElevation();
        h<<elevError;
        excessElevCount+=fabs(elevError)>conterval/10;
        along+=rng.expcrandom();
        if (fabs(elevError)>maxElevError)
          maxElevError=fabs(elevError);
      }
      along-=doc.pl[1].contours[i].length();
    }
  h.plot(ps,HISTO_LINEAR);
  ps.endpage();
  ps.trailer();
  ps.close();
  doc.writeXml(ofile);
  cout<<"Total contour length: "<<totalContourLength.total()<<"   \n";
  tassert(expectedLength<0 || fabs(totalContourLength.total()-expectedLength)<0.1);
  cout<<h.gettotal()<<" points sampled, "<<excessElevCount<<" out of tolerance\n";
  cout<<"Maximum error "<<maxElevError<<" Contour interval "<<conterval<<endl;
  tassert(sqr(h.gettotal()-totalContourLength.total())<9*totalContourLength.total());
  tassert(sqr(excessElevCount)<9*totalContourLength.total());
  /* Ideally excessElevCount is 0, and for the well-behaved contouraster it is,
   * but for the ill-behaved contourwheel, it is not.
   */
  cout<<"Total hash "<<tothash<<endl;
  tassert(maxElevError<conterval);
}

void testcontour()
/* The total lengths of contours, especially of the wheel pattern, are
 * sensitive to bendlimit. The values 2490.48 and 1836.62 are for bendlimit=120°.
 * The contours of the wheel pattern also depend on the processor and the
 * operating system. 1386.62 is for Intel Core I7 running Linux. On DragonFly
 * BSD, the top, left, and bottom are different because there is no M_PIl.
 * On Raspberry Pi, the contours are completely different because there are no
 * ten-byte floats.
 */
{
  xyz offset;
  ContourInterval oneMeter(1,0,false),twoMeter(1,1,false),fiveMeter(1,2,false);
  ContourInterval oneFoot(0.3048,0,false),twoFoot(0.3048,1,false),fiveFoot(0.3048,2,false);
  assert(oneMeter.mediumInterval()==1);
  assert(twoMeter.mediumInterval()==2);
  assert(fiveMeter.mediumInterval()==5);
  assert(oneMeter.coarseInterval()==5);
  assert(twoMeter.coarseInterval()==10);
  assert(fiveMeter.coarseInterval()==20);
  assert(oneFoot.mediumInterval()==0.3048);
  assert(twoFoot.mediumInterval()==0.6096);
  assert(fiveFoot.mediumInterval()==1.524);
  assert(oneFoot.coarseInterval()==1.524);
  assert(twoFoot.coarseInterval()==3.048);
  assert(fiveFoot.coarseInterval()==6.096);
  offset=xyz(-1000000,-1500000,0); // This offset made a spike in contours[7].
  doc.makepointlist(1);
  doc.pl[1].clear();
  doc.changeOffset(xyz(0,0,0));
  setsurface(CIRPAR);
  aster(doc,100);
  moveup(doc,-0.001);
  doc.changeOffset(offset);
  // The triangle with center (0.6438,3.85625) had a spike in a contour.
  test1contour("contouraster",offset,xy(100.6438,3.85625),0.03,2490.48);
  doc.pl[1].clear();
  doc.changeOffset(xyz(0,0,0));
  setsurface(HASH);
  wheelwindow(doc,100);
  moveup(doc,-0.001);
  doc.changeOffset(offset);
  /* The triangle with center (-6.677,-0.21) is where tracing got lost.
   * It had different numbers of subdiv segments when displaced than not.
   * This intermittent bug remained after the main bug was fixed.
   * This is the triangle with corners (102 104 1). When the corners are
   * assigned to points A,B,C in that order, there is no problem. In some
   * other order, it failed, as follows:
   * A is point 102, 10.025 m W of point 1 (but it's not A when it fails).
   * B is point 104, S86.4W 10,025 from point 1.
   * C is point 1, at (0,0) or displaced to (1 Mm,1.5 Mm).
   * D is the primary critical point, S88.1574W 0.0254 from C.
   * E is on CA near D.
   * F is on BC near D.
   * G is on AB near B.
   * H is on BC near B.
   * I is the secondary critical point, on DH near H.
   * It subdivides the triangle with DE, DF, DC, GH, DG, DA, DI, IH, IF, and IG.
   * However, it sometimes kept IC, thinking that it intersects DF at F,
   * instead of between D and F. One of the three possible values for point I
   * caused the problem.
   */
  test1contour("contourwheel",offset,xy(-106.677,-0.21),0.3,-1836.62);
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
  PostScript ps;
  ps.open("foldcontour.ps");
  ps.prolog();
  ps.startpage();
  ps.setscale(194,-143,221,182,-DEG60);
  doc.makepointlist(1);
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
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(194,-143,221,182,-DEG60);
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.trailer();
  ps.close();
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
  PostScript ps;
  ps.open("tracingstop.ps");
  ps.prolog();
  ps.startpage();
  ps.setscale(144,51,147,54,0);
  doc.makepointlist(1);
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
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(144,51,147,54,0);
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"foldcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.trailer();
  ps.close();
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
  PostScript ps;
  ps.open("zigzagcontour.ps");
  ps.prolog();
  ps.startpage();
  ps.setscale(15111,14793,15346,15108,0);
  doc.makepointlist(1);
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
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"zigzagcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  conterval=0.1;
  roughcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.startpage();
  ps.setscale(15111,14793,15346,15108,0);
  ps.setcolor(0,1,1);
  for (i=0;i<doc.pl[1].triangles.size();i++)
    for (j=0;j<doc.pl[1].triangles[i].subdiv.size();j++)
      ps.spline(doc.pl[1].triangles[i].subdiv[j].approx3d(1));
  //rasterdraw(doc.pl[1],xy(0,0),30,30,30,0,3,"zigzagcontour.ppm");
  //cout<<"Lowest "<<tinlohi[0]<<" Highest "<<tinlohi[1]<<endl;
  smoothcontours(doc.pl[1],conterval);
  ps.setcolor(0,0,0);
  for (i=0;i<doc.pl[1].contours.size();i++)
  {
    //cout<<"Contour length: "<<doc.pl[1].contours[i].length()<<endl;
    ps.spline(doc.pl[1].contours[i].approx3d(1));
  }
  ps.endpage();
  ps.trailer();
  ps.close();
  doc.writeXml(ofile);
}

void testleastsquares()
{
  matrix a(3,2);
  vector<double> b,x;
  b.push_back(4);
  b.push_back(1);
  b.push_back(3);
  a[0][0]=1;
  a[0][1]=3; // http://ltcconline.net/greenl/courses/203/MatrixOnVectors/leastSquares.htm
  a[1][0]=2;
  a[1][1]=4;
  a[2][0]=1;
  a[2][1]=6;
  x=linearLeastSquares(a,b);
  cout<<"Least squares ("<<ldecimal(x[0])<<','<<ldecimal(x[1])<<")\n";
  tassert(dist(xy(x[0],x[1]),xy(-29/77.,51/77.))<1e-9);
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
  PostScript pscr;
  cubic.setslope(START,1);
  pscr.open("splitcubic.ps");
  pscr.prolog();
  pscr.startpage();
  pscr.setscale(-1,0,1,0.5,0);
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
  pscr.spline(ps.approx3d(0.1));
  pscr.setcolor(0,0,1);
  pscr.spline(pspos.approx3d(0.1));
  pscr.spline(psneg.approx3d(0.1));
  pscr.endpage();
  pscr.trailer();
  pscr.close();
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
  RoscatStruct ros;
  vector<int> ainx,binx;
  doc.makepointlist(1);
  for (i=0;i<2;i++)
    doc.pl[i].clear();
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
  doc.pl[0].addpoint(1,point(0,0,0,"eip"));
  doc.pl[0].addpoint(2,point(41,0,0,"eip"));
  doc.pl[0].addpoint(3,point(41,99,0,"eip"));
  doc.pl[0].addpoint(4,point(0,99,0,"eip"));
  doc.pl[1].addpoint(1,point(0,0,0,"eip"));
  doc.pl[1].addpoint(3,point(29,29,0,"eip"));
  doc.pl[1].addpoint(4,point(-41,99,0,"eip"));
  doc.pl[1].addpoint(2,point(-70,70,0,"eip"));
  ainx.push_back(1);
  ainx.push_back(2);
  ainx.push_back(3);
  ainx.push_back(4);
  binx.push_back(1);
  binx.push_back(3);
  binx.push_back(4);
  binx.push_back(2);
  ros=absorient(doc.pl[0],ainx,doc.pl[1],binx);
  cout<<"From "<<ros.tfrom.getx()<<','<<ros.tfrom.gety()<<" rotate "<<ros.ro;
  cout<<" to "<<ros.tto.getx()<<','<<ros.tto.gety()<<endl;
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
  halton h;
  int i,j,bartot;
  double x,wid,st;
  PostScript ps;
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
  ps.open("histogram.ps");
  ps.setpaper(papersizes["A4 portrait"],1);
  ps.prolog();
  for (j=0;j<60;j++)
  {
    wid=exp((j-29.5)/8);
    st=frac(j*M_1PHI)*log(10);
    histo0.clear(st,st+wid);
    for (i=0;i<15625;i++)
    {
      x=h.scalar(wid)+st;
      histo0<<x;
    }
    ps.startpage();
    histo0.plot(ps,HISTO_LOG);
    ps.endpage();
  }
  ps.close();
}

void testsmooth5()
{
  unsigned int i,lasti,previ;
  int nsmooth=0,recip=2,lastrecip=1,prevrecip;
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
  for (i=21600;i<DEG180;i++)
    if (smooth5(i))
    {
      recip=rint((double)DEG180/i);
      if (recip==lastrecip)
        break;
      prevrecip=lastrecip;
      lastrecip=recip;
      previ=lasti;
      lasti=i;
    }
  cout<<"Finest angle usable is "<<previ<<" per 180°.\n"<<lasti<<" and "<<i<<" are too close."<<endl;
  for (i=4294967295;!smooth5(i);--i);
  cout<<"Largest smooth number is "<<i<<endl;
  tassert(nearestSmooth(7)==8);
  tassert(nearestSmooth(rint((double)DEG180/SMOOTH5LIMIT))==1620000);
}

void testquadhash()
{
  int i,j,l,lastang=-1,hash,qsz=16;
  int n,lastn=300;
  int nhashes[17]={1,1,9,29,93,201,433,749,1309,2041,3145,4493,6453,8745,11905,15557,20173};
  map<double,int> langles;
  map<double,int>::iterator k;
  map<int,double> hashmap;
  vector<int> btangles;
  matrix acorr,invcorr;
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
  for (qsz=4;qsz<=16;qsz++)
  {
    hashmap.clear();
    for (i=0;i<qsz;i++)
      for (j=0;j<qsz;j++)
	all256[i][j]=sin(i-(qsz-1)*0.5+(j-(qsz-1)*0.5)*M_1PHI+M_PI/4);
    for (l=0;l<576;l++)
    {
      s=sin(btangles[l]);
      c=cos(btangles[l]);
      for (ofs=0;ofs<10.6;ofs+=0.0234375)
      {
	sum=n=0;
	for (i=0;i<qsz;i++)
	{
	  y=i-(qsz-1)*0.5;
	  for (j=0;j<qsz;j++)
	  {
	    x=j-(qsz-1)*0.5;
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
	hash=quadhash(some256,qsz);
	if (hashmap[hash]!=0 && hashmap[hash]!=sum)
	{
	  cout<<"Hash collision: "<<hash<<' '<<l<<' '<<ofs<<endl;
	}
	if (hashmap[hash]==0 && n==256 /*&& l%32==0*/)
	{
	  acorr=autocorr(some256,qsz);
	  invcorr=invert(acorr);
	  acorr.dump();
	  dump256(some256,qsz);
	  invcorr.dump();
	  cout<<endl;
	}
	hashmap[hash]=sum;
      }
    }
    cout<<hashmap.size()<<" different hashes"<<endl;
    tassert(hashmap.size()==nhashes[qsz]);
  }
}

void testvball()
{
  int lat,lon,olat,olon,i,j;
  vball v,places[33];
  bool vequal,xyzequal;
  string placenames[33];
  xyz dir;
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
  cout<<"Testing equality of volleyball coordinates..."<<endl;
  places[0]=vball(1,xy(-1,-0.51473));
  places[1]=vball(5,xy(-0.51473,-1));
  placenames[0]=placenames[1]="Divinópolis, Minas Gerais";
  places[2]=vball(5,xy(-0.60349,1));
  places[3]=vball(6,xy(-1,0.60349));
  placenames[2]=placenames[3]="Mangareva, French Polynesia";
  places[4]=vball(6,xy(1,0.86386));
  places[5]=vball(2,xy(-0.86386,-1));
  placenames[4]=placenames[5]="Lake Everard, South Australia";
  places[6]=vball(2,xy(0.32052,1));
  places[7]=vball(1,xy(1,0.32052));
  placenames[6]=placenames[7]="Aden, Yemen";
  places[8]=vball(1,xy(0.43136,1));
  places[9]=vball(3,xy(1,0.43136));
  placenames[8]=placenames[9]="Sofia, Bulgaria";
  places[10]=vball(2,xy(1,-0.71990));
  places[11]=vball(3,xy(-0.71990,1));
  placenames[10]=placenames[11]="Pyongyang, North Korea";
  places[12]=vball(6,xy(0.89463,-1));
  places[13]=vball(3,xy(-1,0.89463));
  placenames[12]=placenames[13]="Nagano, Japan";
  places[14]=vball(5,xy(1,0.05405));
  places[15]=vball(3,xy(-0.05405,-1));
  placenames[14]=placenames[15]="St. Paul, Minnesota";
  places[16]=vball(1,xy(0.06993,-1));
  places[17]=vball(4,xy(1,-0.06993));
  placenames[16]=placenames[17]="Meteor Rise Seamount";
  places[18]=vball(2,xy(-1,-0.06993));
  places[19]=vball(4,xy(-0.06993,-1));
  placenames[18]=placenames[19]="point on Southeast Indian Ocean Ridge";
  places[20]=vball(6,xy(0.15385,1));
  places[21]=vball(4,xy(-1,-0.15385));
  placenames[20]=placenames[21]="Mount Taylor, South Island";
  places[22]=vball(5,xy(-1,-0.33266));
  places[23]=vball(4,xy(0.33266,1));
  placenames[22]=placenames[23]="Corcovado, Chubut";
  places[24]=vball(1,xy(1,1));
  places[25]=vball(2,xy(1,1));
  places[26]=vball(3,xy(1,1));
  placenames[24]=placenames[25]=placenames[26]="Chamchamal, Sulaymaniya";
  places[27]=vball(6,xy(1,-1));
  places[28]=vball(2,xy(1,-1));
  places[29]=vball(3,xy(-1,1));
  placenames[27]=placenames[28]=placenames[29]="Tamba, Hyogo";
  /* This is a close call. The point is about as far north of Tamba as it is
   * west of Fukuchiyama, Kyoto. It's about 1 km WSW of a bend in the border,
   * on the Hyogo side.
   */
  places[30]=vball(6,xy(1,1));
  places[31]=vball(2,xy(-1,-1));
  places[32]=vball(4,xy(-1,-1));
  placenames[30]=placenames[31]=placenames[32]="Port Lincoln, South Australia";
  for (i=0;i<sizeof(places)/sizeof(places[0]);i++)
  {
    dir=decodedir(places[i]);
    cout<<radtoangle(dir.lat(),DEGREE+SEXAG1)<<' '<<radtoangle(dir.lon(),DEGREE+SEXAG1)<<' ';
    cout<<placenames[i]<<endl;
  }
  for (i=0;i<sizeof(places)/sizeof(places[0]);i++)
  {
    v=places[i];
    v.switchFace();
    cout<<((v==places[i])?':':'x');
    //tassert(v==places[i] && v.face!=places[i].face);
    for (j=0;j<sizeof(places)/sizeof(places[0]);j++)
    {
      vequal=places[i]==places[j];
      xyzequal=decodedir(places[i])==decodedir(places[j]);
      cout<<((vequal)?"* ":"  ");
      tassert(vequal==xyzequal);
    }
    cout<<endl;
  }
  cout<<"done."<<endl;
}

xy unfold(vball pnt)
{
  xy ret(-2,-2);
  switch (pnt.face)
  {
    case 1:
      ret=xy(pnt.x,pnt.y);
      break;
    case 2:
      ret=xy(2-pnt.y,pnt.x);
      break;
    case 3:
      ret=xy(pnt.y,2-pnt.x);
      break;
    case 4:
      ret=xy(-pnt.y,pnt.x-2);
      break;
    case 5:
      ret=xy(-2-pnt.y,pnt.x);
      break;
    case 6:
      ret=xy(4-pnt.x,-pnt.y);
      break;
  }
  return ret;
}

void plot1bdy(PostScript &ps,g1boundary g1)
{
  int i;
  ps.startline();
  for (i=0;i<g1.size();i++)
    ps.lineto(unfold(g1[i]));
  ps.endline(true);
}

void plotbdy(PostScript &ps,gboundary &gb)
{
  int i;
  ps.setscale(-3,-3,3,5,DEG90);
  for (i=0;i<gb.size();i++)
    plot1bdy(ps,gb[i]);
}

char displayDigits[6][7]=
{
  {
    0x04, //  *  
    0x0c, // **  
    0x14, //* *  
    0x04, //  *  
    0x04, //  *  
    0x04, //  *  
    0x1f  //*****
  },
  {
    0x0e, // *** 
    0x11, //*   *
    0x01, //    *
    0x02, //   * 
    0x04, //  *  
    0x08, // *   
    0x1f  //*****
  },
  {
    0x0e, // *** 
    0x11, //*   *
    0x01, //    *
    0x06, //  ** 
    0x01, //    *
    0x11, //*   *
    0x0e  // *** 
  },
  {
    0x02, //   * 
    0x06, //  ** 
    0x0a, // * * 
    0x1f, //*****
    0x02, //   * 
    0x02, //   * 
    0x02  //   * 
  },
  {
    0x1f, //*****
    0x10, //*    
    0x10, //*    
    0x1e, //**** 
    0x01, //    *
    0x01, //    *
    0x1e  //**** 
  },
  {
    0x07, //  ***
    0x08, // *   
    0x10, //*    
    0x1e, //**** 
    0x11, //*   *
    0x11, //*   *
    0x0e  // *** 
  }
};

void testgeoidboundary()
{
  int i,j,r,gbarea;
  double x,peri;
  g1boundary g1,g2;
  gboundary gb;
  vball v;
  smallcircle c;
  Quaternion ro;
  geoid gd,outgd;
  PostScript ps;
  tassert(splitLevel(-1)==0);
  tassert(splitLevel(1)==0);
  r=rng.uirandom();
  for (x=0,i=1;i<33;i++)
  {
    tassert(splitLevel(x)==i);
    x=x/2+((r&(1<<(i-1)))?0.5:-0.5);
  }
  ro=randomVersor();
  //ro=versor(xyz(0,0,0));
  v.face=1;
  v.x=v.y=0;
  g1.push_back(v);
  v.x=0.5;
  g1.push_back(v);
  v.y=0.5;
  g1.push_back(v);
  v.x=0;
  g1.push_back(v);
  v.y=0;
  v.x=-0.25;
  g2.push_back(v);
  v.y=-0.5;
  g2.push_back(v);
  v.x=0.25;
  g2.push_back(v);
  v.y=0;
  g2.push_back(v);
  gb.push_back(g1);
  gb.push_back(g2);
  gb.consolidate(0);
  tassert(gb.size()==2);
  tassert(gb[0].size()==4);
  tassert(gb[1].size()==4);
  tassert(fabs(gb.perimeter()-23e6)<0.2e6);
  tassert(fabs(bintodeg(gb.area())-24)<0.01);
  gb.consolidate(2);
  tassert(gb.size()==2);
  tassert(gb[0].size()==4); // Nothing's happened, because the two squares
  tassert(gb[1].size()==4); // meet on a line at level 1.
  tassert(fabs(gb.perimeter()-23e6)<0.2e6);
  tassert(fabs(bintodeg(gb.area())-24)<0.01);
  cout<<"perimeter of two squares is "<<gb.perimeter()<<" before consolidating"<<endl;
  gbarea=gb.area();
  cout<<"area "<<gbarea<<' '<<bintodeg(gbarea)<<endl;
  gb.consolidate(1);
  tassert(gb.size()==2);
  tassert(gb[0].size()==8);
  tassert(gb[1].size()==0);
  tassert(fabs(gb.perimeter()-20e6)<0.2e6);
  tassert(fabs(bintodeg(gb.area())-24)<0.01);
  cout<<"perimeter of two squares is "<<gb.perimeter()<<" after consolidating"<<endl;
  gbarea=gb.area();
  cout<<"area "<<gbarea<<' '<<bintodeg(gbarea)<<endl;
  gb.splitoff(1);
  tassert(gb.size()==2);
  gb.deleteCollinear();
  tassert(gb[0].size()==8);
  tassert(fabs(gb.perimeter()-20e6)<0.2e6);
  tassert(fabs(bintodeg(gb.area())-24)<0.01);
  gb.deleteEmpty();
  tassert(gb.size()==1);
  tassert(gb[0].size()==8);
  tassert(fabs(gb.perimeter()-20e6)<0.2e6);
  tassert(fabs(bintodeg(gb.area())-24)<0.01);
  // start another test: 回
  gb.clear();
  g1.clear();
  v.face=2;
  v.x=v.y=-1;
  g1.push_back(v);
  v.x=.5;
  g1.push_back(v);
  v.y=.5;
  g1.push_back(v);
  v.x=-1;
  g1.push_back(v);
  v.y=-.5;
  g1.push_back(v);
  v.x=-.5;
  g1.push_back(v);
  v.y=0;
  g1.push_back(v);
  v.x=0;
  g1.push_back(v);
  v.y=-.5;
  g1.push_back(v);
  v.x=-1;
  g1.push_back(v);
  gb.push_back(g1);
  cout<<"perimeter of square in square is "<<gb.perimeter()<<" before splitting off"<<endl;
  gbarea=gb.area();
  cout<<"area "<<gbarea<<' '<<bintodeg(gbarea)<<endl;
  tassert(fabs(gb.perimeter()-42e6)<0.2e6);
  tassert(fabs(bintodeg(gbarea)-66.87)<0.01);
  gb.splitoff(3);
  gb.deleteCollinear();
  cout<<"gb.size at 3 "<<gb.size()<<endl;
  gb.splitoff(2);
  gb.deleteCollinear();
  cout<<"gb.size at 2 "<<gb.size()<<endl;
  gb.splitoff(1);
  gb.deleteCollinear();
  cout<<"gb.size at 1 "<<gb.size()<<endl;
  for (i=0;i<gb.size();i++)
    cout<<"gb["<<i<<"].size "<<gb[i].size()<<endl;
  cout<<"perimeter of square in square is "<<gb.perimeter()<<" after splitting off"<<endl;
  gbarea=gb.area();
  cout<<"area "<<gbarea<<' '<<bintodeg(gbarea)<<endl;
  tassert(gb.size()==2);
  tassert(gb[0].size()==4);
  tassert(gb[1].size()==4);
  tassert(fabs(gb.perimeter()-38e6)<0.2e6);
  tassert(fabs(bintodeg(gbarea)-66.87)<0.01);
  // start another test: back and forth along an oblique or edge-crossing line
  gb.clear();
  g1.clear();
  v.face=1;
  v.x=0.8;
  v.y=0.9;
  g1.push_back(v);
  v.face=2;
  g1.push_back(v);
  v.face=3;
  g1.push_back(v);
  v.face=1;
  g1.push_back(v);
  v.face=5;
  v.x=-0.9;
  v.y=0.8;
  g1.push_back(v);
  v.face=6;
  g1.push_back(v);
  v.face=4;
  g1.push_back(v);
  v.face=5;
  g1.push_back(v);
  gb.push_back(g1);
  cout<<"perimeter of two triangles is "<<gb.perimeter()<<" before splitting off"<<endl;
  gb.splitoff(-1);
  cout<<"perimeter of two triangles is "<<gb.perimeter()<<" after splitting off"<<endl;
  tassert(gb.size()==2);
  /* The main test is as follows:
   * 1. Create an excerpt of a global geoid file, using five circles centered
   *    at corners of a regular icosahedron and passing through corners of
   *    a regular dodecahedron. The icosahedron is in random orientation.
   * 2. Compute the boundary of the cubemap.
   * 3. Check that the boundary has two components and that the length
   *    (measured by midpoints) and area are close to correct.
   * Corner of an icosahedron: (0,1,φ) normalized -> (0,0.52573,0.85065)
   * Corner of a dodecahedron: (sqrt(1/3),sqrt(1/3),sqrt(1/3))
   * Radius of a circle: 4156.174 km, 37.40556°, 223132877
   * Area of figure: 242.1199 Mm², 341.77317°, 2038756371
   * Perimeter of figure: 72902.392 km, 656.12152°, 3913917342.
   */
  excerptcircles.clear();
  c.center=ro.rotate(xyz(3524578,5702887,0));
  c.setradius(223132877);
  excerptcircles.push_back(c);
  c.center=ro.rotate(xyz(5702887,0,3524578));
  c.setradius(223132877);
  excerptcircles.push_back(c);
  c.center=ro.rotate(xyz(0,-3524578,5702887));
  c.setradius(223132877);
  excerptcircles.push_back(c);
  c.center=ro.rotate(xyz(-5702887,0,3524578));
  c.setradius(223132877);
  excerptcircles.push_back(c);
  c.center=ro.rotate(xyz(-3524578,5702887,0));
  c.setradius(223132877);
  excerptcircles.push_back(c);
  geo.clear();
  geo.push_back(gd);
  outgd.ghdr=new geoheader;
  outgd.cmap=new cubemap;
  outgd.cmap->scale=1/65536.;
  outgd.ghdr->logScale=-16;
  outgd.ghdr->planet=BOL_EARTH;
  outgd.ghdr->dataType=BOL_UNDULATION;
  outgd.ghdr->encoding=BOL_VARLENGTH;
  outgd.ghdr->ncomponents=1;
  outgd.ghdr->tolerance=0.003;
  outgd.ghdr->sublimit=1000;
  outgd.ghdr->spacing=1e5;
  totalArea.clear();
  dataArea.clear();
  for (i=0;i<6;i++)
  {
    interroquad(outgd.cmap->faces[i],3e5);
    refine(outgd.cmap->faces[i],outgd.cmap->scale,outgd.ghdr->tolerance,outgd.ghdr->sublimit,outgd.ghdr->spacing,4,false);
  }
  outProgress();
  cout<<endl;
  drawglobecube(1024,62,-7,&outgd,0,"geoidboundary.ppm");
  gb=outgd.cmap->gbounds();
  ps.open("geoidboundary.ps");
  ps.prolog();
  ps.startpage();
  plotbdy(ps,gb);
  ps.endpage();
  ps.trailer();
  ps.close();
  writeboldatni(outgd,"geoidboundary.bol");
  cout<<"gb.size "<<gb.size()<<endl;
  for (i=0;i<gb.size();i++)
    cout<<"gb["<<i<<"].size "<<gb[i].size()<<endl;
  tassert(gb.size()==2);
  peri=gb.perimeter(true);
  cout<<"perimeter of five circles is "<<peri<<endl;
  tassert(peri>72.9e6 && peri<75e6);
  gbarea=gb.area();
  cout<<"area "<<gbarea<<' '<<bintodeg(gbarea)<<endl;
  tassert(gbarea>2038756371 && gbarea<2060000000);
  // The area is a little bigger so that it covers all the circles completely.
}

void testvballgeoid()
  /* Make a geoid file showing the numerals 1-6 on their faces. The KML file
   * will be for developers to see how volleyball coordinates work.
   */
{
  int i,j;
  int nCircles=11;
  int circleSize=rint(DEG45/nCircles);
  gboundary gb;
  vball v;
  smallcircle c;
  geoid gd,outgd;
  PostScript ps;
  excerptcircles.clear();
  for (v.face=1;v.face<7;v.face++)
  {
    for (i=0;i<nCircles;i++)
    { // Draw the borders of the faces.
      v.y=-1;
      v.x=(2*i+0.5-nCircles)/nCircles;
      c.center=decodedir(v);
      c.setradius(circleSize);
      excerptcircles.push_back(c);
      v.x=1;
      v.y=(2*i+0.5-nCircles)/nCircles;
      c.center=decodedir(v);
      c.setradius(circleSize);
      excerptcircles.push_back(c);
      v.y=1;
      v.x=(nCircles-2*i-0.5)/nCircles;
      c.center=decodedir(v);
      c.setradius(circleSize);
      excerptcircles.push_back(c);
      v.x=-1;
      v.y=(nCircles-2*i-0.5)/nCircles;
      c.center=decodedir(v);
      c.setradius(circleSize);
      excerptcircles.push_back(c);
    }
    for (i=0;i<7;i++)
      for (j=0;j<5;j++)
	if ((displayDigits[v.face-1][i]>>j)&1)
	{
	  v.x=(2.-j)/nCircles;
	  v.y=(3.-i)/nCircles;
	  c.center=decodedir(v);
	  c.setradius(circleSize);
	  excerptcircles.push_back(c);
	}
  }
  geo.clear();
  geo.push_back(gd);
  outgd.ghdr=new geoheader;
  outgd.cmap=new cubemap;
  outgd.cmap->scale=1/65536.;
  outgd.ghdr->logScale=-16;
  outgd.ghdr->planet=BOL_EARTH;
  outgd.ghdr->dataType=BOL_UNDULATION;
  outgd.ghdr->encoding=BOL_VARLENGTH;
  outgd.ghdr->ncomponents=1;
  outgd.ghdr->tolerance=0.003;
  outgd.ghdr->sublimit=1000;
  outgd.ghdr->spacing=1e5;
  totalArea.clear();
  dataArea.clear();
  for (i=0;i<6;i++)
  {
    interroquad(outgd.cmap->faces[i],3e5);
    refine(outgd.cmap->faces[i],outgd.cmap->scale,outgd.ghdr->tolerance,outgd.ghdr->sublimit,outgd.ghdr->spacing,4,false);
  }
  outProgress();
  cout<<endl;
  drawglobecube(1024,62,-7,&outgd,0,"vball.ppm");
  gb=outgd.cmap->gbounds();
  ps.open("vball.ps");
  ps.prolog();
  ps.startpage();
  plotbdy(ps,gb);
  ps.endpage();
  ps.trailer();
  ps.close();
  writeboldatni(outgd,"vball.bol");
}

void drawproj1bdy(PostScript &ps,polyarc proj1bdy)
{
  int i,ori=0;
  double minx,maxx,miny,maxy;
  latlong ll;
  minx=proj1bdy.dirbound(-ori);
  miny=proj1bdy.dirbound(-ori+DEG90);
  maxx=-proj1bdy.dirbound(-ori+DEG180);
  maxy=-proj1bdy.dirbound(-ori-DEG90);
  ps.startpage();
  ps.setscale(minx,miny,maxx,maxy,ori);
  ps.spline(proj1bdy.approx3d(0.1/ps.getscale()));
  ps.endpage();
}

void test1kml(cylinterval cyl,string name,int pieces)
{
  gboundary bdy;
  bdy=gbounds(cyl);
  tassert(pieces<0 || pieces==bdy.size());
  if (pieces>=0 && pieces!=bdy.size())
    cerr<<name<<": "<<pieces<<" pieces expected, "<<bdy.size()<<" found"<<endl;
  outKml(bdy,name+".kml");
  cout<<name+".kml"<<endl;
}

void testkml()
{
  int i,r;
  PostScript ps;
  smallcircle avl150,tvu150,cham150,athwi150;
  cylinterval lune,nearpole,empty,emptym,emptyp,band30,band40,band50,antarctic,full;
  g1boundary gPode,gAntipode;
  gboundary gPodes,gRingFive,gVballGeoid,gOneFace,bigBdy,smallBdy;
  double bigperim,smallperim;
  geoid ringFive,vballGeoid,oneFace;
  KmlRegionList kmlReg;
  unsigned bigReg,smallReg;
  polyarc pPode,pAntipode;
  vball v;
  v.face=2;
  v.x=0.29296875;
  v.y=0.585693359375;
  gPode.push_back(v);
  v.x=0.29248046875;
  gPode.push_back(v);
  v.y=0.585205078125;
  gPode.push_back(v);
  v.x=0.29296875;
  gPode.push_back(v);
  v.face=5;
  v.x=-0.29296875;
  v.y=0.585205078125;
  gAntipode.push_back(v);
  v.x=-0.29248046875;
  gAntipode.push_back(v);
  v.y=0.585693359375;
  gAntipode.push_back(v);
  v.x=-0.29296875;
  gAntipode.push_back(v);
  gPodes.push_back(gPode);
  gPodes.push_back(gAntipode);
  pPode=flatten(gPode);
  pAntipode=flatten(gAntipode);
  cout<<"Pode length "<<pPode.length()<<" Antipode length "<<pAntipode.length()<<endl;
  cout<<"Pode area "<<pPode.area()<<" Antipode area "<<pAntipode.area()<<endl;
  tassert(fabs(pPode.length()-9585.5)<0.1);
  tassert(fabs(pAntipode.length()-9.845e11)<1e8);
  tassert(pPode.area()>0);
  tassert(pAntipode.area()<0);
  kmlReg=kmlRegions(gPodes);
  cout<<kmlReg.regionMap.size()<<" regions; blank regions are inside "<<kmlReg.blankBitCount<<" boundaries"<<endl;
  tassert(kmlReg.regionMap.size()==3);
  tassert(kmlReg.blankBitCount==0);
  bigReg=kmlReg.biggestBlankRegion(gPodes);
  cout<<"biggest blank region is "<<bigReg<<endl;
  tassert(bigReg==0);
  ps.open("kml.ps");
  ps.prolog();
  drawproj1bdy(ps,pPode);
  drawproj1bdy(ps,pAntipode);
  ps.startpage();
  plotbdy(ps,gPodes);
  ps.endpage();
  // Start test with file previously written by testgeoidboundary
  if (readboldatni(ringFive,"geoidboundary.bol")<2)
    cerr<<"Please run \"bezitest geoidboundary\" first."<<endl;
  else
    cout<<"Read geoidboundary.bol written by geoidboundary test"<<endl;
  gRingFive=ringFive.cmap->gbounds();
  kmlReg=kmlRegions(gRingFive);
  cout<<kmlReg.regionMap.size()<<" regions; blank regions are inside "<<kmlReg.blankBitCount<<" boundaries"<<endl;
  for (i=0;i<gRingFive.size();i++)
    drawproj1bdy(ps,gRingFive.getFlatBdy(i));
  tassert(kmlReg.regionMap.size()==3);
  tassert(kmlReg.blankBitCount==1);
  bigReg=kmlReg.biggestBlankRegion(gRingFive);
  cout<<"biggest blank region is "<<bigReg<<endl;
  /* The biggest blank region can be 1 or 2, depending on where on earth the
   * ring is. The other blank region is 2 or 1. The nonblank region is 3.
   * The biggest region's boundary is twice as long as the smallest region's.
   */
  smallReg=3-bigReg;
  tassert(bigReg*smallReg==2);
  bigBdy=regionBoundary(kmlReg,gRingFive,bigReg);
  smallBdy=regionBoundary(kmlReg,gRingFive,smallReg);
  bigperim=bigBdy.perimeter(true);
  smallperim=smallBdy.perimeter(true);
  cout<<"bigBdy is "<<bigperim/smallperim<<" times as long as smallBdy"<<endl;
  tassert(bigperim/smallperim<2.07 && bigperim/smallperim>1.98);
  ps.trailer();
  ps.close();
  outKml(gRingFive,"geoidboundary.kml");
  if (readboldatni(vballGeoid,"vball.bol")==2)
  {
    gVballGeoid=vballGeoid.cmap->gbounds();
    outKml(gVballGeoid,"vball.kml");
  }
  /* Test a single face. This is to check whether the polygons in KML consist
   * of geodesics or loxodromes.
   */
  oneFace.ghdr=new geoheader;
  oneFace.cmap=new cubemap;
  *oneFace.ghdr=*ringFive.ghdr;
  oneFace.cmap->faces[0].und[0]=65535;
  gOneFace=oneFace.cmap->gbounds();
  outKml(gOneFace,"oneface.kml");
  avl150.center=Sphere.geoc(degtobin(35.58),degtobin(-82.56),0);
  r=radtobin(15e4/EARTHRAD);
  avl150.setradius(r);
  cham150.center=xyz(EARTHRAD,EARTHRAD,EARTHRAD)/M_SQRT_3;
  cham150.setradius(r);
  tvu150.center=Sphere.geoc(degtobin(-16.86),degtobin(-179.95),0);
  tvu150.setradius(r);
  athwi150.center=xyz(0,-4504977.3,4504977.3);
  athwi150.setradius(r);
  test1kml(avl150.boundrect(),"avl150",1);
  test1kml(cham150.boundrect(),"cham150",1);
  test1kml(tvu150.boundrect(),"tvu150",1);
  test1kml(athwi150.boundrect(),"athwi150",1);
  i=rng.uirandom();
  r=(rng.usrandom()<<14)-0x1fffaa04;
  lune.wbd=i;
  lune.ebd=i+DEG60;
  lune.sbd=-DEG90;
  lune.nbd=DEG90;
  nearpole.wbd=i;
  nearpole.ebd=i+DEG60;
  nearpole.sbd=MIN1-DEG90;
  nearpole.nbd=DEG90-MIN1;
  empty.wbd=empty.ebd=i;
  empty.nbd=empty.sbd=r;
  emptym.wbd=emptym.ebd=i;
  emptym.nbd=DEG40;
  emptym.sbd=-DEG40;
  emptyp.wbd=i;
  emptyp.ebd=i+DEG60;
  emptyp.nbd=emptyp.sbd=r;
  band30.wbd=band40.wbd=band50.wbd=antarctic.wbd=full.wbd=i;
  band30.ebd=band40.ebd=band50.ebd=antarctic.ebd=full.ebd=i+DEG360;
  band30.nbd=DEG30;
  band30.sbd=-DEG30;
  band40.nbd=DEG40;
  band40.sbd=-DEG40;
  band50.nbd=DEG50;
  band50.sbd=-DEG50;
  antarctic.nbd=-DEG60; // boundary of Southern Ocean
  antarctic.sbd=-397063600; // Antarctic Circle
  full.nbd=DEG90;
  full.sbd=-DEG90;
  test1kml(lune,"lune",1);
  test1kml(nearpole,"nearpole",1);
  test1kml(empty,"empty",0);
  test1kml(emptym,"emptym",0);
  test1kml(emptyp,"emptyp",0);
  test1kml(band30,"band30",2);
  test1kml(band40,"band40",2);
  test1kml(band50,"band50",2);
  test1kml(antarctic,"antarctic",2);
  test1kml(full,"full",0);
}

void testgeoid()
{
  array<vball,4> bounds;
  array<double,4> bdist;
  array<int,6> undrange;
  array<int,5> undhisto;
  int i,j,k,qsz=16;
  /* qsz is the size of the square lattice used to sample a geoquad.
   * It can range from 4 to 16.
   */
  double x,y,sum,qpoints[16][16],u0,u1;
  //vector<double> anga,apxa;
  double areadiff,minareadiff;
  int minareasub;
  vball v;
  geoquad gq,gq1,*pgq;
  geoheader hdr;
  fstream file;
  array<unsigned,2> ghash;
  array<double,6> corr;
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
    corr=correction(gq,qpoints,qsz);
    for (i=0;i<6;i++)
      cout<<corr[i]<<' ';
    cout<<endl;
  }
  cout<<"done."<<endl;
  cout<<"Testing area of geoquad..."<<endl;
  gq1=gq;
  gq.clear();
  pgq=&gq;
  minareadiff=INFINITY;
  for (i=0;i<24;i++)
  {
    areadiff=(pgq->apxarea()-pgq->angarea())/sqr(pgq->scale);
    cout<<setw(2)<<i<<setprecision(10)
    <<setw(18)<<pgq->apxarea()/sqr(pgq->scale)
    <<setw(18)<<pgq->angarea()/sqr(pgq->scale)
    <<setw(18)<<pgq->area()/sqr(pgq->scale)
    <<setw(18)<<setprecision(6)<<areadiff
    <<endl;
    if (fabs(areadiff)<minareadiff)
    {
      minareadiff=fabs(areadiff);
      minareasub=i;
    }
    pgq->subdivide();
    pgq=pgq->sub[rng.ucrandom()&3];
  }
  cout<<"angarea and apxarea are closest at subdivision level "<<minareasub<<endl;
  gq1=gq;
  cout<<"done."<<endl;
  cout<<"Testing geoquad bounds..."<<endl;
  gq.clear();
  gq.center=xy(0.125,0.375);
  gq.scale=0.5;
  for (i=1;i<7;i++)
  {
    gq.face=i;
    v=gq.vcenter();
    bounds=gq.bounds();
    /* The bounds are the four great circles whose arcs bound the geoquad.
     * They are returned as the farther centers of the great circles.
     * They should be about 12.5 Mm from the center of the geoquad,
     * measured along the surface of the earth. Measured through the earth,
     * that is 10.6 Mm. Actual values range from 10.468 to 10.892 Mm.
     */
    for (j=0;j<4;j++)
      bdist[j]=dist(decodedir(v),decodedir(bounds[j]));
    stable_sort(bdist.begin(),bdist.end());
    for (j=0;j<4;j++)
      cout<<bdist[j]<<' ';
    cout<<endl;
    tassert(bdist[0]>10.45e6);
    tassert(bdist[3]<10.9e6);
  }
  gq1=gq;
  cout<<"done."<<endl;
  cout<<"Testing conversion from geolattice to geoquad and I/O..."<<endl;
  geo.resize(1);
  geo[0].glat=new geolattice;
  geo[0].glat->settest();
  for (i=0;i<5;i++)
    for (j=0;j<5;j++)
    {
      tassert(geo[0].glat->eslope[5*i+j]==89232+16384*j);
      tassert(geo[0].glat->nslope[5*i+j]==91784-8192*i);
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
  totalArea.clear();
  dataArea.clear();
  for (i=0;i<6;i++)
  {
    interroquad(cube.faces[i],3e5);
    refine(cube.faces[i],cube.scale,hdr.tolerance,hdr.sublimit,hdr.spacing,qsz,false);
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
      if (!std::isnan(u0))
      {
	tassert(fabs(u1-u0)<0.001);
      }
    }
  file.open("test.bol.dump",ios::out);
  cube.dump(file);
  file.close();
  undrange=cube.undrange();
  cout<<"Undulation range: constant "<<undrange[0]<<'-'<<undrange[1];
  cout<<" linear "<<undrange[2]<<'-'<<undrange[3];
  cout<<" quadratic "<<undrange[4]<<'-'<<undrange[5]<<endl;
  undhisto=cube.undhisto();
  cout<<"1 byte "<<undhisto[0]<<"; 2 bytes "<<undhisto[1]<<"; 3 bytes "<<undhisto[2]<<"; 4 bytes "<<undhisto[3]<<endl;
  writeusngsbin(geo[0],"test.bin");
  writecarlsongsf(geo[0],"test.gsf");
  writeusngatxt(geo[0],"test.grd");
  cout<<"done."<<endl;
}

void outcyl(cylinterval c)
{
  cout<<"latitude "<<bintodeg(c.sbd)<<'-'<<bintodeg(c.nbd);
  cout<<" longitude "<<bintodeg(c.wbd)<<'-'<<bintodeg(c.ebd);
}

array<int,2> plotcenter(PostScript &ps,geoquad &quad,smallcircle sc)
{
  int i;
  array<int,2> ret,subcount;
  bool ovlp,ovlp0,centerin;
  centerin=sc.in(decodedir(quad.vcenter()));
  ovlp=overlap(sc,quad);
  /*ovlp0=overlap0(sc,quad);
  if (ovlp!=ovlp0)
  {
    ovlp=overlap(sc,quad);
    ovlp0=overlap0(sc,quad);
  }*/
  ret[0]=ovlp;
  ret[1]=centerin;
  if (ovlp && quad.subdivided())
    for (i=0;i<4;i++)
    {
      subcount=plotcenter(ps,*quad.sub[i],sc);
      ret[0]+=subcount[0];
      ret[1]+=subcount[1];
    }
  if (centerin)
    ps.setcolor(0,0,0);
  else
    ps.setcolor(0.5,0.5,1);
  ps.dot(unfold(quad.vcenter()));
  return ret;
}

array<int,2> plotcenters(PostScript &ps,string name,smallcircle sc)
{
  array<int,2> ret,subcount;
  ret[0]=ret[1]=0;
  ps.setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
  {
    subcount=plotcenter(ps,cube.faces[i],sc);
    ret[0]+=subcount[0];
    ret[1]+=subcount[1];
  }
  cout<<name<<' '<<ret[0]<<" overlap, "<<ret[1]<<" centers in"<<endl;
  return ret;
}

void testsmallcircle()
{
  int r,i;
  array<int,2> count;
  PostScript ps;
  smallcircle avl150,eho150,clt150,brw150; // Asheville, Shelby, Charlotte, Barrow
  smallcircle athwi45d; // Athens, Wisconsin, 45°N, to test a circle passing through the pole
  smallcircle ush4000; // circle encloses the pole
  smallcircle gps5311; // circle intersects five faces
  smallcircle cham8000; // centered near Chamchamal, circle intersects all six faces
  xyz xprod,qaraqoga;
  // Qaraqoğa, Pavlodar, Kazakhstan, is 10 Mm from both Asheville and Charlotte.
  vector<xyz> avlint,ehoint,cltint;
  cylinterval avlcyl,ehocyl,cltcyl,brwcyl,athwicyl,ushcyl;
  avl150.center=Sphere.geoc(degtobin(35.58),degtobin(-82.56),0);
  eho150.center=Sphere.geoc(degtobin(35.29),degtobin(-81.54),0);
  clt150.center=Sphere.geoc(degtobin(35.23),degtobin(-80.84),0);
  brw150.center=Sphere.geoc(degtobin(71.2906),degtobin(-156.789),0);
  qaraqoga=Sphere.geoc(degtobin(52.43),degtobin(75.07),0);
  r=radtobin(15e4/EARTHRAD);
  avl150.setradius(r);
  eho150.setradius(r);
  clt150.setradius(r);
  brw150.setradius(r);
  tassert(eho150.farin(clt150.center)>eho150.farin(avl150.center));
  tassert(avl150.in(eho150.center));
  tassert(!avl150.in(clt150.center));
  tassert(eho150.in(clt150.center));
  xprod=cross(avl150.center,clt150.center);
  xprod*=EARTHRAD/xprod.length();
  cout<<radtodeg(xprod.lat())<<' '<<radtodeg(xprod.lon())<<' '<<dist(xprod,qaraqoga)<<endl;
  tassert(dist(xprod,qaraqoga)<2e4);
  avlint=gcscint(xprod,avl150);
  ehoint=gcscint(xprod,eho150);
  cltint=gcscint(xprod,clt150);
  tassert(avlint.size()==2);
  tassert(ehoint.size()==2);
  tassert(cltint.size()==2);
  for (i=0;i<2;i++)
  {
    avlint[i]*=EARTHRAD;
    ehoint[i]*=EARTHRAD;
    cltint[i]*=EARTHRAD;
  }
  cout<<dist(avlint[0],avlint[1])<<' '<<dist(ehoint[0],ehoint[1])<<' '<<dist(cltint[0],cltint[1])<<endl;
  tassert(dist(avlint[0],avlint[1])>299792); // The distance < 300 km because
  tassert(dist(ehoint[0],ehoint[1])<299792); // it's straight through the earth.
  tassert(dist(cltint[0],cltint[1])>299792); // Shelby is off the great circle.
  athwi45d.center=xyz(0,-4504977.3,4504977.3);
  ush4000.center=Sphere.geoc(degtobin(-54.8),degtobin(-68.3),0);
  athwi45d.setradius(DEG45);
  ush4000.setradius(radtobin(4e6/EARTHRAD));
  avlcyl=avl150.boundrect();
  ehocyl=eho150.boundrect();
  cltcyl=clt150.boundrect();
  brwcyl=brw150.boundrect();
  athwicyl=athwi45d.boundrect();
  ushcyl=ush4000.boundrect();
  cout<<"Asheville ";
  outcyl(avlcyl);
  cout<<endl<<"Shelby ";
  outcyl(ehocyl);
  cout<<endl<<"Charlotte ";
  outcyl(cltcyl);
  cout<<endl<<"Barrow ";
  outcyl(brwcyl);
  cout<<endl<<"Athens, WI ";
  outcyl(athwicyl);
  cout<<endl<<"Ushuaia ";
  outcyl(ushcyl);
  cout<<endl;
  tassert(avlcyl.sbd==204195973);
  tassert(avlcyl.nbd==220289961);
  tassert(avlcyl.wbd==-502384254);
  tassert(avlcyl.ebd==-482594912);
  tassert(ehocyl.sbd==202466056);
  tassert(ehocyl.nbd==218560044);
  tassert(ehocyl.wbd==-496264134);
  tassert(ehocyl.ebd==-476545958);
  tassert(cltcyl.sbd==202108142);
  tassert(cltcyl.nbd==218202130);
  tassert(cltcyl.wbd==-492081173);
  tassert(cltcyl.ebd==-472377595);
  tassert(athwicyl.sbd==0);
  tassert(athwicyl.nbd==DEG90);
  tassert(athwicyl.wbd==-DEG180);
  tassert(athwicyl.ebd==0);
  tassert(ushcyl.sbd==-DEG90);
  tassert(ushcyl.nbd==-112308239);
  tassert(ushcyl.ebd-ushcyl.wbd==DEG360);
  gps5311.center=xyz(0,-EARTHRAD,0);
  cham8000.center=xyz(EARTHRAD,EARTHRAD,EARTHRAD)/M_SQRT_3;
  r=radtobin(8e6/EARTHRAD);
  cham8000.setradius(r);
  r=radtobin(5311111/EARTHRAD);
  gps5311.setradius(r);
  cube.clear();
  for (i=0;i<6;i++)
    cube.faces[i].filldepth(6);
  /* 64×64; as each face is a little less than 70 mm square, that's
   * about 1 dot per millimeter when printed
   */
  ps.open("smallcircle.ps");
  ps.prolog();
  ps.startpage();
  count=plotcenters(ps,"Asheville",avl150);
  tassert(count[0]==25 && count[1]==5);
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Shelby",eho150);
  tassert(count[0]==22 && count[1]==5);
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Charlotte",clt150);
  tassert(count[0]==21 && count[1]==5);
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Athens, WI",athwi45d);
  tassert(count[0]>=5164 && count[0]<=5188 && count[1]==4968);
  /* Depending on implementation, count[0] could be 5164 or 5188. The geoquads
   * in the difference are tangent at one corner to the North Pole or
   * Galápagos; the intersections computed in the overlap function include
   * two points 222 pm apart, which have the same bearing from Athens.
   */
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Ushuaia",ush4000);
  tassert(count[0]==3501 && count[1]==3275);
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Galápagos",gps5311);
  tassert(count[0]==5437 && count[1]==5157);
  ps.endpage();
  ps.startpage();
  count=plotcenters(ps,"Chamchamal",cham8000);
  tassert(count[0]==11085 && count[1]==10689);
  ps.endpage();
  ps.trailer();
  ps.close();
}

void testcylinterval()
{
  cylinterval globe,lougou,howland,udallgu,udallvi,rec0,rec60,empty;
  cylinterval alaska,lower48,allus,westnc;
  cylinterval res0,res1,res2;
  vector<cylinterval> manycyl;
  int i;
  globe.ebd=DEG180;
  globe.wbd=-DEG180;
  globe.nbd=DEG90;
  globe.sbd=-DEG90;
  howland.ebd=degtobin(-175.6);
  howland.wbd=degtobin(-177.6);
  howland.nbd=degtobin(1.8);
  howland.sbd=degtobin(-0.2);
  lougou.ebd=degtobin(3.9);
  lougou.wbd=degtobin(2.9);
  lougou.nbd=degtobin(11.6);
  lougou.sbd=degtobin(10.6);
  udallgu.ebd=862681133;
  udallgu.wbd=862681132;
  udallgu.nbd=80217794;
  udallgu.sbd=80217793;
  udallvi.ebd=-385146884;
  udallvi.wbd=-385146885;
  udallvi.nbd=105916015;
  udallvi.sbd=105916014;
  rec0.ebd=degtobin(0.1);
  rec0.wbd=degtobin(-0.1);
  rec0.nbd=degtobin(0.1);
  rec0.sbd=degtobin(-0.1);
  rec60.ebd=degtobin(0.1);
  rec60.wbd=degtobin(-0.1);
  rec60.nbd=degtobin(60.1);
  rec60.sbd=degtobin(59.9);
  empty.ebd=DEG120;
  empty.wbd=DEG120;
  empty.nbd=DEG30;
  empty.sbd=DEG30;
  alaska.ebd=1395864371;
  alaska.wbd=1026019965;
  alaska.nbd=429496730;
  alaska.sbd=292296385;
  lower48.ebd=1789569707;
  lower48.wbd=1372003442;
  lower48.nbd=345983477;
  lower48.sbd=143165577;
  allus.ebd=degtobin(300); // East of Virgin Islands
  allus.wbd=degtobin(143); // West of Guam
  allus.nbd=degtobin(72); // North of Alaska
  allus.sbd=degtobin(-17); // South of Samoa
  westnc.ebd=degtobin(-79.183333333); // Combination of Asheville, Shelby,
  westnc.wbd=degtobin(-84.216666667); // and Charlotte 150 km circles,
  westnc.nbd=degtobin(36.933333333); // rounded to the nearest minute.
  westnc.sbd=degtobin(33.883333333);
  cout<<"Area of globe "<<hnum(globe.area())<<endl;
  tassert(fabs(globe.area()-510e12)<0.1e12);
  cout<<"Area of Howland "<<hnum(howland.area())<<" Lougou "<<hnum(lougou.area())<<endl;
  res0=combine(howland,lougou);
  res1=combine(lougou,howland);
  res2=combine(res1,res0);
  tassert(res0.nbd==res1.nbd);
  tassert(res0.sbd==res1.sbd);
  tassert(res0.ebd!=res1.ebd);
  tassert(res0.wbd!=res1.wbd);
  tassert(res2.ebd==res2.wbd+DEG360);
  tassert(res0.area()==res1.area());
  tassert(fabs(res0.area()-26.3e12)<0.1e12);
  cout<<"Area of combine(howland,lougou) "<<hnum(res0.area())<<endl;
  cout<<"Area of combine(lougou,howland) "<<hnum(res1.area())<<endl;
  res0=combine(udallvi,udallgu);
  res1=combine(udallgu,udallvi);
  tassert(res0.nbd==res1.nbd);
  tassert(res0.sbd==res1.sbd);
  tassert(res0.ebd==res1.ebd);
  tassert(res0.wbd==res1.wbd);
  cout<<"Area of combine(udallvi,udallgu) "<<hnum(res0.area())<<endl;
  cout<<"Longitude interval of combine(udallvi,udallgu) "<<bintodeg(res0.ebd-res0.wbd)<<endl;
  tassert(fabs(res0.area()-7.7e12)<0.1e12);
  tassert(res0.ebd-res0.wbd==899655632);
  tassert(gap(udallgu,udallvi)==899655630);
  res0=intersect(udallgu,udallvi);
  tassert(res0.area()==0);
  cout<<"Area(rec0)/area(rec60) "<<ldecimal(rec0.area()/rec60.area())<<endl;
  tassert(fabs(rec0.area()/rec60.area()-2)<2e-6);
  res0=combine(howland,empty);
  res1=combine(empty,howland);
  tassert(res0.nbd==res1.nbd);
  tassert(res0.sbd==res1.sbd);
  tassert(res0.ebd==res1.ebd);
  tassert(res0.wbd==res1.wbd);
  tassert(res0.nbd==howland.nbd);
  tassert(res0.sbd==howland.sbd);
  tassert(res0.ebd==howland.ebd);
  tassert(res0.wbd==howland.wbd);
  res0=combine(howland,globe);
  res1=combine(globe,howland);
  tassert(res0.nbd==res1.nbd);
  tassert(res0.sbd==res1.sbd);
  tassert(res0.ebd==res1.ebd);
  tassert(res0.wbd==res1.wbd);
  tassert(res0.nbd==globe.nbd);
  tassert(res0.sbd==globe.sbd);
  tassert(res0.ebd==globe.ebd);
  tassert(res0.wbd==globe.wbd);
  cout<<"Area of Alaska "<<hnum(alaska.area())<<" Lower 48 "<<hnum(lower48.area())<<endl;
  /* These are the areas of NGS geoid files. The areas of Alaska and
   * Lower 48 are only 1.7 Mm² and 8.1 Mm², respectively.
   */
  res0=intersect(alaska,lower48);
  cout<<"Area of intersect(alaska,lower48) "<<hnum(res0.area())<<endl;
  tassert(res0.nbd=lower48.nbd);
  tassert(res0.sbd=alaska.sbd);
  tassert(res0.ebd=alaska.ebd);
  tassert(res0.wbd=lower48.wbd);
  cout<<"Area of westnc "<<hnum(westnc.area())<<endl;
  cout<<"Area of allus "<<hnum(allus.area())<<endl;
  res0=intersect(westnc,allus);
  res1=combine(westnc,allus);
  cout<<"Area of intersect(westnc,allus) "<<hnum(res0.area())<<endl;
  cout<<"Area of combine(westnc,allus) "<<hnum(res1.area())<<endl;
  tassert(res0.area()==westnc.area());
  tassert(res1.area()==allus.area());
  res0=intersect(westnc,lower48);
  res1=combine(westnc,lower48);
  cout<<"Area of intersect(westnc,lower48) "<<hnum(res0.area())<<endl;
  cout<<"Area of combine(westnc,lower48) "<<hnum(res1.area())<<endl;
  tassert(res0.area()==westnc.area());
  tassert(res1.area()==lower48.area());
  for (i=0;i<10;i++)
  {
    res0.nbd=degtobin(i)+1;
    res0.sbd=degtobin(i)-1;
    res0.wbd=degtobin(17*i+12)-1;
    res0.ebd=degtobin(17*i+12)+rng.usrandom();
    manycyl.push_back(res0);
    res0.wbd=~res0.wbd;
    res0.ebd=~res0.ebd;
    swap(res0.ebd,res0.wbd);
    manycyl.push_back(res0);
  }
  res0.ebd=DEG180+rng.usrandom()-32767;
  res0.wbd=DEG180-rng.usrandom()*8192;
  res0.sbd=degtobin(-8);
  res0.nbd=degtobin(-6);
  manycyl.push_back(res0);
  res0.wbd=~res0.wbd;
  res0.ebd=~res0.ebd;
  swap(res0.ebd,res0.wbd);
  manycyl.push_back(res0);
  res0=combine(manycyl);
  outcyl(res0);
  tassert(res0.ebd-res0.wbd==degtobin(336)+2);
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
  for (i=-40;i<=40;i++)
  {
    writegeint(geintf,i-0x1f202020);
    writegeint(geintf,i-0x202020);
    writegeint(geintf,i-0x2020);
    writegeint(geintf,i-0x20);
    writegeint(geintf,i);
    writegeint(geintf,i+0x20);
    writegeint(geintf,i+0x2020);
    writegeint(geintf,i+0x202020);
    writegeint(geintf,i+0x1f202020);
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
  for (i=-40;i<=40;i++)
  {
    j=readgeint(geintf);
    tassert(i==j+0x1f202020);
    j=readgeint(geintf);
    tassert(i==j+0x202020);
    j=readgeint(geintf);
    tassert(i==j+0x2020);
    j=readgeint(geintf);
    tassert(i==j+0x20);
    j=readgeint(geintf);
    tassert(i==j);
    j=readgeint(geintf);
    tassert(i==j-0x20);
    j=readgeint(geintf);
    tassert(i==j-0x2020);
    j=readgeint(geintf);
    tassert(i==j-0x202020);
    j=readgeint(geintf);
    tassert(i==j-0x1f202020);
  }
  cout<<dec<<nancount<<" NANs"<<endl;
  tassert(nancount==1);
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
  bool ret,listTests=false;
  if (testfail)
  {
    cout<<"failed before "<<testname<<endl;
    //sleep(2);
  }
  ret=args.size()==0;
  for (i=0;i<args.size();i++)
  {
    if (testname==args[i])
      ret=true;
    if (args[i]=="-l")
      listTests=true;
  }
  if (listTests)
    cout<<testname<<endl;
  return ret;
}

void testlooseness()
{
  double looseness,len,midlength;
  len=1e-5;
  midlength=1.8e6;
  looseness=DEG60/((len/midlength)/DBL_EPSILON*2)+1;
  cout<<looseness<<endl;
  len=200;
  midlength=1.41421356;
  looseness=DEG60/((len/midlength)/DBL_EPSILON*2)+1;
  cout<<looseness<<endl;
}

int main(int argc, char *argv[])
{
  int i;
  for (i=1;i<argc;i++)
    args.push_back(argv[i]);
  doc.pl.resize(2);
  readTmCoefficients();
  if (shoulddo("sizeof"))
    testsizeof();
  if (shoulddo("area3"))
    testarea3();
  if (shoulddo("relprime"))
    testrelprime();
  if (shoulddo("zoom"))
    testzoom();
  if (shoulddo("random"))
    testrandom(); // may take >7 s; time is random
  if (shoulddo("intersection"))
    testintersection();
  if (shoulddo("in"))
    testin();
  if (shoulddo("triangle"))
    testtriangle();
  if (shoulddo("measure"))
    testmeasure();
  if (shoulddo("matrix"))
    testmatrix();
  if (shoulddo("quaternion"))
    testquaternion();
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
    testmaketinbigaster(); // >1 s
  if (shoulddo("maketinstraightrow"))
    testmaketinstraightrow();
  if (shoulddo("maketinlongandthin"))
    testmaketinlongandthin();
  if (shoulddo("maketinlozenge"))
    testmaketinlozenge();
  if (shoulddo("maketinring"))
    testmaketinring();
  if (shoulddo("maketinwheel"))
    testmaketinwheel();
  if (shoulddo("maketinellipse"))
    testmaketinellipse();
  if (shoulddo("tripolygon"))
    testtripolygon();
  if (shoulddo("tindxf"))
    testtindxf();
  if (shoulddo("break0"))
    testbreak0();
  if (shoulddo("brent"))
    testbrent();
  if (shoulddo("newton"))
    testnewton();
  if (shoulddo("manysum"))
    testmanysum(); // >2 s
  if (shoulddo("vcurve"))
    testvcurve();
  if (shoulddo("integertrig"))
    testintegertrig();
  if (shoulddo("leastsquares"))
    testleastsquares();
  if (shoulddo("minquad"))
    testminquad();
  if (shoulddo("circle"))
    testcircle();
  if (shoulddo("segment"))
    testsegment();
  if (shoulddo("arc"))
    testarc();
  if (shoulddo("spiral"))
    testspiral();
  if (shoulddo("spiralarc"))
    testspiralarc(); // 10.5 s
  if (shoulddo("cogospiral"))
    testcogospiral();
  if (shoulddo("curly"))
    testcurly();
  if (shoulddo("curvefit"))
    testcurvefit();
  if (shoulddo("manyarc"))
    testmanyarc(); // 3 s
  if (shoulddo("closest"))
    testclosest();
  if (shoulddo("qindex"))
    testqindex();
  if (shoulddo("makegrad"))
    testmakegrad();
  if (shoulddo("derivs"))
    testderivs();
  if (shoulddo("trianglecontours"))
    trianglecontours(); // >5 s and outputs some new files each time
#ifndef NDEBUG
  if (shoulddo("parabinter"))
    testparabinter();
#endif
  if (shoulddo("rasterdraw"))
    testrasterdraw(); // 2 s
  if (shoulddo("dirbound"))
    testdirbound();
  if (shoulddo("stl"))
    teststl();
  if (shoulddo("halton"))
    testhalton(); // 2.5 s
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
  if (shoulddo("pnezd"))
    testpnezd();
  if (shoulddo("ldecimal"))
    testldecimal();
  if (shoulddo("ellipsoid"))
    testellipsoid();
  if (shoulddo("projection"))
    testprojection();
  if (shoulddo("color"))
    testcolor();
  if (shoulddo("layer"))
    testlayer();
  if (shoulddo("contour"))
    testcontour(); // >7 s
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
    testquadhash(); // 8 s
  if (shoulddo("smallcircle"))
    testsmallcircle();
  if (shoulddo("cylinterval"))
    testcylinterval();
  if (shoulddo("vball"))
    testvball();
  if (shoulddo("geoid"))
    testgeoid();
  if (shoulddo("geoidboundary"))
    testgeoidboundary(); // 45 s
  if (shoulddo("vballgeoid"))
    testvballgeoid(); // 206 s
  if (shoulddo("kml"))
    testkml(); // 19.5 s
  if (shoulddo("geint"))
    testgeint();
  //clampcubic();
  //splitcubic();
  //printf("sin(int)=%f sin(float)=%f\n",sin(65536),sin(65536.));
  //cornustats();
  //testlooseness();
  cout<<"\nTest "<<(testfail?"failed":"passed")<<endl;
  return testfail;
}
