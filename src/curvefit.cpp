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
    bear=q.bearing(along);
    ret.push_back(distanceInDirection(q.station(along),points[i],bear));
  }
  return ret;
}

polyarc arcFitApprox(Circle startLine,double startOff,int startBear,std::vector<xy> points,Circle endLine,double endOff)
{
  polyarc ret;
  int i,bear=startBear,lastbear;
  xy startPoint=startLine.station(startOff);
  xy endPoint=endLine.station(endOff);
  ret.insert(startPoint);
  for (i=0;i<points.size();i++)
  {
    lastbear=bear;
    bear=twicedir(i?points[i-1]:startPoint,points[i])-bear;
    ret.insert(points[i]);
    ret.setdelta(i,bear-lastbear);
  }
  lastbear=bear;
  bear=twicedir(i?points[i-1]:startPoint,endPoint);
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
