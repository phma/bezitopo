/******************************************************/
/*                                                    */
/* cogospiral.cpp - intersections of spirals          */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
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

using namespace std;

alosta::alosta()
{
  along=0;
  station=xy(0,0);
}

alosta::alosta(double a,xy s)
{
  along=a;
  station=s;
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

vector<alosta> intersection1(spiralarc a,double a1,double a2,spiralarc b,double b1,double b2,bool extend)
/* Returns two alostas, one for a and one for b, or nothing.
 * If extend is true, the spiralarcs are extended to twice their length;
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
  int closecount=0;
  alosta aalosta[3],balosta[3];
  vector<alosta> ret;
  aalosta[0]=alosta(a1,a.station(a1));
  aalosta[1]=alosta(a2,a.station(a2));
  balosta[0]=alosta(b1,b.station(b1));
  balosta[1]=alosta(b2,b.station(b2));
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
    if (aalosta[2].along<-a.length()/2 || aalosta[2].along>3*a.length()/2)
      aalosta[2].along=NAN;
    if (extend && aalosta[2].along<0)
      aalosta[2].along=-aalosta[2].along;
    if (extend && aalosta[2].along>a.length())
      aalosta[2].along=2*a.length()-aalosta[2].along;
    aalosta[2].station=a.station(aalosta[2].along);
    di0=dist(insect,balosta[0].station);
    di1=dist(insect,balosta[1].station);
    d01=dist(balosta[0].station,balosta[1].station);
    if (di1>d01 && di1>di0)
      di0=-di0;
    if (di0>d01 && di0>di1)
      di1=-di1;
    balosta[2].along=(balosta[0].along*di1+balosta[1].along*di0)/d01;
    if (balosta[2].along<-b.length()/2 || balosta[2].along>3*b.length()/2)
      balosta[2].along=NAN;
    if (extend && balosta[2].along<0)
      balosta[2].along=-balosta[2].along;
    if (extend && balosta[2].along>b.length())
      balosta[2].along=2*b.length()-balosta[2].along;
    balosta[2].station=b.station(balosta[2].along);
    isnewcloser=sortpts(aalosta,balosta);
    //cout<<"isnewcloser "<<isnewcloser<<' '<<ldecimal(dist(aalosta[0].station,balosta[0].station))<<' '<<(a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096<<endl;
    if (dist(aalosta[0].station,balosta[0].station)<(a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096)
    {
      closecount++;
      if (aalosta[0].station==balosta[0].station)
	closecount++;
    }
    else
      closecount=0;
  }
  while (isnewcloser && closecount<2);
  if (closecount>1)
  {
    ret.push_back(aalosta[0]);
    ret.push_back(balosta[0]);
  }
  return ret;
}

vector<alosta> intersection1(segment a,double a1,double a2,segment b,double b1,double b2,bool extend)
/* Same as above, but for two segments, for speed and accuracy.
 * Converting a segment to spiralarc sets the midbearing, which for a 4 km
 * segment results is an error up to 1 µm at the ends.
 * 
 * It can exit in two ways:
 * • The point in aalosta and the point in balosta which are closest are close
 *   enough to be the same point. They are returned.
 * • The new point is out of range of either or both of the curves.
 *   Returns an empty vector.
 */
{
  bool isnewcloser,arecloseenough;
  xy insect;
  double di0,di1,d01;
  alosta aalosta[3],balosta[3];
  vector<alosta> ret;
  aalosta[0]=alosta(a1,a.station(a1));
  aalosta[1]=alosta(a2,a.station(a2));
  balosta[0]=alosta(b1,b.station(b1));
  balosta[1]=alosta(b2,b.station(b2));
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
    if (aalosta[2].along<-a.length()/2 || aalosta[2].along>3*a.length()/2)
      aalosta[2].along=NAN;
    if (extend && aalosta[2].along<0)
      aalosta[2].along=-aalosta[2].along;
    if (extend && aalosta[2].along>a.length())
      aalosta[2].along=2*a.length()-aalosta[2].along;
    aalosta[2].station=a.station(aalosta[2].along);
    di0=dist(insect,balosta[0].station);
    di1=dist(insect,balosta[1].station);
    d01=dist(balosta[0].station,balosta[1].station);
    if (di1>d01 && di1>di0)
      di0=-di0;
    if (di0>d01 && di0>di1)
      di1=-di1;
    balosta[2].along=(balosta[0].along*di1+balosta[1].along*di0)/d01;
    if (balosta[2].along<-b.length()/2 || balosta[2].along>3*b.length()/2)
      balosta[2].along=NAN;
    if (extend && balosta[2].along<0)
      balosta[2].along=-balosta[2].along;
    if (extend && balosta[2].along>b.length())
      balosta[2].along=2*b.length()-balosta[2].along;
    balosta[2].station=b.station(balosta[2].along);
    isnewcloser=sortpts(aalosta,balosta);
    //cout<<"isnewcloser "<<isnewcloser<<ldecimal(dist(aalosta[0].station,balosta[0].station))<<' '<<a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station)<<endl;
    arecloseenough=dist(aalosta[0].station,balosta[0].station)<
        (a.length()+b.length()+dist(aalosta[0].station,-balosta[0].station))*DBL_EPSILON*4096;
  }
  while (isnewcloser && !arecloseenough);
  if (arecloseenough)
  {
    ret.push_back(aalosta[0]);
    ret.push_back(balosta[0]);
  }
  return ret;
}

