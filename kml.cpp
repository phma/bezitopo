/******************************************************/
/*                                                    */
/* kml.cpp - Keyhole Markup Language                  */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
/* Writes the boundary of an excerpt of a geoid file to a KML file
 * so that it can be seen on a map.
 */
#include <climits>
#include "kml.h"
#include "projection.h"
#include "halton.h"
using namespace std;

double middleOrdinate(latlong ll0,latlong ll1)
/* Computes the middle by simply averaging coordinates. This is fine, as
 * the only lines that are boundaries of cylintervals are meridians
 * and parallels.
 */
{
  latlong llmid;
  xyz xyz0,xyz1,xyzmid;
  llmid=latlong((ll0.lat+ll1.lat)/2,(ll0.lon+ll1.lon)/2);
  xyz0=Sphere.geoc(ll0,0);
  xyz1=Sphere.geoc(ll1,0);
  xyzmid=(xyz0+xyz1)/2;
  return dist(xyzmid,Sphere.geoc(llmid,0));
}

latlong splitPoint(latlong ll0,latlong ll1,int i,int n)
{
  int j=n-i;
  return latlong((ll0.lat*j+ll1.lat*i)/n,(ll0.lon*j+ll1.lon*i)/n);
}

/* KML requires that inner boundaries be distinguished from the outer boundary.
 * The g1boundary as computed by cubemap has no notion of inner or outer
 * boundary, so the KML export routine must figure out which is the outer
 * boundary. To do this:
 * 1. Find the largest blank area. This is the outside.
 * 2. Mark all g1boundaries adjacent to the outside as outer boundaries.
 * 3. Output each one as KML, including any immediately inside g1boundary as
 *    inner boundary.
 * 4. Remove the written boundaries from the gboundary.
 * 5. If there is any g1boundary left, return to step 1.
 * This requires calculating when a point is inside a g1boundary. To do this,
 * pick a point remote from land and not exactly on any geoquad boundary, and
 * stereographically project the sphere around this point, getting a polyarc.
 * Then use the polyarc::in method to determine whether the projection of
 * the point is inside the polyarc.
 */

void openkml(ofstream &file,char *filename)
{
  file.open(filename);
  file<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      <<"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
      <<"<Document>\n";
}

void closekml(ofstream &file)
{
  file<<"</Document></kml>\n";
  file.close();
}

polyarc flatten(g1boundary g1)
{
  int i;
  polyarc ret;
  arc tmp;
  xy midpt;
  for (i=0;i<g1.size();i++)
    ret.insert(sphereStereoArabianSea.geocentricToGrid(decodedir(g1[i])));
  for (i=0;i<g1.size();i++)
  {
    tmp=ret.getarc(i);
    midpt=sphereStereoArabianSea.geocentricToGrid(decodedir(g1.seg(i).midpoint()));
    tmp=arc(tmp.getstart(),xyz(midpt,0),tmp.getend());
    if (tmp.chordlength()<EARTHRAD && -abs(tmp.getdelta())<-DEG180)
    {
      cerr<<"Took greater arc when it shouldn't"<<endl;
      tmp=arc(tmp.getstart(),xyz(midpt,0),tmp.getend());
    }
    ret.setdelta(i,tmp.getdelta());
  }
  return ret;
}

KmlRegionList kmlRegions(gboundary &gb)
/* Given a gboundary (which has its flatBdy computed, if it didn't already),
 * computes the regions that it divides the earth into. There are normally
 * one more regions than g1boundaries. If gb.size() is more than 32, they
 * cannot all be distinguished; in this case, or if a region is empty,
 * it continues for 30 iterations per segment of boundary before giving up.
 */
{
  int i,r;
  map<unsigned int,xyz>::iterator j;
  latlong ll;
  xyz pnt;
  KmlRegionList ret;
  for (i=0;i<gb.totalSegments()*30 && ret.regionMap.size()<=gb.size();i++)
  {
    pnt=gb.nearPoint();
    r=gb.in(pnt);
    ret.regionMap[r]=pnt;
  }
  ret.blankBitCount=INT_MAX;
  for (j=ret.regionMap.begin();j!=ret.regionMap.end();j++)
    if (j->first<ret.blankBitCount)
      ret.blankBitCount=j->first;
  return ret;
}

int bitcount(int n)
{
  n=((n&0xaaaaaaaa)>>1)+(n&0x55555555);
  n=((n&0xcccccccc)>>2)+(n&0x33333333);
  n=((n&0xf0f0f0f0)>>4)+(n&0x0f0f0f0f);
  n=((n&0xff00ff00)>>8)+(n&0x00ff00ff);
  n=((n&0xffff0000)>>16)+(n&0x0000ffff);
  return n;
}

gboundary regionBoundary(KmlRegionList& regionList,gboundary& allBdy,unsigned reg)
{
  map<unsigned int,xyz>::iterator i;
  int n;
  gboundary ret;
  for (i=regionList.regionMap.begin();i!=regionList.regionMap.end();i++)
    if (bitcount(reg^i->first)==1)
      for (n=0;n<allBdy.size();n++)
        if ((reg^i->first)&(1<<n))
          ret.push_back(allBdy[n]);
  return ret;
}

unsigned int KmlRegionList::biggestBlankRegion(gboundary& allBdy)
/* Finds the biggest blank region and returns the bitmap indicating which
 * g1boundaries it's in. The areas of regions are returned signed, but must
 * be compared unsigned, and the areas of blank regions are the negatives
 * of the actual areas.
 */
{
  map<unsigned int,xyz>::iterator i;
  int n;
  unsigned thisarea,biggestarea=0,ret;
  gboundary regionBdy;
  for (i=regionMap.begin();i!=regionMap.end();i++)
    if (bitcount(i->first)==blankBitCount)
    {
      regionBdy=regionBoundary(*this,allBdy,i->first);
      thisarea=-regionBdy.area();
      if (thisarea>biggestarea)
      {
        biggestarea=thisarea;
        ret=i->first;
      }
    }
  return ret;
}
