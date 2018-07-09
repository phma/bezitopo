/******************************************************/
/*                                                    */
/* manyarc.cpp - approximate spiral with many arcs    */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include "manyarc.h"
#include "rootfind.h"
#include "manysum.h"
#include "ldecimal.h"
#include "leastsquares.h"
#include "cogospiral.h"
using namespace std;

/* Spiralarcs are used for centerlines of highways. A property line or easement
 * may be defined as a distance offset from the centerline of a highway or
 * railroad, but an offset from a spiral is hard to work with. Previously
 * surveyors have connected points on the offset with straight lines, but
 * that doesn't look very good. Instead, one should approximate the spiralarc
 * with several circular arcs and offset the arcs.
 */

double manyArcTrimFunc(double p,double n)
{
  manysum ret;
  ret-=4*p*p*p/3;
  ret+=n*p*p;
  ret+=p*p;
  ret-=n*p;
  ret+=n/6;
  return ret.total();
}

double manyArcTrimDeriv(double p,double n)
{
  manysum ret;
  ret-=4*p*p;
  ret+=n*p*2;
  ret+=p*2;
  ret-=n;
  return ret.total();
}

double manyArcTrim(unsigned n)
/* Computes the amount by which to trim [0,n] to get n segments to fit n arcs
 * to a spiralarc. Define f(x) as piecewise linear from (0,0) to (1,1) to (2,4)
 * to (3,9) and so on. Lower f(x) by about 1/6 so that there's as much area
 * above f(x) and below x² as below f(x) and above x². (It's exactly 1/6 when
 * n is infinite.) Then trim p off each end where f(x) intersects x².
 * 
 * ∫ [p,n-p] (f(x)-f(p)+p²-x²) dx = 0
 * -4p³/3+(n+1)p²-np+n/6=0
 * There are two solutions in [0,1]; we want the one in [0,1/2].
 */
{
  double p;
  Newton ne;
  p=ne.init(0,manyArcTrimFunc(0,n),manyArcTrimDeriv(0,n),0.5,manyArcTrimFunc(0.5,n),manyArcTrimDeriv(0.5,n));
  while (!ne.finished())
  {
    p=ne.step(manyArcTrimFunc(p,n),manyArcTrimDeriv(p,n));
  }
  return p;
}

vector<segment> manyQuad(segment cubic,int narcs)
/* Approximates cubic with narcs quadratics, where the first and last
 * are trimmed by the manyArcTrim function.
 */
{
  int i;
  double piecelength,thispiecelength,abscissa[5],overhang;
  double ordinate,lastordinate,startx;
  double startslope,midslope,endslope;
  double p,q,accel;
  vector<segment> ret;
  p=manyArcTrim(narcs);
  q=p*p-p+0.25;
  piecelength=cubic.length()/(narcs-2*p);
  overhang=piecelength*p;
  lastordinate=cubic.getstart().getz();
  startx=cubic.getstart().getx();
  for (i=0;i<narcs;i++)
  {
    abscissa[1]=i*piecelength+cubic.getstart().getx();
    abscissa[0]=abscissa[1]-overhang;
    abscissa[2]=abscissa[0]+piecelength/2;
    abscissa[4]=abscissa[0]+piecelength;
    abscissa[3]=abscissa[4]-overhang;
    midslope=(cubic.slope(abscissa[3]-startx)+cubic.slope(abscissa[1]-startx))/2;
    accel=(cubic.slope(abscissa[3]-startx)-cubic.slope(abscissa[1]-startx))/(abscissa[3]-abscissa[1]);
    thispiecelength=piecelength;
    if (i)
      startslope=midslope-accel*piecelength/2;
    else
    {
      startslope=midslope-accel*(abscissa[2]-abscissa[1]);
      //cout<<"startslope="<<startslope<<", should be "<<cubic.startslope()<<endl;
      thispiecelength-=overhang;
    }
    if (i<narcs-1)
      endslope=midslope+accel*piecelength/2;
    else
    {
      endslope=midslope+accel*(abscissa[3]-abscissa[2]);
      //cout<<"endslope="<<endslope<<", should be "<<cubic.endslope()<<endl;
      thispiecelength-=overhang;
    }
    ordinate=lastordinate+thispiecelength*(startslope+endslope)/2;
    ret.push_back(segment(xyz(abscissa[i==0]          ,0,lastordinate),
			  xyz(abscissa[4-(i==narcs-1)],0,    ordinate)));
    ret[i].setslope(START,startslope);
    ret[i].setslope(END,endslope);
    lastordinate=ordinate;
  }
  return ret;
}

