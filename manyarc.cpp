/******************************************************/
/*                                                    */
/* manyarc.cpp - approximate spiral with many arcs    */
/*                                                    */
/******************************************************/
/* Copyright 2018-2020 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <cassert>
#include <array>
#include "manyarc.h"
#include "rootfind.h"
#include "manysum.h"
#include "ldecimal.h"
#include "random.h"
#include "leastsquares.h"
#include "cogospiral.h"

#define METHOD 1
/* Method 1: Adjust the length of each arc so that the sum of their displacements
 * equals the displacement of the spiralarc.
 * Method 2: Adjust the lengths of all but two of the arcs to minimize the squared
 * distance from the spiralarcs. The two arcs are used to make the sum of the
 * displacements equal to the displacement of the spiralarc.
 * Method 3: Adjust the ends of the arcs along lines perpendicular to the
 * spiralarc so that the bearing at the end matches that of the spiralarc.
 */

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

double maxErrorCubic(int narcs)
/* Returns the maximum error of a piecewise quadratic approximation to a cubic
 * whose throw is 1. This should be close to the maximum error of a piecewise
 * circular approximation to a spiralarc with the same throw.
 */
{
  segment cubic(xyz(0,0,0),0,0,xyz(1,0,4));
  vector<segment> approx;
  segment diff;
  double firstlength;
  vector<double> vex;
  approx=manyQuad(cubic,narcs);
  firstlength=approx[0].length();
  diff=segment(xyz(0,0,0),xyz(firstlength,0,cubic.elev(firstlength)-approx[0].getend().elev()));
  diff.setslope(START,0);
  diff.setslope(END,cubic.slope(firstlength)-approx[0].endslope());
  vex=diff.vextrema(false);
  return -diff.station(vex[0]).elev();
}

segment spiralToCubic(spiralarc a)
/* Returns a cubic with horizontal length equal to the length, slope equal
 * to the bearing, second derivative equal to the curvature, and third
 * derivative equal to the clothance of the given spiralarc.
 */
{
  double length=a.length();
  double curvature=a.curvature(length/2);
  double clothance=a.clothance();
  double midslope=bintorad(a.bearing(length/2));
  double startz=-cub(length)*clothance/48+sqr(length)*curvature/8-length*midslope/2;
  double endz=cub(length)*clothance/48+sqr(length)*curvature/8+length*midslope/2;
  segment ret(xyz(-length/2,0,startz),xyz(length/2,0,endz));
  ret.setslope(START,sqr(length)*clothance/8-length*curvature/2+midslope);
  ret.setslope(END,sqr(length)*clothance/8+length*curvature/2+midslope);
  return ret;
}

vector<Circle> crossLines(spiralarc &a,vector<segment> &q)
/* q is the piecewise quadratic approximation to the cubic corresponding to a.
 * Returns a vector of lines that cross a perpendicularly. The arc distances
 * between them equal the horizontal lengths of the elements of q.
 */
{
  int i;
  double along;
  vector<Circle> ret;
  for (i=0;i<q.size();i++)
  {
    along=q[i].getstart().getx()-q[0].getstart().getx();
    ret.push_back(Circle(a.station(along),a.bearing(along)+DEG90));
  }
  ret.push_back(Circle(a.getend(),a.endbearing()+DEG90));
  return ret;
}

vector<double> offsets(segment &a,vector<segment> &q)
/* q is the piecewise quadratic approximation to a, the cubic corresponding
 * to a spiralarc. Returns the offsets of the endpoints of the elements of q
 * from a. These are used to offset points from the spiralarc along crossLines;
 * the points must then be adjusted. The 0th and last numbers returned are 0.
 */
{
  int i;
  double along;
  vector<double> ret;
  for (i=0;i<q.size();i++)
  {
    along=q[i].getstart().getx()-q[0].getstart().getx();
    ret.push_back(q[i].getstart().getz()-a.station(along).getz());
  }
  ret.push_back(0);
  return ret;
}

vector<xyz> pointSeq(vector<Circle> &cl,vector<double> &os)
{
  int i;
  vector<xyz> ret;
  assert(cl.size()==os.size());
  for (i=0;i<cl.size();i++)
    ret.push_back(cl[i].station(os[i]));
  return ret;
}

int endDirectionError(spiralarc &a,const vector<xyz> &ps)
/* Starts at the startbearing of a, and returns how much a smooth polyarc
 * would differ from the endbearing of a, without actually drawing arcs.
 */
{
  int i,bear=a.startbearing();
  for (i=1;i<ps.size();i++)
    bear=twicedir(xy(ps[i-1]),xy(ps[i]))-bear;
  return bear-a.endbearing();
}

