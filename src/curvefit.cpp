/******************************************************/
/*                                                    */
/* curvefit.cpp - fit polyarc/alignment to points     */
/*                                                    */
/******************************************************/
/* Copyright 2022-2024 Pierre Abbat.
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
#include "curvefit.h"
#include "manysum.h"
#include "csv.h"
#include "ldecimal.h"
#include "leastsquares.h"

using namespace std;

int fitDir=0;
/* fitDir is rotated by PHIQUARTER (145.623°) and the endpoints, except the
 * first and last which are moved along their lines, are moved along lines
 * either parallel or perpendicular to fitDir, whichever is closest to
 * sideways to the polyarc. This avoids moving an endpoint along the polyarc,
 * which results in little change, and none if the curvatures are equal.
 */

void stepDir()
{
  fitDir+=PHIQUARTER;
}

double FitRec::shortDist(Circle startLine,Circle endLine) const
/* Returns a distance less than (unless there're only two endpoints) the
 * shortest distance between successive endpoints. This is used to convert
 * length adjustments to angle adjustments. The distance is the inverse
 * of the sum of the inverses of distances.
 */
{
  int i;
  vector<double> inv;
  if (endpoints.size())
  {
    inv.push_back(1/dist(startLine.station(startOff),endpoints[0]));
    for (i=0;i<endpoints.size()-1;i++)
      inv.push_back(1/dist(endpoints[i],endpoints[i+1]));
    inv.push_back(1/dist(endpoints.back(),endLine.station(endOff)));
  }
  else
    inv.push_back(1/dist(startLine.station(startOff),endLine.station(endOff)));
  return 1/pairwisesum(inv);
}

bool FitRec::isnan() const
{
  bool ret=::isnan(startOff) || ::isnan(endOff);
  int i;
  for (i=0;i<endpoints.size() && !ret;i++)
    ret=ret || endpoints[i].isnan();
  return ret;
}

void FitRec::breakArcs(set<int> which,polyarc apx)
{
  set<int>::reverse_iterator i;
  for (i=which.rbegin();i!=which.rend();i++)
    endpoints.insert(endpoints.begin()+*i,apx.getarc(*i).midpoint());
}

double diff(const FitRec &a,const FitRec &b,Circle startLine,Circle endLine)
/* Returns the root-sum-square of the distances between corresponding
 * endpoints, plus the difference in start bearing converted to distances.
 * a and b must have the same number of endpoints. Assumes that their
 * start and end lines are the same.
 */
{
  vector<double> sq;
  int i;
  sq.push_back(sqr(bintorad(foldangle(a.startBear-b.startBear))*a.shortDist(startLine,endLine)));
  sq.push_back(sqr(bintorad(foldangle(a.startBear-b.startBear))*b.shortDist(startLine,endLine)));
  assert(a.endpoints.size()==b.endpoints.size());
  for (i=0;i<a.endpoints.size();i++)
    sq.push_back(sqr(dist(a.endpoints[i],b.endpoints[i])));
  sq.push_back(sqr(a.startOff-b.startOff));
  sq.push_back(sqr(a.endOff-b.endOff));
  return sqrt(pairwisesum(sq));
}

FitRec initialCurve(deque<Circle> lines,int pieces,PostScript &ps,BoundRect &br)
{
  int i,j;
  int bear0,bear1;
  FitRec ret;
  spiralarc spi;
  polyarc apx;
  assert(lines.size()>1);
  if (pieces<2)
    pieces=2;
  if (ps.isOpen())
  {
    ps.startpage();
    ps.setscale(br);
  }
  for (i=0;i<lines.size()-1;i++)
  {
    spi=spiralarc(lines[i].station(0),lines[i+1].station(0));
    bear0=lines[i].bearing(0)+DEG90;
    bear1=lines[i+1].bearing(0)+DEG90;
    spi.setdelta(bear1-bear0,bear1+bear0-2*spi.chordbearing());
    apx=manyArc(spi,pieces);
    if (ps.isOpen())
    {
      ps.setcolor(0,0,1);
      ps.spline(spi.approx3d(0.001/ps.getscale()));
      ps.setcolor(0,0,0);
      ps.spline(apx.approx3d(0.001/ps.getscale()));
    }
    for (j=0;j<pieces;j++)
      ret.endpoints.push_back(apx.getEndpoint(j+1));
    if (i==0)
    {
      ret.startBear=bear0;
      ret.startCur=apx.getarc(0).curvature(0);
    }
  }
  if (ps.isOpen())
    ps.endpage();
  ret.endpoints.pop_back();
  ret.startOff=ret.endOff=0;
  return ret;
}

vector<double> curvefitResiduals(polyarc q,vector<xy> points)
/* The points must not be off the ends of q.
 */
{
  vector<double> ret;
  int i,bear;
  double along;
  for (i=0;i<points.size();i++)
  {
    along=q.closest(points[i]);
    bear=q.bearing(along)+DEG90;
    ret.push_back(distanceInDirection(q.station(along),points[i],bear));
  }
  return ret;
}