array<int,2> ends(polyarc apx)
/* Returns the indices of the two arcs whose angle is closest to 90°.
 * For most spiralarcs in actual use, they are the first and the last.
 */
{
  int i,j,sz=apx.size(),angle,leastangle=DEG120;
  array<int,2> ret;
  vector<int> bearing;
  for (i=0;i<sz;i++)
    bearing.push_back(apx.getarc(i).chordbearing());
  for (i=0;i<sz;i++)
    for (j=0;j<i;j++)
    {
      angle=(bearing[i]-bearing[j]-DEG90)&(DEG180-1);
      if (angle>DEG45)
	angle-=DEG90;
      angle=abs(angle);
      if (angle<leastangle)
      {
	leastangle=angle;
	ret[0]=i;
	ret[1]=j;
      }
    }
  return ret;
}

double meanSquareDistance(polyarc apx,spiralarc a)
{
  int i;
  vector<double> acc;
  arc oneArc;
  for (i=0;i<apx.size();i++)
  {
    oneArc=apx.getarc(i);
    acc.push_back(meanSquareDistance(&oneArc,&a)*oneArc.length());
  }
  return pairwisesum(acc)/apx.length();
}

vector<double> weightedDistance(polyarc apx,spiralarc a)
/* The distances are weighted so that, if they are squared and totaled up,
 * the result is meanSquareDistance times the total length of apx.
 */
{
  int i,j;
  vector<double> ret;
  array<double,4> wd1;
  arc oneArc;
  for (i=0;i<apx.size();i++)
  {
    oneArc=apx.getarc(i);
    wd1=weightedDistance(&oneArc,&a);
    for (j=0;j<4;j++)
      ret.push_back(wd1[j]);
  }
  return ret;
}

polyarc manyArcUnadjusted(spiralarc a,int narcs)
{
  int sb=a.startbearing(),eb=a.endbearing();
  int i;
  double piecelength,thispiecelength,chordlength,abscissa[5],overhang;
  double startbear,midbear,endbear;
  int midbeara,midbearb;
  double p,q,cur;
  polyarc ret;
  xy thispoint=a.getstart();
  ret.insert(thispoint);
  p=manyArcTrim(narcs);
  q=p*p-p+0.25;
  piecelength=a.length()/(narcs-2*p);
  overhang=piecelength*p;
  for (i=0;i<narcs;i++)
  {
    abscissa[1]=i*piecelength;
    abscissa[0]=abscissa[1]-overhang;
    abscissa[2]=abscissa[0]+piecelength/2;
    abscissa[4]=abscissa[0]+piecelength;
    abscissa[3]=abscissa[4]-overhang;
    midbeara=a.bearing(abscissa[3]);
    midbearb=a.bearing(abscissa[1]);
    midbear=bintorad(midbeara)+bintorad(midbearb-midbeara)/2;
    cur=bintorad(midbeara-midbearb)/(abscissa[3]-abscissa[1]);
    thispiecelength=piecelength;
    if (i)
      startbear=midbear-cur*piecelength/2;
    else
    {
      startbear=midbear-cur*(abscissa[2]-abscissa[1]);
      thispiecelength-=overhang;
    }
    if (i<narcs-1)
      endbear=midbear+cur*piecelength/2;
    else
    {
      endbear=midbear+cur*(abscissa[3]-abscissa[2]);
      thispiecelength-=overhang;
    }
    if (endbear==startbear)
      chordlength=thispiecelength;
    else
      chordlength=thispiecelength*sin((endbear-startbear)/2)/((endbear-startbear)/2);
    thispoint+=chordlength*cossin((endbear+startbear)/2);
    ret.insert(thispoint);
    ret.setdelta(i,radtobin(endbear-startbear));
  }
  ret.open();
  ret.setlengths();
  return ret;
}

