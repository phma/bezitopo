/******************************************************/
/*                                                    */
/* cogospiral.cpp - intersections of spirals          */
/*                                                    */
/******************************************************/
/* Copyright 2015,2017-2018 Pierre Abbat.
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
#include <cfloat>
#include <iostream>
#include "ldecimal.h"
#include "cogospiral.h"
#include "manysum.h"
#include "relprime.h"
#include "matrix.h"
#include "pointlist.h"
#include "minquad.h"
#include "rootfind.h"

using namespace std;

alosta::alosta()
{
  along=0;
  station=xy(0,0);
  bearing=0;
  curvature=0;
}

alosta::alosta(double a,xy s)
{
  along=a;
  station=s;
  bearing=0;
  curvature=0;
}

alosta::alosta(double a,xy s,int b,double c)
{
  along=a;
  station=s;
  bearing=b;
  curvature=c;
}

void alosta::setStation(segment *seg,double alo)
{
  along=alo;
  station=seg->station(alo);
  bearing=seg->bearing(alo);
  curvature=seg->curvature(alo);
}

bool sortpts(alosta a[],alosta b[])
// Returns true if any swaps took place.
{
  bool ret=false;
  int i,j,apos,bpos;
  double dst,closest;
  closest=INFINITY;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
    {
      dst=dist(a[i].station,b[j].station);
      if (dst<closest)
      {
	closest=dst;
	apos=i;
	bpos=j;
      }
    }
  if (apos>0)
  {
    ret=true;
    swap(a[0],a[apos]);
  }
  if (bpos>0)
  {
    ret=true;
    swap(b[0],b[bpos]);
  }
  closest=INFINITY;
  for (i=1;i<3;i++)
    for (j=1;j<3;j++)
    {
      dst=dist(a[i].station,b[j].station);
      if (dst<closest)
      {
	closest=dst;
	apos=i;
	bpos=j;
      }
    }
  if (apos>1)
  {
    ret=true;
    swap(a[1],a[apos]);
  }
  if (bpos>1)
  {
    ret=true;
    swap(b[1],b[bpos]);
  }
  if (a[0].station==a[1].station)
  {
    ret=true;
    swap(a[1],a[2]); // if a[0]==a[1], a divide by zero results in the next iteration
  }
  if (b[0].station==b[1].station)
  {
    ret=true;
    swap(b[1],b[2]);
  }
  return ret;
}

bool sortpts2(alosta a[],alosta b[])
// Returns true if any swaps took place.
{
  bool ret=false;
  int i,j,apos,bpos;
  double dst,closest;
  closest=INFINITY;
  for (i=0;i<2;i++)
    for (j=0;j<2;j++)
    {
      dst=dist(a[i].station,b[j].station);
      if (dst<closest)
      {
	closest=dst;
	apos=i;
	bpos=j;
      }
    }
  if (apos>0)
  {
    ret=true;
    swap(a[0],a[apos]);
  }
  if (bpos>0)
  {
    ret=true;
    swap(b[0],b[bpos]);
  }
  return ret;
}

vector<alosta> intersection1(segment *a,double a1,double a2,segment *b,double b1,double b2,bool extend)
/* Returns two alostas, one for a and one for b, or nothing.
 * If extend is true, the segments/arcs/spiralarcs are extended to twice their length;
 * e.g. one of length 5 extends from station -2.5 to station 7.5.
 * 
 * It can exit in three ways:
 * • The point in aalosta and the point in balosta which are closest are close
 *   enough to be the same point. They are returned.
 * • The new points are farther from each other and the previous points than
 *   previous points are from each other. Returns an empty vector.
 * • The new point is out of range of either or both of the curves.
 *   Returns an empty vector.
 */
{
  bool isnewcloser;
  xy insect;
  double di0,di1,d01;
  int closecount=0,mirrorcount=0;
  alosta aalosta[3],balosta[3];
  vector<alosta> ret;
  aalosta[0].setStation(a,a1);
  aalosta[1].setStation(a,a2);
  balosta[0].setStation(b,b1);
  balosta[1].setStation(b,b2);
  do
  {
    insect=intersection(aalosta[0].station,aalosta[1].station,balosta[0].station,balosta[1].station);
    di0=dist(insect,aalosta[0].station);
    di1=dist(insect,aalosta[1].station);
    d01=dist(aalosta[0].station,aalosta[1].station);
    if (di1>d01 && di1>di0)
      di0=-di0;
    if (di0>d01 && di0>di1)
      di1=-di1;
    aalosta[2].along=(aalosta[0].along*di1+aalosta[1].along*di0)/d01;
    if (aalosta[2].along<-a->length()/2 || aalosta[2].along>3*a->length()/2)
      aalosta[2].along=NAN;
    if (!extend && aalosta[2].along<0)
    {
      aalosta[2].along=-aalosta[2].along;
      mirrorcount++;
    }
    if (!extend && aalosta[2].along>a->length())
    {
      aalosta[2].along=2*a->length()-aalosta[2].along;
      mirrorcount++;
    }
    aalosta[2].setStation(a,aalosta[2].along);
    di0=dist(insect,balosta[0].station);
    di1=dist(insect,balosta[1].station);
    d01=dist(balosta[0].station,balosta[1].station);
    if (di1>d01 && di1>di0)
      di0=-di0;
    if (di0>d01 && di0>di1)
      di1=-di1;
    balosta[2].along=(balosta[0].along*di1+balosta[1].along*di0)/d01;
    if (balosta[2].along<-b->length()/2 || balosta[2].along>3*b->length()/2)
      balosta[2].along=NAN;
    if (!extend && balosta[2].along<0)
    {
      balosta[2].along=-balosta[2].along;
      mirrorcount++;
    }
    if (!extend && balosta[2].along>b->length())
    {
      balosta[2].along=2*b->length()-balosta[2].along;
      mirrorcount++;
    }
    balosta[2].setStation(b,balosta[2].along);
    isnewcloser=sortpts(aalosta,balosta);
    //cout<<"isnewcloser "<<isnewcloser<<' '<<ldecimal(dist(aalosta[0].station,balosta[0].station))<<' '<<(a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096<<endl;
    if (dist(aalosta[0].station,balosta[0].station)<(a->length()+b->length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096)
    {
      closecount++;
      if (aalosta[0].station==balosta[0].station)
	closecount++;
    }
    else
      closecount=0;
  }
  while (isnewcloser && closecount<2 && mirrorcount<256);
  if (closecount>1)
  {
    ret.push_back(aalosta[0]);
    ret.push_back(balosta[0]);
  }
  return ret;
}

vector<alosta> intersection1(segment *a,double a1,segment *b,double b1,bool extend)
/* Returns two alostas, one for a and one for b, or nothing.
 * If extend is true, the segments/arcs/spiralarcs are extended to twice their length;
 * e.g. one of length 5 extends from station -2.5 to station 7.5.
 * 
 * It can exit in three ways:
 * • The point in aalosta and the point in balosta which are closest are close
 *   enough to be the same point. They are returned.
 * • The new points are farther from each other and the previous points than
 *   previous points are from each other. Returns an empty vector.
 * • The new point is out of range of either or both of the curves.
 *   Returns an empty vector.
 */
{
  bool isnewcloser;
  xy insect;
  double di0,di1,d01;
  int closecount=0,mirrorcount=0;
  alosta aalosta[2],balosta[2];
  vector<alosta> ret;
  aalosta[0].setStation(a,a1);
  balosta[0].setStation(b,b1);
  do
  {
    insect=intersection(aalosta[0].station,aalosta[0].bearing,balosta[0].station,balosta[0].bearing);
    di0=distanceInDirection(aalosta[0].station,insect,aalosta[0].bearing);
    aalosta[1].along=aalosta[0].along+di0;
    if (aalosta[1].along<-a->length()/2 || aalosta[1].along>3*a->length()/2)
      aalosta[1].along=NAN;
    if (!extend && aalosta[1].along<0)
    {
      aalosta[1].along=-aalosta[1].along;
      mirrorcount++;
    }
    if (!extend && aalosta[1].along>a->length())
    {
      aalosta[1].along=2*a->length()-aalosta[1].along;
      mirrorcount++;
    }
    aalosta[1].setStation(a,aalosta[1].along);
    di0=distanceInDirection(balosta[0].station,insect,balosta[0].bearing);
    balosta[1].along=balosta[0].along+di0;
    if (balosta[1].along<-b->length()/2 || balosta[1].along>3*b->length()/2)
      balosta[1].along=NAN;
    if (!extend && balosta[1].along<0)
    {
      balosta[1].along=-balosta[1].along;
      mirrorcount++;
    }
    if (!extend && balosta[1].along>b->length())
    {
      balosta[1].along=2*b->length()-balosta[1].along;
      mirrorcount++;
    }
    balosta[1].setStation(b,balosta[1].along);
    isnewcloser=sortpts2(aalosta,balosta);
    //cout<<"isnewcloser "<<isnewcloser<<' '<<ldecimal(dist(aalosta[0].station,balosta[0].station))<<' '<<(a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096<<endl;
    if (dist(aalosta[0].station,balosta[0].station)<(a->length()+b->length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096)
    {
      closecount++;
      if (aalosta[0].station==balosta[0].station)
	closecount++;
    }
    else
      closecount=0;
  }
  while (isnewcloser && closecount<2 && mirrorcount<256);
  if (closecount>1)
  {
    ret.push_back(aalosta[0]);
    ret.push_back(balosta[0]);
  }
  return ret;
}

/* If two spiralarcs intersect twice near the end of both, the secant method
 * may miss one, but the tangent method will find both. Conversely, if they
 * osculate, the tangent method will fail to converge because of roundoff,
 * but the secant method will find the intersection. So both methods are needed.
 */
vector<array<alosta,2> > intersections(segment *a,segment *b,bool extend)
/* Returns the intersections of a and b in order along a. In cases of multiple
 * intersection (tangency or osculation), it may return the wrong number of
 * intersections, occasionally even the wrong parity of number of intersections
 * (1 or 3 for tangent circles, where it should return 2). You must check whether
 * the resulting pieces of a are on opposite sides of b.
 */
{
  vector<array<alosta,2> > inters,ret;
  array<alosta,2> int1;
  vector<alosta> int0;
  vector<int> bounds;
  int h,i,j,adiv,bdiv,range,rangeSize;
  double maxcur,endcur,alen,blen;
  alen=a->length();
  blen=b->length();
  maxcur=fabs(a->curvature(0));
  endcur=fabs(a->curvature(alen));
  if (endcur>maxcur)
    maxcur=endcur;
  adiv=nearbyint(maxcur*alen+alen/blen)+3;
  if (adiv<3 || adiv>4096)
    adiv=4096;
  maxcur=fabs(b->curvature(0));
  endcur=fabs(b->curvature(blen));
  if (endcur>maxcur)
    maxcur=endcur;
  bdiv=nearbyint(maxcur*blen+blen/alen)+3;
  if (bdiv<3 || bdiv>4096)
    bdiv=4096;
  for (i=0;i<adiv;i++)
    for (j=0;j<bdiv;j++)
    {
      int0=intersection1(a,i*alen/adiv,(i+1)*alen/adiv,b,j*blen/bdiv,(j+1)*blen/bdiv,extend);
      if (int0.size())
      {
	int1[0]=int0[0];
	int1[1]=int0[1];
	inters.push_back(int1);
      }
    }
  for (i=0;i<=adiv;i++)
    for (j=0;j<=bdiv;j++)
    {
      int0=intersection1(a,i*alen/adiv,b,j*blen/bdiv,extend);
      if (int0.size())
      {
	int1[0]=int0[0];
	int1[1]=int0[1];
	inters.push_back(int1);
      }
    }
  for (h=relprime(inters.size());h;h=(h>1)?relprime(h):0) // Shell sort
    for (i=h;i<inters.size();i++)
      for (j=i-h;j>=0 && (inters[j][0].along>inters[j+h][0].along || (inters[j][0].along==inters[j+h][0].along && inters[j][1].along>inters[j+h][1].along));j-=h)
	swap(inters[j],inters[j+h]);
  for (i=0;i<=inters.size();i++)
    if (i==0 || i==inters.size() || -abs(inters[i][0].bearing-inters[i][1].bearing-inters[i-1][0].bearing+inters[i-1][1].bearing)<-255)
    {
      bounds.push_back(i);
      //cout<<i<<' ';
    }
  //cout<<endl;
  for (range=0;range<bounds.size()-1;range++)
  {
    rangeSize=bounds[range+1]-bounds[range];
    for (h=relprime(rangeSize);h;h=(h>1)?relprime(h):0)
      for (i=h;i<rangeSize;i++) // Sort just the alostas of b within the range.
	for (j=i-h;j>=0 && (inters[j+bounds[range]][1].along>inters[j+bounds[range]+h][1].along);j-=h)
	  swap(inters[j+bounds[range]][1],inters[j+bounds[range]+h][1]);
    if (rangeSize&1)
      int1=inters[bounds[range]+rangeSize/2];
    else // Take the median, not the mean. There are usually a few
    {    // outliers which would make the mean inaccurate.
      int1[0].along=(inters[bounds[range]+rangeSize/2  ][0].along+
                     inters[bounds[range]+rangeSize/2-1][0].along)/2;
      if (int1[0].along==inters[bounds[range]+rangeSize/2][0].along)
	int1[0]=inters[bounds[range]+rangeSize/2][0];
      else if (int1[0].along==inters[bounds[range]+rangeSize/2-1][0].along)
	int1[0]=inters[bounds[range]+rangeSize/2-1][0];
      else
	int1[0].setStation(a,int1[0].along);
      int1[1].along=(inters[bounds[range]+rangeSize/2  ][1].along+
                     inters[bounds[range]+rangeSize/2-1][1].along)/2;
      if (int1[1].along==inters[bounds[range]+rangeSize/2][1].along)
	int1[1]=inters[bounds[range]+rangeSize/2][1];
      else if (int1[1].along==inters[bounds[range]+rangeSize/2-1][1].along)
	int1[1]=inters[bounds[range]+rangeSize/2-1][1];
      else
	int1[1].setStation(b,int1[1].along);
    }
    ret.push_back(int1);
  }
  //ret=inters;
  return ret;
}

double meanSquareDistance(segment *a,segment *b)
/* All points on a should have a closest point on b, without going off the ends
 * of b. In other words, a should be part of the approximation to b.
 */
{
  vector<double> c;
  xy astation,bstation;
  double aalong,balong,alen=a->length();
  aalong=GAUSSQ4P0P*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  c.push_back(sqr(dist(astation,bstation))*GAUSSQ4P0W);
  aalong=GAUSSQ4P1P*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  c.push_back(sqr(dist(astation,bstation))*GAUSSQ4P1W);
  aalong=(1-GAUSSQ4P1P)*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  c.push_back(sqr(dist(astation,bstation))*GAUSSQ4P1W);
  aalong=(1-GAUSSQ4P0P)*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  c.push_back(sqr(dist(astation,bstation))*GAUSSQ4P0W);
  return pairwisesum(c);
}

array<double,4> weightedDistance(segment *a,segment *b)
/* As above, a should be part of an approximation to b.
 * Returns the distances weighted so that, if all of them over an entire
 * approximation are squared, summed, and divided by the length, the result
 * is meanSquareDistance(apx,b). Used in least squares.
 */
{
  array<double,4> ret;
  array<int,4> times;
  xy astation,bstation;
  double aalong,balong,alen=a->length(),sqrtlen=sqrt(alen);
  int bbear;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  bbear=b->bearing(balong);
#ifndef NDEBUG
  times[0]=closetime;
#endif
  ret[0]=distanceInDirection(astation,bstation,bbear+DEG90)*sqrt(GAUSSQ4P0W)*sqrtlen;
  aalong=GAUSSQ4P1P*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  bbear=b->bearing(balong);
#ifndef NDEBUG
  times[1]=closetime;
#endif
  ret[1]=distanceInDirection(astation,bstation,bbear+DEG90)*sqrt(GAUSSQ4P1W)*sqrtlen;
  aalong=(1-GAUSSQ4P1P)*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  bbear=b->bearing(balong);
#ifndef NDEBUG
  times[2]=closetime;
#endif
  ret[2]=distanceInDirection(astation,bstation,bbear+DEG90)*sqrt(GAUSSQ4P1W)*sqrtlen;
  aalong=(1-GAUSSQ4P0P)*alen;
  astation=a->station(aalong);
  balong=b->closest(astation);
  bstation=b->station(balong);
  bbear=b->bearing(balong);
#ifndef NDEBUG
  times[3]=closetime;
#endif
  ret[3]=distanceInDirection(astation,bstation,bbear+DEG90)*sqrt(GAUSSQ4P0W)*sqrtlen;
  return ret;
}

array<double,2> besidement(Circle a,Circle b)
/* Returns the distance along the two circles to the points where the circles
 * are closest or farthest apart. There are two such points on each circle,
 * unless it's a straight line; it generally returns the one closer to the
 * zero point. The bearings of the circles at the points returned are the same,
 * not 180° apart.
 *
 * If the circles are concentric, the solution is indeterminate. It may return
 * {NaN,NaN} or a pair of finite numbers, depending on the roundoff error
 * of bearings. If the circles are both straight lines and intersect, the
 * solution does not exist; it returns {∞,∞}.
 */
{
  array<double,2> ret;
  xy pnta,pntb;
  double stepa,stepb,erra,errb,distalong,distacross,curscale;
  double scra,scrb,atx,aty,h;
  int beara,bearavg,bearhd,bearb,cendir;
  matrix mat(2,2),v(2,1);
  //Measure ms; // for debugging
  //ms.addUnit(DEGREE_B);
  //ms.addUnit(METER);
  //ms.setDefaultPrecision(LENGTH,0.001);
  //ms.setDefaultPrecision(ANGLE_B,SEC1);
  curscale=1/hypot(a.radius(),b.radius());
  if (curscale)
  {
    scra=a.radius()*curscale;
    scrb=b.radius()*curscale;
  }
  else
  {
    scra=sign(b.curvature());
    scrb=sign(a.curvature());
  }
  beara=a.bearing(0);
  bearb=b.bearing(0);
  pnta=a.station(0);
  pntb=b.station(0);
  bearavg=beara+(bearhd=foldangle(bearb-beara)/2);
  distalong=distanceInDirection(pnta,pntb,bearavg);
  distacross=distanceInDirection(pnta,pntb,bearavg-DEG90);
  //cout<<"Distance along "<<ms.formatMeasurementUnit(distalong,LENGTH)<<' '<<
  //  ms.formatMeasurementUnit(bearavg,ANGLE_B);
  //cout<<" Distance across "<<ms.formatMeasurementUnit(distacross,LENGTH)<<' '<<
  //  ms.formatMeasurementUnit(bearavg-DEG90,ANGLE_B)<<endl;
  //cout<<"Bearing half difference "<<ms.formatMeasurementUnit(bearhd,ANGLE_B)<<endl;
  cendir=atan2i(aty=distalong*curscale-sin(bearhd)*(scra+scrb),
		atx=distacross*curscale+cos(bearhd)*(scra-scrb));
  if (cendir>DEG90)
    cendir-=DEG180;
  if (cendir<-DEG90)
    cendir+=DEG180;
  //cout<<"cendir "<<ms.formatMeasurementUnit(cendir,ANGLE_B)<<endl;
  if (a.curvature())
    stepa=bintorad(cendir+bearhd)/a.curvature();
  else
    stepa=distanceInDirection(pnta,b.center(),beara);
  if (b.curvature())
    stepb=bintorad(cendir-bearhd)/b.curvature();
  else
    stepb=distanceInDirection(pntb,a.center(),bearb);
  if (a.curvature()==0 && b.curvature()==0)
    if (foldangle(beara-bearb))
      stepa=stepb=INFINITY;
    else
      stepa=stepb=NAN;
  if (atx==0 && aty==0 && curscale>0)
    stepa=stepb=NAN;
  h=0.001/hypot(a.curvature(),b.curvature());
  h/=significand(h);
  pnta=a.station(stepa);
  pntb=b.station(stepb);
  if (a.curvature() && b.curvature())
  {
    ret[0]=minquad(stepa-h,sqr(dist(pntb,a.station(stepa-h))),
		   stepa  ,sqr(dist(pntb,a.station(stepa  ))),
		   stepa+h,sqr(dist(pntb,a.station(stepa+h))));
    ret[1]=minquad(stepb-h,sqr(dist(pnta,b.station(stepb-h))),
		   stepb  ,sqr(dist(pnta,b.station(stepb  ))),
		   stepb+h,sqr(dist(pnta,b.station(stepb+h))));
    ret[0]=(ret[0]*fabs(b.curvature())+stepa*fabs(a.curvature()))/(fabs(b.curvature())+fabs(a.curvature()));
    ret[1]=(ret[1]*fabs(a.curvature())+stepb*fabs(b.curvature()))/(fabs(a.curvature())+fabs(b.curvature()));
  }
  else if (a.curvature())
  {
    ret[1]=stepb;
    ret[0]=minquad(stepa-h,sqr(dist(pntb,a.station(stepa-h))),
		   stepa  ,sqr(dist(pntb,a.station(stepa  ))),
		   stepa+h,sqr(dist(pntb,a.station(stepa+h))));
  }
  else if (b.curvature())
  {
    ret[0]=stepa;
    ret[1]=minquad(stepb-h,sqr(dist(pnta,b.station(stepb-h))),
		   stepb  ,sqr(dist(pnta,b.station(stepb  ))),
		   stepb+h,sqr(dist(pnta,b.station(stepb+h))));
  }
  else
  {
    ret[0]=stepa;
    ret[1]=stepb;
  }
  return ret;
}

vector<alosta> besidement1(segment *a,double a1,segment *b,double b1)
/* Finds a point on a and a point on b which are beside each other, that is,
 * the bearings of the curves at those points are equal and perpendicular to
 * the direction from one point to the other.
 * Returns two alostas, one for a and one for b, or nothing.
 * It can exit in three ways:
 * • It finds a cycle of length 1, i.e. neither aalosta nor balosta changes, and
 *   both are within their curve lengths. They are returned.
 * • It finds a cycle of length greater than 2. Returns an empty vector.
 * • One or both of the points goes off the end of its curve or is NaN.
 *   Returns an empty vector.
 * • The points jiggle chaotically in a small area. Returns the average.
 */
{
  int i=0,lasti,done=0;
  double aalong[2],balong[2];
  double aavg=NAN,adev=NAN,bavg=NAN,bdev=NAN;
  vector<double> asum,adevsum,bsum,bdevsum;
  vector<alosta> ret;
  array<double,2> step;
  aalong[0]=a1;
  balong[0]=b1;
  do
  {
    i++;
    step=besidement(a->osculatingCircle(aalong[0]),b->osculatingCircle(balong[0]));
    aalong[0]+=step[0]/sqrt(i);
    balong[0]+=step[1]/sqrt(i);
    asum.push_back(aalong[0]);
    adevsum.push_back(sqr(aalong[0]-aavg));
    bsum.push_back(balong[0]);
    bdevsum.push_back(sqr(balong[0]-bavg));
    if ((i&(i-1))==0)
    {
      aalong[1]=aalong[0];
      balong[1]=balong[0];
      aavg=pairwisesum(asum)/asum.size();
      bavg=pairwisesum(bsum)/bsum.size();
      adev=sqrt(pairwisesum(adevsum)/adevsum.size());
      bdev=sqrt(pairwisesum(bdevsum)/bdevsum.size());
      //cout<<i<<' '<<ldecimal(aavg)<<' '<<ldecimal(bavg)<<' '<<adev*a->curvature(aavg)<<' '<<bdev*b->curvature(bavg)<<endl;
      if (fabs(adev*a->curvature(aavg))<1.2e-8 && fabs(bdev*b->curvature(bavg))<1.2e-8)
	done=4;
      asum.clear();
      bsum.clear();
      adevsum.clear();
      bdevsum.clear();
      lasti=i;
    }
    else if (aalong[0]==aalong[1] && balong[0]==balong[1])
      if (i-lasti==1)
	done=1;
      else
	done=2;
    if (aalong[0]<0 || aalong[0]>a->length() || isnan(aalong[0]) ||
        balong[0]<0 || balong[0]>b->length() || isnan(balong[0]))
      done=3;
  } while (!done);
  cout<<"done="<<done<<'\n';
  if (done==1)
  {
    ret.resize(2);
    ret[0].setStation(a,aalong[0]);
    ret[1].setStation(b,balong[0]);
  }
  if (done==4)
  {
    ret.resize(2);
    ret[0].setStation(a,aavg);
    ret[1].setStation(b,bavg);
  }
  return ret;
}

double fbeside(double x,arc &a,spiralarc &b,double arcmid,double spiralmid,double scale)
{
  array<double,2> step;
  step=besidement(a.osculatingCircle(arcmid+scale*(x-spiralmid)),b.osculatingCircle(x));
  return x+step[1];
}

vector<alosta> besidement2(arc a,spiralarc b)
/* This should work where besidement1 doesn't. In an approximation, there's a
 * point near the middle of an arc where the spiral has the same curvature as
 * the arc, so the line between the centers of curvature (that of the arc lying
 * just outside the evolute of the spiral) is parallel to the average of the
 * bearings of the arc and the spiral. This results in the bearing it tries
 * to find suddenly flipping, and the next point in the iteration being way off
 * the end of the arc, and besidement1 gives up.
 *
 * besidement2 uses this flip point to split the arc and spiral in two parts,
 * in each of which it searches for the point where f(x)-x=0, where f(x) is
 * the distance along the spiral returned by besidement and x is the distance
 * along the spiral where the osculating circle is taken.
 */
{
  vector<alosta> ret;
  double arcmid=a.length()/2,spiralmid;
  double scale; // length of a / length of corresponding part of b
  double spiralstart,spiralend;
  double x0,y0,x1,y1,x2,y2;
  double split0,split2;
  brent br;
  spiralstart=b.closest(a.getstart(),INFINITY,true);
  spiralend=b.closest(a.getend(),INFINITY,true);
  spiralmid=(spiralstart+spiralend)/2;
  scale=a.length()/(spiralend-spiralstart);
  x0=spiralstart;
  x1=spiralmid;
  x2=spiralend;
  y0=fbeside(x0,a,b,arcmid,spiralmid,scale);
  y1=fbeside(x1,a,b,arcmid,spiralmid,scale);
  y2=fbeside(x2,a,b,arcmid,spiralmid,scale);
  while (x0<x1 && x1<x2)
  {
    if (fabs(y0-y1)>fabs(y1-y2))
    {
      x2=x1;
      y2=y1;
    }
    else
    {
      x0=x1;
      y0=y1;
    }
    x1=(x0+x2)/2;
    y1=fbeside(x1,a,b,arcmid,spiralmid,scale);
  }
  //cout<<"It flips at "<<x1<<" between "<<spiralstart<<" and "<<spiralend<<'\n';
  split0=x0;
  split2=x2;
  x0=spiralstart;
  x2=split0;
  y0=fbeside(x0,a,b,arcmid,spiralmid,scale)-x0;
  y2=fbeside(x2,a,b,arcmid,spiralmid,scale)-x2;
  if (y0*y2<=0)
  {
    x1=br.init(x0,y0,x2,y2);
    while (!br.finished())
    {
      y1=fbeside(x1,a,b,arcmid,spiralmid,scale)-x1;
      x1=br.step(y1);
    }
    ret.resize(ret.size()+2);
    cout<<"beside "<<x1<<endl;
    ret[ret.size()-1].setStation(&b,x1);
    ret[ret.size()-2].setStation(&a,a.closest(ret[ret.size()-1].station));
  }
  x0=split2;
  x2=spiralend;
  y0=fbeside(x0,a,b,arcmid,spiralmid,scale)-x0;
  y2=fbeside(x2,a,b,arcmid,spiralmid,scale)-x2;
  if (y0*y2<=0)
  {
    x1=br.init(x0,y0,x2,y2);
    while (!br.finished())
    {
      y1=fbeside(x1,a,b,arcmid,spiralmid,scale)-x1;
      x1=br.step(y1);
    }
    ret.resize(ret.size()+2);
    cout<<"beside "<<x1<<endl;
    ret[ret.size()-1].setStation(&b,x1);
    ret[ret.size()-2].setStation(&a,a.closest(ret[ret.size()-1].station));
  }
  return ret;
}
