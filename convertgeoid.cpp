/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
/*                                                    */
/******************************************************/
#include <iostream>
#include <ctime>
#include "geoid.h"
#include "sourcegeoid.h"
#include "document.h"
#include "raster.h"
#include "hlattice.h"
#include "relprime.h"
#include "ps.h"
#include "manysum.h"
using namespace std;

document doc;
cubemap cube;
manysum dataArea,totalArea;
time_t progressTime;

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 589824/51409
 * 4: 65536/7225
 * 5: 589824/51409
 */

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

void plotcenter(geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotcenter(*quad.sub[i]);
  else
    dot(unfold(quad.vcenter()));
}

void plotcenters()
{
  setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotcenter(cube.faces[i]);
}

void outProgress()
{
  cout<<"Total area "<<totalArea.total()*1e-12<<" Data area "<<dataArea.total()*1e-12<<"    \r";
  cout.flush();
}

void progress(geoquad &quad)
{
  double qarea;
  time_t now;
  qarea=quad.area();
  if (!quad.subdivided())
  {
    if (!quad.isnan())
      dataArea+=qarea;
    totalArea+=qarea;
  }
  now=time(nullptr);
  if (now!=progressTime)
  {
    progressTime=now;
    outProgress();
  }
}

void plotinter(geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotinter(*quad.sub[i]);
  else
  {
    setcolor(0,0,1);
    for (i=0;i<quad.nums.size();i++)
      dot(unfold(vball(quad.face,quad.nums[i])));
    setcolor(1,0,0);
    for (i=0;i<quad.nans.size();i++)
      dot(unfold(vball(quad.face,quad.nans[i])));
  }
}

void plotinters()
{
  setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotinter(cube.faces[i]);
}

/* Check the square for the presence of geoid data by interrogating it with a
 * hexagonal lattice. The size of the hexagon is sqrt(2/3) times the length
 * of the square (sqrt(1/2) to get the half diagonal of the square, sqrt(4/3)
 * to get the radius from the apothem), except for the whole face, where it
 * is (1+sqrt(1/3))/2 times the length of the square, since two sides of the
 * hexagon are parallel to two sides of the square. The process continues
 * until the entire square has been interrogated or there are at least one
 * point in nan and one point in num.
 * 
 * This procedure doesn't return anything. Use geoquad::isfull. It is possible
 * that interrogating finds a square full, but one of the 256 points used to
 * compute the coefficients is NaN.
 */
void interroquad(geoquad &quad,double spacing)
{
  xyz corner(3678298.565,3678298.565,3678298.565),ctr,xvec,yvec,tmp,pt;
  vball v;
  hvec h;
  int radius,i,n,rp;
  double qlen,hradius;
  ctr=quad.centeronearth();
  xvec=corner*ctr;
  yvec=xvec*ctr;
  xvec/=xvec.length();
  yvec/=yvec.length();
  tmp=(2+M_SQRT_3)*yvec+xvec;
  xvec-=yvec;
  yvec=tmp/tmp.length();
  xvec/=xvec.length();
  // xvec and yvec are now at 120° to match the components of hvec
  qlen=quad.length();
  if (qlen>1e7)
    hradius=qlen*(1+M_SQRT_1_3)/2;
  else if (qlen>5e6)
    hradius=qlen*0.95;
  else
    hradius=qlen*M_SQRT_2_3;
  if (spacing<1)
    spacing=1;
  radius=rint(hradius/spacing);
  if (radius>26754)
  {
    radius=26754; // largest hexagon with <2147483648 points
    spacing=hradius/radius;
  }
  hlattice hlat(radius);
  xvec*=spacing;
  yvec*=spacing;
  rp=relprime(hlat.nelts);
  for (i=n=0;i<hlat.nelts && !(quad.nums.size() && quad.nans.size());i++)
  {
    h=hlat.nthhvec(n);
    v=encodedir(ctr+h.getx()*xvec+h.gety()*yvec);
    pt=decodedir(v);
    if (quad.in(v))
    {
      if (std::isfinite(avgelev(pt)))
	quad.nums.push_back(v.getxy());
      else
	quad.nans.push_back(v.getxy());
    }
    n-=rp;
    if (n<0)
      n+=hlat.nelts;
  }
}