polyarc adjustEnds(polyarc apx,spiralarc a,int n0,int n1)
/* Adjusts the n0th and n1st arc of apx so that the ends match those of a.
 * n0 and n1 should be the two closest to right angles to each other.
 * For a spiralarc with small delta and not including its inflection point,
 * they are the first and last arcs.
 */
{
  int narcs=apx.size();
  int i;
  double along;
  vector<double> alongs;
  vector<xy> arcpoints;
  matrix arcdisp(2,2);
  xy enddiff,thispoint;
  vector<double> adjustment01,adjustment(narcs,0),shortfall;
  polyarc ret;
  for (i=0;i<=narcs;i++)
  {
    along=apx.getCumLength(i);
    arcpoints.push_back(apx.station(along));
    alongs.push_back(along);
  }
  arcdisp[0][0]=arcpoints[n0+1].getx()-arcpoints[n0].getx();
  arcdisp[1][0]=arcpoints[n0+1].gety()-arcpoints[n0].gety();
  arcdisp[0][1]=arcpoints[n1+1].getx()-arcpoints[n1].getx();
  arcdisp[1][1]=arcpoints[n1+1].gety()-arcpoints[n1].gety();
  enddiff=a.getend()-apx.station(apx.length());
  shortfall.push_back(enddiff.getx());
  shortfall.push_back(enddiff.gety());
  adjustment01=linearLeastSquares(arcdisp,shortfall);
  adjustment[n0]=adjustment01[0];
  adjustment[n1]=adjustment01[1];
  thispoint=a.getstart();
  ret.insert(thispoint);
  for (i=0;i<narcs;i++)
  {
    if (i==narcs-1 &&
        abs(foldangle(dir(thispoint,a.getend())-dir(arcpoints[i],arcpoints[i+1])))<2 &&
        dist(thispoint,a.getend())>2e9*dist(thispoint+(arcpoints[i+1]-arcpoints[i])*(1+adjustment[i]),a.getend()))
      thispoint=a.getend();
    else
      thispoint+=(arcpoints[i+1]-arcpoints[i])*(1+adjustment[i]);
    ret.insert(thispoint);
    ret.setdelta(i,apx.getarc(i).getdelta());
  }
  ret.open();
  ret.setlengths();
  return ret;
}

polyarc adjustManyArc(polyarc apx,spiralarc a)
{
  int narcs=apx.size();
  int i;
  double along;
  vector<double> alongs;
  vector<xy> arcpoints,spiralpoints;
  matrix arcdisp(2,narcs);
  xy enddiff,thispoint;
  vector<double> adjustment,shortfall;
  polyarc ret;
  for (i=0;i<=narcs;i++)
  {
    along=apx.getCumLength(i);
    arcpoints.push_back(apx.station(along));
    spiralpoints.push_back(a.station(along)); // The lengths of the curves are equal.
    alongs.push_back(along);
  }
  for (i=0;i<narcs;i++)
  {
    cout<<"Piece "<<i<<" arc length "<<ldecimal(alongs[i+1]-alongs[i])<<'\n';
    cout<<"Arc chord "<<ldecimal(dist(arcpoints[i],arcpoints[i+1]),0.001)<<' '<<ldecimal(bintodeg(dir(arcpoints[i],arcpoints[i+1])),0.0001)<<"°\n";
    cout<<"Spiral chord "<<ldecimal(dist(spiralpoints[i],spiralpoints[i+1]),0.001)<<' '<<ldecimal(bintodeg(dir(spiralpoints[i],spiralpoints[i+1])),0.0001)<<"°\n";
    arcdisp[0][i]=arcpoints[i+1].getx()-arcpoints[i].getx();
    arcdisp[1][i]=arcpoints[i+1].gety()-arcpoints[i].gety();
  }
  enddiff=a.getend()-apx.station(apx.length());
  shortfall.push_back(enddiff.getx());
  shortfall.push_back(enddiff.gety());
  adjustment=linearLeastSquares(arcdisp,shortfall);
  thispoint=a.getstart();
  ret.insert(thispoint);
  for (i=0;i<narcs;i++)
  {
    cout<<"Adjust arc "<<i<<" by "<<adjustment[i]<<endl;
    if (i==narcs-1 &&
        abs(foldangle(dir(thispoint,a.getend())-dir(arcpoints[i],arcpoints[i+1])))<2 &&
        dist(thispoint,a.getend())>2e9*dist(thispoint+(arcpoints[i+1]-arcpoints[i])*(1+adjustment[i]),a.getend()))
      thispoint=a.getend();
    else
      thispoint+=(arcpoints[i+1]-arcpoints[i])*(1+adjustment[i]);
    ret.insert(thispoint);
    ret.setdelta(i,apx.getarc(i).getdelta());
  }
  ret.open();
  ret.setlengths();
  return ret;
}