polyarc manyArcApprox3(spiralarc a,vector<Circle> lines,vector<double> offs)
{
  polyarc ret;
  vector<xyz> ps=pointSeq(lines,offs);
  int i,bear=a.startbearing(),lastbear;
  ret.insert(a.getstart());
  for (i=1;i<ps.size();i++)
  {
    lastbear=bear;
    bear=twicedir(xy(ps[i-1]),xy(ps[i]))-bear;
    ret.insert(ps[i]);
    ret.setdelta(i-1,bear-lastbear);
  }
  ret.open();
  ret.setlengths();
  return ret;
}

vector<double> adjust1step3(spiralarc a,vector<Circle> lines,vector<double> offs)
{
  int nlines=lines.size();
  int i,j,ederr;
  double randmul;
  double sidepull=a.length()/(nlines-1)/2e3;
  vector<double> plusoffsets,minusoffsets,adjustment,ret;
  vector<double> deflection;
  matrix sidedefl(1,nlines-2);
  for (i=1;i<nlines-1;i++)
  {
    plusoffsets.clear();
    minusoffsets.clear();
    for (j=0;j<nlines;j++)
      if (j==i)
      {
	plusoffsets.push_back(offs[j]+sidepull);
	minusoffsets.push_back(offs[j]-sidepull);
      }
      else
      {
	plusoffsets.push_back(offs[j]);
	minusoffsets.push_back(offs[j]);
      }
    sidedefl[0][i-1]=(endDirectionError(a,pointSeq(lines,plusoffsets))-
		      endDirectionError(a,pointSeq(lines,minusoffsets)))/sidepull/2;
  }
  ederr=endDirectionError(a,pointSeq(lines,offs));
  deflection.push_back(ederr);
  adjustment=minimumNorm(sidedefl,deflection);
  ret.push_back(offs[0]);
  if (ederr>-3 && ederr<3)
  for (i=1;i<nlines-1;i++)
  {
    randmul=(rng.ucrandom()+0.5)/256;
    ret.push_back(offs[i]-adjustment[i-1]*randmul);
  }
  else
  for (i=1;i<nlines-1;i++)
    ret.push_back(offs[i]-adjustment[i-1]);
  ret.push_back(offs.back());
  return ret;
}

vector<double> adjustManyArc3(spiralarc a,vector<Circle> lines,vector<double> offs)
{
  int i,err;
  for (i=0;i<55;i++)
  {
    err=endDirectionError(a,pointSeq(lines,offs));
    if (err==0)
      break;
    offs=adjust1step3(a,lines,offs);
  }
  return offs;
}