void refine(geoquad &quad,double vscale,double tolerance,double sublimit,double spacing)
{
  int i,j=0,numnums,ncorr;
  double area,qpoints[16][16],sqerror;
  array<double,6> corr;
  xyz pt;
  vball v;
  xy qpt;
  area=quad.apxarea();
  //cout<<"Area: exact "<<quad.area()<<" approx "<<area<<" ratio "<<quad.area()/area<<endl;
  if (quad.scale>2)
  {
    cout<<"face "<<quad.face<<" ctr "<<quad.center.getx()<<','<<quad.center.gety()<<endl;
    cout<<quad.nans.size()<<" nans "<<quad.nums.size()<<" nums before"<<endl;
  }
  if (quad.nans.size()+quad.nums.size()==0 || (quad.isfull() && area/(quad.nans.size()+quad.nums.size())>sqr(spacing)))
    interroquad(quad,spacing);
  if (area<sqr(sublimit) || quad.isfull())
    for (i=0;i<16;i++)
      for (j=0;j<16;j++)
      {
	qpt=quad.center+xy(quad.scale,0)*(i-7.5)/8+xy(0,quad.scale)*(j-7.5)/8;
	v=vball(quad.face,qpt);
	pt=decodedir(v);
	qpoints[i][j]=avgelev(pt)/vscale;
	if (std::isfinite(qpoints[i][j]))
	  quad.nums.push_back(qpt);
	else
	  quad.nans.push_back(qpt);
      }
  if (quad.scale>2)
    cout<<quad.nans.size()<<" nans "<<quad.nums.size()<<" nums after"<<endl;
  if (area<sqr(sublimit) || quad.isfull()!=0)
  {
    for (numnums=i=0;i<16;i++)
      for (j=0;j<16;j++)
	if (std::isfinite(qpoints[i][j]))
	  numnums++;
    if (numnums>127)
    {
      if (quad.isnan())
	quad.und[0]=0;
      corr=correction(quad,qpoints);
      for (sqerror=i=0;i<6;i++)
	sqerror+=sqr(corr[i]);
      //cout<<"numnums "<<numnums<<" sqerror "<<sqerror<<" before ";
      /* The program can get into an infinite loop in which one of the corrections
       * is 0.5 and another is -0.5, then on the next iteration one is -0.5 and
       * another is 0.5, and so on forever. This happens with the USGS data
       * at point (3,-0.58112335205078125,-0.69258880615234375), which is near
       * the overlap of the Alaska and Lower 48 files, but not at the edge of
       * either. It takes two iterations to arrive at the infinite loop. But if
       * only half of the points have known undulation, it can take as many as
       * 1498 iterations to converge, so stop the loop at 1536.
       */
      for (j=0,ncorr=6;ncorr && j<1536;j++)
      {
	for (i=ncorr=0;i<6;i++)
	{
	  quad.und[i]+=rint(corr[i]);
	  ncorr+=rint(corr[i])!=0;
	}
	corr=correction(quad,qpoints);
	for (sqerror=i=0;i<6;i++)
	  sqerror+=sqr(corr[i]);
      }
      //cout<<sqerror<<" after"<<endl;
    }
    //else
      //cout<<"numnums "<<numnums<<endl;
  }
  if (area>=sqr(sublimit) && (quad.isfull()==0 || maxerror(quad,qpoints)>tolerance/vscale))
  {
    quad.subdivide();
    for (i=0;i<4;i++)
      refine(*quad.sub[i],vscale,tolerance,sublimit,spacing);
  }
  progress(quad);
}

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
  cout<<"c: "<<cube.undulation(lat,lon)<<endl;
}

int main(int argc, char *argv[])
{
  int i;
  cube.scale=1/65536.;
  geo.resize(6);
  readusngsbin(geo[0],"../g2012bu0.bin");
  readusngsbin(geo[1],"../g2012ba0.bin");
  readusngsbin(geo[2],"../g2012bh0.bin");
  readusngsbin(geo[3],"../g2012bg0.bin");
  readusngsbin(geo[4],"../g2012bp0.bin");
  readusngsbin(geo[5],"../g2012bs0.bin");
  drawglobecube(1024,62,-7,1,0,"geoid.ppm");
  for (i=0;i<6;i++)
  {
    //cout<<"Face "<<i+1;
    //cout.flush();
    interroquad(cube.faces[i],3e5);
    /*if (cube.faces[i].isfull()>=0)
      cout<<" has data"<<endl;
    else
      cout<<" is empty"<<endl;*/
    refine(cube.faces[i],cube.scale,0.1,3000,1e5);
  }
  outProgress();
  cout<<endl;
  outund("Green Hill",degtobin(35.4),degtobin(-82.05));
  outund("Charlotte",degtobin(35.22),degtobin(-80.84));
  outund("Kitimat",degtobin(54.0547),degtobin(-128.6578)); // in the overlap of two files
  outund("Denali",degtobin(63.0695),degtobin(-151.0074));
  outund("Haleakala",degtobin(20.7097),degtobin(-156.2533));
  psopen("geoid.ps");
  psprolog();
  startpage();
  plotcenters();
  endpage();
  //startpage();
  //plotinters();
  //endpage();
  pstrailer();
  psclose();
  return 0;
}
