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
#include "ldecimal.h"
using namespace std;

char corners[4]={0,1,3,2};

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

double middleOrdinate(vsegment vseg)
/* Computes the middle by simply averaging coordinates. This usually results
 * in a too-big middle ordinate, so apply Element 3:35.
 */
{
  vball vmid;
  xyz xyz0,xyz1,xyzmid;
  double ret,chord;;
  vmid=vseg.midpoint();
  xyz0=decodedir(vseg.start);
  xyz1=decodedir(vseg.end);
  xyzmid=(xyz0+xyz1)/2;
  ret=dist(xyzmid,decodedir(vmid));
  chord=dist(xyz0,xyz1);
  /* At this point, ret may be up to twice the actual middle ordinate.
   * If the two endpoints are in the same face, the chord is less than
   * EARTHRAD*sqrt(8/3). If they're in different faces, the midpoint
   * was computed the slow way and the middle ordinate is accurate.
   */
  if (chord<1.633*EARTHRAD)
  {
    ret=sqr(chord)/4/(2*EARTHRAD-ret);
    ret=sqr(chord)/4/(2*EARTHRAD-ret);
  }
  return ret;
}

latlong splitPoint(latlong ll0,latlong ll1,int i,int n)
{
  int j=n-i;
  if (i&&j)
    return latlong((ll0.lat*j+ll1.lat*i)/n,(ll0.lon*j+ll1.lon*i)/n);
  else if (i)
    return ll1; // (lat*n)/n occasionally !=lat
  else
    return ll0;
}

vector<latlong> splitPoints(latlong ll0,latlong ll1)
{
  int n=1+floor(fabs(ll0.lat-ll1.lat)+fabs(ll0.lon-ll1.lon));
  int newn,i;
  vector<latlong> ret;
  double midord;
  while (true)
  {
    midord=middleOrdinate(splitPoint(ll0,ll1,n/2,n),splitPoint(ll0,ll1,n/2+1,n));
    if (midord<=MAXMIDORD)
      break;
    newn=floor(n*sqrt(midord/MAXMIDORD));
    if (newn<=n)
      newn=n+1;
    n=newn;
  }
  for (i=0;i<n;i++)
    ret.push_back(splitPoint(ll0,ll1,i,n));
  return ret;
}

gboundary gbounds(cylinterval cyl)
{
  g1boundary ret1;
  gboundary ret;
  vector<latlong> side;
  int i,j;
  for (i=0;i<4;i++)
  {
    side=splitPoints(cyl.corner(corners[i]),cyl.corner(corners[(i+1)&3]));
    for (j=0;j<side.size();j++)
      ret1.push_back(encodedir(Sphere.geoc(side[j],0)));
  }
  ret1.setInner(false);
  ret.push_back(ret1);
  ret.splitoff(-1);
  ret.deleteNullSegments();
  ret.deleteRetrace();
  ret.deleteEmpty();
  return ret;
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

void openkml(ofstream &file,string filename)
{
  file.open(filename);
  file<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      <<"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
      <<"<Document>\n";
}

void kmlBoundary(ofstream &file,g1boundary g)
{
  bool inner=g.isInner();
  int i;
  latlong ll;
  file<<(inner?"<innerBoundaryIs>":"<outerBoundaryIs>")<<"<LinearRing><coordinates>\n";
  for (i=0;i<=g.size();i++)
  {
    ll=decodedir(g[inner?(g.size()-i):i]).latlon();
    file<<ldecimal(radtodeg(ll.lon),1/(EARTHRAD*cos(ll.lat)+1))<<','<<ldecimal(radtodeg(ll.lat),1/EARTHRAD)<<'\n';
  }
  file<<"</coordinates></LinearRing>"<<(inner?"</innerBoundaryIs>":"</outerBoundaryIs>")<<endl;
}

void refine(g1boundary &g1)
{
  int i;
  double mo;
  for (i=0;i<g1.size();i++)
    if ((mo=middleOrdinate(g1.seg(i)))>MAXMIDORD)
    {
      //cout<<"midord "<<mo<<endl;
      g1.halve(i);
      i=-1;
    }
}

void kmlPolygon(ofstream &file,gboundary g)
{
  int i;
  g1boundary g1;
  file<<"<Placemark><Polygon>\n";
  for (i=0;i<g.size();i++)
  {
    g1=g[i];
    refine(g1);
    kmlBoundary(file,g1);
  }
  file<<"</Polygon></Placemark>"<<endl;
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
  ret.setlengths();
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

#ifdef NUMSGEOID
gboundary gbounds(geoid &geo)
{
  cylinterval n;
  n.setempty();
  if (geo.cmap)
    return geo.cmap->gbounds();
  else if (geo.glat)
    return gbounds(geo.glat->boundrect());
  else
    return gbounds(n);
}
#endif

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

gboundary extractRegion(gboundary &gb)
// Extracts a full region bounding the largest blank region and removes it from gb.
{
  gboundary ret;
  g1boundary gb1;
  unsigned blankRegion,fullRegion;
  KmlRegionList regionList;
  map<unsigned int,xyz>::iterator i;
  int j,k;
  vector<int> delenda;
  regionList=kmlRegions(gb);
  blankRegion=regionList.biggestBlankRegion(gb);
  for (i=regionList.regionMap.begin();i!=regionList.regionMap.end();i++)
    if (bitcount(i->first^blankRegion)==1)
      fullRegion=i->first;
  ret=regionBoundary(regionList,gb,fullRegion);
  for (j=0;j<gb.size();j++)
    for (k=0;k<ret.size();k++)
      if (gb[j]==ret[k])
      {
        delenda.push_back(j);
        ret.setInner(k,((~fullRegion^blankRegion)>>j)&1);
      }
  if (delenda.size()==0 && ret.size()==0 && gb.size()>0)
  {
    cerr<<"No region found, deleting one g1boundary"<<endl;
    delenda.push_back(0);
    ret.push_back(gb[0]);
  }
  for (j=delenda.size()-1;j>=0;j--)
    gb.erase(delenda[j]);
  return ret;
}

void outKml(gboundary gb,string filename)
{
  ofstream file;
  gboundary poly;
  openkml(file,filename);
  while (gb.size())
  {
    poly=extractRegion(gb);
    kmlPolygon(file,poly);
  }
  closekml(file);
}