/* Next try at adjusting the arcs:
 * Use the Gaussian quadrature, but instead of squaring the distances and adding
 * them up, leave them unsquared and separate. They are the residuals for least
 * squares. This will produce 4*narcs equations in narcs-2 unknowns, which can
 * be solved by least squares. The two arcs whose chords are most nearly at
 * right angles are used to make the total displacement match that of the
 * spiralarc, while the rest of the arcs are the variables for least squares.
 */

polyarc adjust1step(polyarc apx,spiralarc a,int n0,int n1)
{
  int narcs=apx.size();
  int i,i2,j;
  double along;
  vector<double> alongs;
  arc oneArc;
  xy thispluspoint,thisminuspoint;
  vector<xy> pluspoints,minuspoints;
  vector<double> plusdists,minusdists;
  matrix sidedisp(4*narcs,narcs-2);
  polyarc plusadj,minusadj;
  for (i=i2=0;i<narcs;i++)
    if (i!=n0 && i!=n1)
    {
      thispluspoint=thisminuspoint=a.getstart();
      pluspoints.clear();
      minuspoints.clear();
      pluspoints.push_back(thispluspoint);
      minuspoints.push_back(thisminuspoint);
      for (j=0;j<narcs;j++)
      {
	oneArc=apx.getarc(j);
	if (j==i)
	{
	  thispluspoint+=(oneArc.getend()-oneArc.getstart())*1.0005;
	  thisminuspoint+=(oneArc.getend()-oneArc.getstart())*0.9995;
	}
	else
	{
	  thispluspoint+=(oneArc.getend()-oneArc.getstart());
	  thisminuspoint+=(oneArc.getend()-oneArc.getstart());
	}
	pluspoints.push_back(thispluspoint);
	minuspoints.push_back(thisminuspoint);
      }
      plusadj=minusadj=polyarc();
      plusadj.insert(pluspoints[0]);
      minusadj.insert(minuspoints[0]);
      for (j=0;j<narcs;j++)
      {
	plusadj.insert(pluspoints[j+1]);
	minusadj.insert(minuspoints[j+1]);
	plusadj.setdelta(j,apx.getarc(j).getdelta());
	minusadj.setdelta(j,apx.getarc(j).getdelta());
      }
      plusadj.open();
      minusadj.open();
      plusadj.setlengths();
      minusadj.setlengths();
      plusadj=adjustEnds(plusadj,a,n0,n1);
      minusadj=adjustEnds(minusadj,a,n0,n1);
      plusdists=weightedDistance(plusadj,a);
      minusdists=weightedDistance(minusadj,a);
      for (j=0;j<narcs;j++)
	sidedisp[j][i2]=1000*(minusdists[j]-plusdists[j]);
      i2++;
    }
}

polyarc manyArc(spiralarc a,int narcs)
{
  polyarc ret;
  ret=manyArcUnadjusted(a,narcs);
  ret=adjustManyArc(ret,a);
  return ret;
}