array<int,2> ends(polyarc apx)
/* Returns the indices of the two arcs whose angle is closest to 90°.
 * For most spiralarcs in actual use, they are the first and the last.
 * If apx is straight, returns {-1,-1}.
 */
{
  int i,j,sz=apx.size(),angle,leastangle=DEG90-1;
  array<int,2> ret;
  vector<int> bearing;
  ret[0]=ret[1]=-1;
  for (i=0;i<sz;i++)
    bearing.push_back(apx.getarc(i).chordbearing());
  for (i=0;i<sz;i++)
    for (j=0;j<i;j++)
    {
      angle=(bearing[i]-bearing[j]-DEG90)&(DEG180-1);
      if (angle>DEG90)
	angle-=DEG180;
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

polyarc twoArcs(spiralarc a)
/* When narcs=2 and the inflection point is close to the midpoint of the
 * spiralarc, manyArcUnadjusted produces a polyarc for which adjustment is
 * numerically unstable, producing arcs with negative length or failing to
 * adjust. twoArcs constructs one arc whose delta equals the spiralarc's
 * first delta, then constructs two arcs tangent to the ends of the spiralarc,
 * meeting tangently in the middle of the first arc.
 */
{
  arc frame(a.getstart(),a.getend(),a.getdelta());
  polyarc ret;
  int midbear;
  ret.insert(frame.getstart());
  ret.insert(frame.midpoint());
  ret.insert(frame.getend());
  ret.open();
  midbear=2*a.bearing(a.length()/2)-a.chordbearing();
  ret.setdelta(0,midbear-a.startbearing());
  ret.setdelta(1,a.endbearing()-midbear);
  ret.setlengths();
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
 * they are the first and last arcs. If n0 and n1 are both -1, which means
 * that apx is straight, it just adjusts the end.
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
  if (n0!=n1)
  {
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
  }
  thispoint=a.getstart();
  ret.insert(thispoint);
  for (i=0;i<narcs;i++)
  {
    if (i==narcs-1 &&
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

polyarc adjustManyArc1(polyarc apx,spiralarc a)
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
  adjustment=minimumNorm(arcdisp,shortfall);
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

polyarc adjust1step2(polyarc apx,spiralarc a,int n0,int n1,double &adjsq)
{
  int narcs=apx.size();
  int i,i2,j;
  double along;
  vector<double> alongs;
  arc oneArc;
  xy thispluspoint,thisminuspoint,thispoint;
  vector<xy> pluspoints,minuspoints;
  vector<double> plusdists,minusdists,apxdists,adjustment;
  matrix sidedisp(4*narcs,narcs-2);
  polyarc plusadj,minusadj,ret;
  for (i=i2=0;i<narcs && n0+n1>0;i++)
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
      for (j=0;j<4*narcs;j++)
	sidedisp[j][i2]=1000*(minusdists[j]-plusdists[j]);
      i2++;
    }
  if (n0!=n1)
  {
    apxdists=weightedDistance(apx,a);
    adjustment=linearLeastSquares(sidedisp,apxdists);
    thispoint=a.getstart();
    ret.insert(thispoint);
    for (i=i2=0;i<narcs;i++)
    {
      oneArc=apx.getarc(i);
      if (i!=n0 && i!=n1)
	thispoint+=(oneArc.getend()-oneArc.getstart())*(1+adjustment[i2++]);
      else
	thispoint+=(oneArc.getend()-oneArc.getstart());
      ret.insert(thispoint);
      ret.setdelta(i,apx.getarc(i).getdelta());
    }
    ret.open();
    ret.setlengths();
  }
  else
    ret=apx;
  ret=adjustEnds(ret,a,n0,n1);
  for (i=0;i<adjustment.size();i++)
    adjustment[i]*=adjustment[i];
  adjsq=pairwisesum(adjustment);
  return ret;
}

polyarc adjustManyArc2(polyarc apx,spiralarc a)
{
  double adjsq;
  int i=0;
  array<int,2> theEnds=ends(apx);
  do
  {
    apx=adjust1step2(apx,a,theEnds[0],theEnds[1],adjsq);
    i++;
  } while (i<5 && adjsq>1e-7);
  return apx;
}

polyarc manyArc(spiralarc a,int narcs)
{
#if METHOD==1
  polyarc ret;
  ret=manyArcUnadjusted(a,narcs);
  ret=adjustManyArc1(ret,a);
#endif
#if METHOD==2
  polyarc ret;
  if (narcs>2)
  {
    ret=manyArcUnadjusted(a,narcs);
    ret=adjustManyArc2(ret,a);
  }
  else
    ret=twoArcs(a);
#endif
#if METHOD==3
  polyarc ret;
  segment cubic=spiralToCubic(a);
  vector<segment> quads=manyQuad(cubic,narcs);
  vector<Circle> lines=crossLines(a,quads);
  vector<double> offs=offsets(cubic,quads);
  offs=adjustManyArc3(a,lines,offs);
  ret=manyArcApprox3(a,lines,offs);
#endif
  return ret;
}

double maxError(polyarc apx,spiralarc a)
/* Returns the maximum error (always positive) of approximation apx to
 * spiralarc a. It checks only the first and last arcs, because the others
 * always have less error.
 */
{
  int i;
  double firstError=0,lastError=0;
  vector<alosta> beside;
  const double fraction=10/17.;
  arc oneArc;
  oneArc=apx.getarc(0);
  //beside=besidement1(&oneArc,oneArc.length()*fraction,&a,oneArc.length()*fraction);
  beside=besidement2(oneArc,a);
  for (i=0;i<beside.size();i+=2)
  {
    lastError=dist(beside[i].station,beside[i+1].station);
    if (lastError>firstError)
      swap(firstError,lastError);
  }
  oneArc=apx.getarc(apx.size()-1);
  //beside=besidement1(&oneArc,oneArc.length()*(1-fraction),&a,a.length()-oneArc.length()*fraction);
  beside=besidement2(oneArc,a);
  for (i=0;i<beside.size();i+=2)
  {
    lastError=dist(beside[i].station,beside[i+1].station);
    if (lastError>firstError)
      swap(firstError,lastError);
  }
  return firstError;
}
