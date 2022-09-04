/******************************************************/
/*                                                    */
/* curvefit.cpp - fit polyarc/alignment to points     */
/*                                                    */
/******************************************************/
/* Copyright 2022 Pierre Abbat.
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
#include "ps.h"
#include "manysum.h"
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

FitRec initialCurve(std::vector<Circle> lines,int pieces)
{
  int i,j;
  int bear0,bear1;
  FitRec ret;
  spiralarc spi;
  polyarc apx;
  assert(lines.size()>1);
  if (pieces<2)
    pieces=2;
  for (i=0;i<lines.size()-1;i++)
  {
    spi=spiralarc(lines[i].station(0),lines[i+1].station(0));
    bear0=lines[i].bearing(0)+DEG90;
    bear1=lines[i+1].bearing(0)+DEG90;
    spi.setdelta(bear1-bear0,bear1+bear0-2*spi.chordbearing());
    apx=manyArc(spi,pieces);
    for (j=0;j<pieces;j++)
      ret.endpoints.push_back(apx.getEndpoint(j+1));
    if (i==0)
    {
      ret.startBear=bear0;
      ret.startCur=apx.getarc(0).curvature(0);
    }
  }
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

FitRec adjust1step(vector<xy> points,Circle startLine,FitRec fr,Circle endLine)
{
  int i,j,sz=fr.endpoints.size();
  vector<double> adjustment;
  vector<double> resid;
  vector<double> plusresid,minusresid;
  polyarc apx=arcFitApprox(startLine,fr,endLine);
  vector<int> adjdirs=adjustDirs(apx,fitDir);
  double shortDist=fr.shortDist(startLine,endLine);
  double h=shortDist*bintorad(FURMAN1);
  double maxadj=0;
  vector<xy> hxy;
  FitRec plusoffsets,minusoffsets,ret;
  matrix sidedefl(points.size(),sz+3);
  for (i=0;i<sz;i++)
    hxy.push_back(cossin(adjdirs[i])*h);
  for (i=0;i<sz+3;i++)
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
      else
      {
	plusoffsets.endpoints.push_back(fr.endpoints[j-1]);
	minusoffsets.endpoints.push_back(fr.endpoints[j-1]);
      }
    }
    if (i==sz+1)
    {
      plusoffsets.endOff=fr.endOff+h;
      minusoffsets.endOff=fr.endOff-h;
    }
    else
      plusoffsets.endOff=minusoffsets.endOff=fr.endOff;
    if (i==sz+2)
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
  // Limit the adjustment to 256 furmans (1.40625°) to keep close to linear.
  for (i=0;i<adjustment.size();i++)
    if (fabs(adjustment[i])>maxadj)
      maxadj=fabs(adjustment[i]);
  for (i=0;maxadj>256 && i<adjustment.size();i++)
    adjustment[i]*=256/maxadj;
  ret.startOff=fr.startOff-h*adjustment[0];
  ret.endOff=fr.endOff-h*adjustment[sz+1];
  ret.startBear=fr.startBear-lrint(adjustment[sz+2]*FURMAN1);
  for (i=0;i<sz;i++)
    ret.endpoints.push_back(fr.endpoints[i]-hxy[i]*adjustment[i+1]);
  return ret;
}
