/******************************************************/
/*                                                    */
/* readtin.cpp - read a TIN into a pointlist          */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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

#include "readtin.h"
#include "dxf.h"
#include "ptin.h"
#include "tintext.h"
#include "carlsontin.h"

using namespace std;

int readTinFile(pointlist &pl,string fileName,double unit)
/* Returns 0 if no TIN was found, 1 if an invalid TIN was found,
 * and 2 if a valid TIN was found.
 */
{
  int i,j;
  vector<array<xyz,3> > bareTriangles;
  ifstream file;
  int status=0;
  bool anytin=false;
  PtinHeader ptinHeader;
  if (status==0)
  {
    bareTriangles=extractTriangles(readDxfGroups(fileName));
    status=bareTriangles.size()>0;
    if (status)
      anytin=true;
  }
  if (status==1)
  {
    if (unit!=1)
      for (i=0;i<bareTriangles.size();i++)
	for (j=0;j<3;j++)
	  bareTriangles[i][j]*=unit;
    try
    {
      pl.makeBareTriangles(bareTriangles);
      bareTriangles.clear();
      cout<<"Read "<<pl.triangles.size()<<" triangles\n";
      pl.fillInBareTin();
      if (pl.checkTinConsistency())
      {
	cout<<pl.triangles.size()<<" triangles after filling in\n";
	pl.addperimeter();
	status=2;
      }
      else
	status=0;
    }
    catch (...)
    {
      status=0;
    }
  }
  if (status==0)
  {
    ptinHeader=readPtin(fileName,pl);
    status=ptinHeader.tolRatio>0;
    if (ptinHeader.tolRatio!=PT_UNKNOWN_HEADER_FORMAT &&
        ptinHeader.tolRatio!=PT_NOT_PTIN_FILE &&
        ptinHeader.tolRatio!=PT_COUNT_MISMATCH)
      anytin=true;
    if (!ptinHeader.tolerance>0)
      status=0;
  }
  if (status==1)
  {
    pl.addperimeter();
    pl.makeqindex();
    status=2;
  }
  if (status==0)
  {
    status=readCarlsonTin(fileName,pl,unit);
  }
  if (status==1)
  {
    anytin=true;
    try
    {
      pl.fillInBareTin();
      if (pl.checkTinConsistency())
      {
	cout<<pl.triangles.size()<<" triangles after filling in\n";
	pl.addperimeter();
	pl.makeqindex();
	status=2;
      }
      else
	status=0;
    }
    catch (...)
    {
      status=0;
    }
  }
  if (status==0)
  {
    try
    {
      status=readTinText(fileName,pl,unit);
    }
    catch (...)
    {
      status=0;
    }
  }
  if (status==1)
  {
    anytin=true;
    try
    {
      pl.fillInBareTin();
      if (pl.checkTinConsistency())
      {
	cout<<pl.triangles.size()<<" triangles after filling in\n";
	pl.addperimeter();
	pl.makeqindex();
	status=2;
      }
      else
	status=0;
    }
    catch (...)
    {
      status=0;
    }
  }
  if (status==0 && anytin)
    status=1;
  return status;
}