double curvefitSquareError(polyarc q,vector<xy> points)
{
  vector<double> resid=curvefitResiduals(q,points);
  int i;
  for (i=0;i<resid.size();i++)
    resid[i]*=resid[i];
  return pairwisesum(resid);
}

double curvefitMaxError(polyarc q,vector<xy> points)
{
  vector<double> resid=curvefitResiduals(q,points);
  int i;
  double maxerr=0;
  for (i=0;i<resid.size();i++)
    if (fabs(resid[i])>maxerr)
      maxerr=fabs(resid[i]);
  return maxerr;
}

vector<int> closestPieces(polyline &p,vector<xy> points)
{
  vector<int> ret;
  int i;
  ret.resize(points.size());
  for (i=0;i<points.size();i++)
    ret[i]=p.stationSegment(p.closest(points[i]));
  return ret;
}

set<int> breakWhich(polyarc q,vector<xy> points)
/* Returns one or two indices of arc to break, those that have the points
 * with the worst errors.
 */
{
  set<int> ret;
  vector<double> resid=curvefitResiduals(q,points);
  vector<int> cp=closestPieces(q,points);
  map<int,int> cpCount;
  int i,negWorst=-1,posWorst=-1;
  double worstPos=-INFINITY,worstNeg=INFINITY;
  for (i=0;i<cp.size();i++)
    cpCount[cp[i]]++;
  for (i=0;i<resid.size();i++)
  {
    if (resid[i]>0 && resid[i]>worstPos && cpCount[cp[i]]>2)
    {
      worstPos=resid[i];
      posWorst=i;
    }
    if (resid[i]<0 && resid[i]<worstNeg && cpCount[cp[i]]>1)
    {
      worstNeg=resid[i];
      negWorst=i;
    }
  }
  if (posWorst>=0)
    ret.insert(cp[posWorst]);
  if (negWorst>=0)
    ret.insert(cp[negWorst]);
  return ret;
}

polyarc arcFitApprox(Circle startLine,FitRec fr,Circle endLine)
{
  polyarc ret;
  int i,bear=fr.startBear,lastbear;
  xy startPoint=startLine.station(fr.startOff);
  xy endPoint=endLine.station(fr.endOff);
  ret.insert(startPoint);
  for (i=0;i<fr.endpoints.size();i++)
  {
    lastbear=bear;
    bear=twicedir(i?fr.endpoints[i-1]:startPoint,fr.endpoints[i])-bear;
    ret.insert(fr.endpoints[i]);
    ret.setdelta(i,bear-lastbear);
  }
  lastbear=bear;
  bear=twicedir(i?fr.endpoints[i-1]:startPoint,endPoint)-bear;
  ret.insert(endPoint);
  ret.setdelta(i,bear-lastbear);
  ret.open();
  ret.setlengths();
  return ret;
}

vector<int> adjustDirs(polyarc apx,int fitDir)
{
  int i,endbear;
  vector<int> ret;
  for (i=0;i<apx.size()-1;i++)
  {
    endbear=apx.getarc(i).endbearing();
    if (isinsector(endbear-fitDir,0xc3c3c3c3))
      ret.push_back(fitDir+DEG90);
    else
      ret.push_back(fitDir);
  }
  return ret;
}

