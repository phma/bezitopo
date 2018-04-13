/******************************************************/
/*                                                    */
/* refinegeoid.cpp - refine geoid approximation       */
/*                                                    */
/******************************************************/
/* Copyright 2016,2017 Pierre Abbat.
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
#include "config.h"
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include <iostream>
#include "refinegeoid.h"
#include "hlattice.h"
#include "relprime.h"
#include "sourcegeoid.h"
#include "ldecimal.h"
using namespace std;

manysum dataArea,totalArea;
time_t progressTime;
int avgelev_interrocount=0,avgelev_refinecount=0;
histogram correctionHist(1,2);

void outProgress()
{
  cout<<"Total area "<<ldecimal(totalArea.total()*1e-12,totalArea.total()*1e-18)
    <<" Data area "<<ldecimal(dataArea.total()*1e-12,dataArea.total()*1e-18)<<"    \r";
  cout.flush();
}

void progress(geoquad &quad)
/* At the end, totalArea is 510.0645 Mm² (4*π*(6371 km)²).
 * dataArea advances more smoothly, but depends on the files read in.
 */
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
#ifdef HAVE_WINDOWS_H
  now=GetTickCount()/1024;
#else
  now=time(nullptr);
#endif
  if (now!=progressTime)
  {
    progressTime=now;
    outProgress();
  }
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
      avgelev_interrocount++;
    }
    n-=rp;
    if (n<0)
      n+=hlat.nelts;
  }
}

void refine(geoquad &quad,double vscale,double tolerance,double sublimit,double spacing,int qsz,bool allbol)
{
  int i,j=0,numnums,ncorr;
  bool biginterior,ovlp;
  double area,qpoints[16][16],sqerror,lastsqerror,maxerr;
  array<double,6> corr;
  geoquadMatch gqMatch;
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
  if (allbol)
    gqMatch=bolMatch(quad);
  if (quad.nans.size()+quad.nums.size()==0 || (quad.isfull() && area/(quad.nans.size()+quad.nums.size())>sqr(spacing)))
    interroquad(quad,spacing);
  //biginterior=area>=sqr(sublimit) && quad.isfull()>0;
  biginterior=false;
  if (biginterior)
    cout<<"big interior quad ";
  for (i=0;i<qsz;i++)
    for (j=0;j<qsz;j++)
    {
      qpt=quad.center+xy(quad.scale,0)*qscale(i,qsz)+xy(0,quad.scale)*qscale(j,qsz);
      v=vball(quad.face,qpt);
      pt=decodedir(v);
      qpoints[i][j]=avgelev(pt)/vscale;
      if (std::isfinite(qpoints[i][j]))
        quad.nums.push_back(qpt);
      else
        quad.nans.push_back(qpt);
    }
  avgelev_refinecount+=sqr(qsz);
  for (ovlp=false,i=0;!ovlp && i<excerptcircles.size();i++)
    if (overlap(excerptcircles[i],quad))
      ovlp=true;
  if (!excerptcircles.size())
    ovlp=true;
  if (quad.scale>2)
    cout<<quad.nans.size()<<" nans "<<quad.nums.size()<<" nums after"<<endl;
  j=0;
  if (ovlp)
  {
    for (numnums=i=0;i<qsz;i++)
      for (j=0;j<qsz;j++)
        if (std::isfinite(qpoints[i][j]))
          numnums++;
    j=0;
    if (numnums*2>sqr(qsz))
    {
      if (quad.isnan())
        quad.und[0]=0;
      corr=correction(quad,qpoints,qsz);
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
        lastsqerror=sqerror;
        for (i=ncorr=0;i<6;i++)
        {
          quad.und[i]+=rint(corr[i]);
          ncorr+=rint(corr[i])!=0;
        }
        corr=correction(quad,qpoints,qsz);
        for (sqerror=i=0;i<6;i++)
          sqerror+=sqr(corr[i]);
      }
    }
    //else
      //cout<<"numnums "<<numnums<<endl;
    maxerr=maxerror(quad,qpoints,qsz);
    if (biginterior)
    {
      switch (quad.isfull())
      {
        case -1:
          cout<<"empty";
          break;
        case 0:
          cout<<"partly full";
          break;
        case 1:
          cout<<"full";
      }
      cout<<" maxerror "<<maxerr*vscale<<endl;
    }
    if (area>=sqr(sublimit) && ((quad.isfull()==0 && 2*numnums<=sqr(qsz)) ||
        maxerr>tolerance/vscale))
    {
      quad.subdivide();
      for (i=0;i<4;i++)
        refine(*quad.sub[i],vscale,tolerance,sublimit,spacing,qsz,allbol);
    }
  }
  progress(quad);
  vector<xy>().swap(quad.nums); // deallocate vectors
  vector<xy>().swap(quad.nans);
  correctionHist<<j;
}