FitRec adjust1step(vector<xy> points,Circle startLine,FitRec fr,Circle endLine,bool twoD)
{
  int i,j,sz=fr.endpoints.size(),d=twoD+1;
  vector<double> adjustment;
  vector<double> resid;
  vector<double> plusresid,minusresid;
  polyarc apx=arcFitApprox(startLine,fr,endLine);
  vector<int> adjdirs=adjustDirs(apx,fitDir);
  double shortDist=fr.shortDist(startLine,endLine);
  double h=shortDist*bintorad(FURMAN1);
  double maxadj=0;
  vector<xy> hxy,hyx;
  FitRec plusoffsets,minusoffsets,ret;
  matrix sidedefl(points.size(),sz*d+3);
  for (i=0;i<sz;i++)
  {
    hxy.push_back(cossin(adjdirs[i])*h);
    hyx.push_back(cossin(adjdirs[i]+DEG90)*h);
  }
  for (i=0;i<sz*d+3;i++)
  {
    plusoffsets.endpoints.clear();
    minusoffsets.endpoints.clear();
    if (i==0)
    {
      plusoffsets.startOff=fr.startOff+h;
      minusoffsets.startOff=fr.startOff-h;
    }
    else
      plusoffsets.startOff=minusoffsets.startOff=fr.startOff;
    for (j=1;j<sz+1;j++)
    {
      if (j==i)
      {
	plusoffsets.endpoints.push_back(fr.endpoints[j-1]+hxy[j-1]);
	minusoffsets.endpoints.push_back(fr.endpoints[j-1]-hxy[j-1]);
      }
      else if (twoD && j+sz==i)
      {
	plusoffsets.endpoints.push_back(fr.endpoints[j-1]+hyx[j-1]);
	minusoffsets.endpoints.push_back(fr.endpoints[j-1]-hyx[j-1]);
      }
      else
      {
	plusoffsets.endpoints.push_back(fr.endpoints[j-1]);
	minusoffsets.endpoints.push_back(fr.endpoints[j-1]);
      }
    }
    if (i==sz*d+1)
    {
      plusoffsets.endOff=fr.endOff+h;
      minusoffsets.endOff=fr.endOff-h;
    }
    else
      plusoffsets.endOff=minusoffsets.endOff=fr.endOff;
    if (i==sz*d+2)
    {
      plusoffsets.startBear=fr.startBear+FURMAN1;
      minusoffsets.startBear=fr.startBear-FURMAN1;
    }
    else
      plusoffsets.startBear=minusoffsets.startBear=fr.startBear;
    plusresid=curvefitResiduals(arcFitApprox(startLine,plusoffsets,endLine),points);
    minusresid=curvefitResiduals(arcFitApprox(startLine,minusoffsets,endLine),points);
    for (j=0;j<plusresid.size();j++)
      sidedefl[j][i]=plusresid[j]-minusresid[j];
  }
  resid=curvefitResiduals(arcFitApprox(startLine,fr,endLine),points);
  adjustment=linearLeastSquares(sidedefl,resid);
  // Limit the adjustment to 4096 furmans (22.5°) to keep close to linear.
  for (i=0;i<adjustment.size();i++)
    if (fabs(adjustment[i])>maxadj)
      maxadj=fabs(adjustment[i]);
  for (i=0;maxadj>4096 && i<adjustment.size();i++)
    adjustment[i]*=4096/maxadj;
  ret.startOff=fr.startOff-h*adjustment[0];
  ret.endOff=fr.endOff-h*adjustment[sz*d+1];
  ret.startBear=fr.startBear-lrint(adjustment[sz*d+2]*FURMAN1);
  for (i=0;i<sz;i++)
    if (twoD)
      ret.endpoints.push_back(fr.endpoints[i]-hxy[i]*adjustment[i+1]-hyx[i]*adjustment[i+sz+1]);
    else
      ret.endpoints.push_back(fr.endpoints[i]-hxy[i]*adjustment[i+1]);
  return ret;
}

FitRec adjustArcs(vector<xy> points,Circle startLine,FitRec fr,Circle endLine)
/* Adjusts the polyarc defined by startLine, fr, and endLine until the maximum
 * error stops getting better.
 */
{
  double lastError=INFINITY,thisError=1e100;
  FitRec lastfr;
  polyarc apx;
  int i=0,j=0;
  while (j<3 || i<5)
  {
    lastfr=fr;
    /* When i=0, there is often a just-split arc, where moving the new
     * endpoint along the arc produces no effect, so the matrix is singular,
     * so do a one-dimensional adjustment first.
     */
    fr=adjust1step(points,startLine,fr,endLine,(i&255)>0);
    if (fr.isnan()) // singular matrix
      fr=adjust1step(points,startLine,lastfr,endLine,false);
    if (fr.isnan()) // something went wrong
    {
      cerr<<"Adjustment is NaN\n";
      fr=lastfr;
    }
    stepDir();
    apx=arcFitApprox(startLine,fr,endLine);
    lastError=thisError;
    thisError=curvefitMaxError(apx,points);
    if (thisError>=lastError)
      j++;
    i++;
  }
  if (thisError>lastError)
    fr=lastfr;
  return fr;
}

polyarc fitPolyarc(Circle startLine,vector<xy> points,Circle endLine,double toler,deque<Circle> hints,int pieces)
{
  int i,j;
  double maxerr=INFINITY;
  FitRec fr,lastfr;
  PostScript ps;
  BoundRect br;
  polyarc apx;
  set<int> breaks;
  hints.push_front(startLine);
  hints.push_back(endLine);
  ps.open("fitPolyarc.ps");
  if (ps.isOpen())
  {
    ps.setpaper(papersizes["A4 landscape"],0);
    ps.prolog();
  }
  for (i=0;i<points.size();i++)
    br.include(points[i]);
  fr=initialCurve(hints,pieces,ps,br);
  /* The number of degrees of freedom is twice the number of endpoints plus 3.
   * The number of points cannot be less than this.
   */
  for (i=0;maxerr>toler;i++)
  {
    lastfr=fr;
    fr=adjustArcs(points,startLine,fr,endLine);
    apx=arcFitApprox(startLine,fr,endLine);
    if (ps.isOpen())
    {
      ps.startpage();
      ps.setscale(br);
      ps.setcolor(0,0,0);
      for (j=0;j<points.size();j++)
	ps.circle(points[j],0.5/ps.getscale());
      ps.setcolor(0,0,1);
      ps.spline(apx.approx3d(0.001/ps.getscale()));
      for (j=0;j<fr.endpoints.size();j++)
	ps.circle(fr.endpoints[j],0.5/ps.getscale());
      ps.endpage();
    }
    cout<<apx.size()<<" arcs\n";
    maxerr=curvefitMaxError(apx,points);
    if (maxerr>toler)
    {
      breaks=breakWhich(apx,points);
      if (fr.endpoints.size()+breaks.size()+3>points.size())
	break;
      else
	fr.breakArcs(breaks,apx);
    }
  }
  return apx;
}
