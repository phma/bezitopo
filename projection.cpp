/******************************************************/
/*                                                    */
/* projection.cpp - map projections                   */
/*                                                    */
/******************************************************/
/* Copyright 2016-2018 Pierre Abbat.
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
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "projection.h"
#include "rootfind.h"
#include "ldecimal.h"
#include "kml.h"

#define PROJ_CC 1
#define PROJ_TM 2
#define PROJ_OM 3

using namespace std;

Projection::Projection()
{
  ellip=&Sphere;
  offset=xy(0,0);
  scale=1;
}

void Projection::setBoundary(g1boundary boundary)
{
  flatBdy=flatten(boundary);
  areaSign=signbit(flatBdy.area());
}

bool Projection::in(xyz geoc)
{
  xy pntproj=sphereStereoArabianSea.geocentricToGrid(geoc);
  return flatBdy.in(pntproj)+areaSign>0.5;
}

bool Projection::in(latlong ll)
{
  return in(Sphere.geoc(ll,0));
}

bool Projection::in(vball v)
{
  return in(decodedir(v));
}

void LambertConicSphere::setParallel(double Parallel)
{
  centralParallel=Parallel;
  exponent=sin(Parallel);
  if (exponent==0)
    coneScale=1;
  else if (fabs(exponent)==1)
    coneScale=2;
  else
    coneScale=cos(Parallel)/pow(tan((M_PIl/2-Parallel)/2),exponent);
  //cout<<"Parallel "<<radtodeg(Parallel)<<" coneScale "<<coneScale<<endl;
}

LambertConicSphere::LambertConicSphere():Projection()
{
  centralMeridian=0;
  setParallel(0);
  poleY=INFINITY;
}

LambertConicSphere::LambertConicSphere(double Meridian,double Parallel):Projection()
{
  latlong maporigin;
  centralMeridian=Meridian;
  setParallel(Parallel);
  poleY=0;
  maporigin=latlong(Meridian,Parallel);
  poleY=-latlongToGrid(maporigin).gety();
}

LambertConicSphere::LambertConicSphere(double Meridian,double Parallel0,double Parallel1):Projection()
{
  latlong maporigin;
  brent br;
  latlong ll;
  double ratiolog0,ratiolog1,Parallel;
  centralMeridian=Meridian;
  setParallel(Parallel0);
  ratiolog0=scaleRatioLog(Parallel0,Parallel1);
  setParallel(Parallel1);
  ratiolog1=scaleRatioLog(Parallel0,Parallel1);
  /* Setting one parallel to ±90° and the other to something else, or either
   * or both to ±(>90°), is an error. I'm not sure whether to throw an exception
   * or set the state to invalid. I'm now setting it to invalid.
   * If the parallels are 45° and 90°, different computers give wildly different
   * results: 84.714395° on Linux/Intel, but 67.5° on Linux/ARM and DFBSD.
   * This is because M_PIl!=M_PI, resulting in M_PIl/2-ll.lat being tiny but
   * positive when ll.lat is M_PIl rounded to double.
   * If the parallels are 45° and DEG90-1 (89.999999832°), the three computers
   * agree that Parallel is 82.686083 (but no more precisely), so that is not
   * an error.
   */
  if ((Parallel0!=Parallel1 && (radtobin(fabs(Parallel0))==DEG90 || radtobin(fabs(Parallel1))==DEG90))
      || fabs(Parallel0)>M_PIl/2
      || fabs(Parallel1)>M_PIl/2)
  {
    centralParallel=poleY=exponent=coneScale=NAN;
    cerr<<"Invalid parallels in LambertConicSphere"<<endl;
  }
  else
  {
    Parallel=br.init(Parallel0,ratiolog0,Parallel1,ratiolog1,false);
    while (!br.finished())
    {
      cout<<"Parallel "<<ldecimal(radtodeg(Parallel))<<endl;
      setParallel(Parallel);
      Parallel=br.step(scaleRatioLog(Parallel0,Parallel1));
    }
    setParallel(Parallel);
    ll.lon=centralMeridian;
    ll.lat=Parallel0;
    scale=1/scaleFactor(ll);
    poleY=0;
    maporigin=latlong(Meridian,Parallel);
    poleY=-latlongToGrid(maporigin).gety();
  }
}

double LambertConicSphere::scaleRatioLog(double Parallel0,double Parallel1)
{
  latlong ll;
  double ret;
  ll.lon=centralMeridian;
  ll.lat=Parallel0;
  ret=log(scaleFactor(ll));
  ll.lat=Parallel1;
  ret-=log(scaleFactor(ll));
  return ret;
}

latlong LambertConicSphere::gridToLatlong(xy grid)
{
  double angle,radius;
  latlong ret;
  grid=(grid-offset)/scale;
  if (exponent==0)
  {
    angle=grid.east()/ellip->geteqr();
    radius=exp(-grid.north()/ellip->getpor());
  }
  else
  {
    angle=atan2(grid.east(),poleY-grid.north())/exponent;
    radius=hypot(grid.east(),poleY-grid.north());
    radius=pow(radius/ellip->getpor()*exponent/coneScale,1/exponent);
  }
  ret.lat=M_PIl/2-2*atan(radius);
  ret.lon=angle+centralMeridian;
  return ret;
}

xyz LambertConicSphere::gridToGeocentric(xy grid)
{
  return ellip->geoc(gridToLatlong(grid),0);
}

xy LambertConicSphere::geocentricToGrid(xyz geoc)
{
  latlongelev lle=ellip->geod(geoc);
  latlong ll(lle);
  return latlongToGrid(ll);
}

xy LambertConicSphere::latlongToGrid(latlong ll)
{
  double radius,angle,northing,easting;
  radius=tan((M_PIl/2-ll.lat)/2);
  angle=ll.lon-centralMeridian;
  while(angle>M_PIl*2)
    angle-=M_PIl*2;
  while(angle<-M_PIl*2)
    angle+=M_PIl*2;
  /* TODO: if exponent is small, say less than 1/64, then use a complex
   * function similar to expm1 on the Mercator coordinates.
   */
  if (exponent==0)
  {
    easting=angle*ellip->geteqr();
    northing=-log(radius)*ellip->getpor();
  }
  else
  {
    radius=pow(radius,exponent)*ellip->getpor()/exponent*coneScale;
    angle*=exponent;
    easting=radius*sin(angle);
    northing=poleY-radius*cos(angle);
  }
  return xy(easting,northing)*scale+offset;
}

double LambertConicSphere::scaleFactor(xy grid)
{
  return scaleFactor(gridToLatlong(grid));
}

double LambertConicSphere::scaleFactor(latlong ll)
{
  double coneradius,cenconeradius,parradius,cenparradius;
  coneradius=tan((M_PIl/2-ll.lat)/2);
  cenconeradius=tan((M_PIl/2-centralParallel)/2);
  parradius=(ellip->geoc(ll.lat,0.,0.)).getx()/ellip->geteqr();
  cenparradius=(ellip->geoc(centralParallel,0.,0.)).getx()/ellip->geteqr();
  return pow(coneradius/cenconeradius,exponent)*cenparradius/parradius*scale;
}

void LambertConicEllipsoid::setParallel(double Parallel)
{
  Parallel=ellip->conformalLatitude(Parallel);
  centralParallel=Parallel;
  exponent=sin(Parallel);
  if (exponent==0)
    coneScale=1;
  else if (fabs(exponent)==1)
    coneScale=2;
  else
    coneScale=cos(Parallel)/pow(tan((M_PIl/2-Parallel)/2),exponent);
  //cout<<"Parallel "<<radtodeg(Parallel)<<" coneScale "<<coneScale<<endl;
}

LambertConicEllipsoid::LambertConicEllipsoid():Projection()
{
  centralMeridian=0;
  setParallel(0);
  poleY=INFINITY;
}

LambertConicEllipsoid::LambertConicEllipsoid(ellipsoid *e,double Meridian,double Parallel):Projection()
{
  latlong maporigin;
  centralMeridian=Meridian;
  ellip=e;
  setParallel(Parallel);
  poleY=0;
  maporigin=latlong(Meridian,Parallel);
  poleY=-latlongToGrid(maporigin).gety();
}

LambertConicEllipsoid::LambertConicEllipsoid(ellipsoid *e,double Meridian,double Parallel0,double Parallel1,latlong zll,xy zxy):Projection()
{
  latlong maporigin;
  brent br;
  latlong ll;
  double ratiolog0,ratiolog1,Parallel;
  centralMeridian=Meridian;
  ellip=e;
  setParallel(Parallel0);
  ratiolog0=scaleRatioLog(Parallel0,Parallel1);
  setParallel(Parallel1);
  ratiolog1=scaleRatioLog(Parallel0,Parallel1);
  /* Setting one parallel to ±90° and the other to something else, or either
   * or both to ±(>90°), is an error. I'm not sure whether to throw an exception
   * or set the state to invalid. I'm now setting it to invalid.
   * If the parallels are 45° and 90°, different computers give wildly different
   * results: 84.714395° on Linux/Intel, but 67.5° on Linux/ARM and DFBSD.
   * This is because M_PIl!=M_PI, resulting in M_PIl/2-ll.lat being tiny but
   * positive when ll.lat is M_PIl rounded to double.
   * If the parallels are 45° and DEG90-1 (89.999999832°), the three computers
   * agree that Parallel is 82.686083 (but no more precisely), so that is not
   * an error.
   */
  if ((Parallel0!=Parallel1 && (radtobin(fabs(Parallel0))==DEG90 || radtobin(fabs(Parallel1))==DEG90))
      || fabs(Parallel0)>M_PIl/2
      || fabs(Parallel1)>M_PIl/2)
  {
    centralParallel=poleY=exponent=coneScale=NAN;
    cerr<<"Invalid parallels in LambertConicEllipsoid"<<endl;
  }
  else
  {
    Parallel=br.init(Parallel0,ratiolog0,Parallel1,ratiolog1,false);
    while (!br.finished())
    {
      cout<<"Parallel "<<ldecimal(radtodeg(Parallel))<<endl;
      setParallel(Parallel);
      Parallel=br.step(scaleRatioLog(Parallel0,Parallel1));
    }
    setParallel(Parallel);
    ll.lon=centralMeridian;
    ll.lat=Parallel0;
    scale=1/scaleFactor(ll);
    poleY=0;
    maporigin=latlong(Meridian,Parallel);
    poleY=-latlongToGrid(maporigin).gety();
  }
  offset=zxy-latlongToGrid(zll);
}

double LambertConicEllipsoid::scaleRatioLog(double Parallel0,double Parallel1)
{
  latlong ll;
  double ret;
  ll.lon=centralMeridian;
  ll.lat=Parallel0;
  ret=log(scaleFactor(ll));
  ll.lat=Parallel1;
  ret-=log(scaleFactor(ll));
  return ret;
}

latlong LambertConicEllipsoid::gridToLatlong(xy grid)
{
  double angle,radius;
  latlong ret;
  grid=(grid-offset)/scale;
  if (exponent==0)
  {
    angle=grid.east()/ellip->sphere->geteqr();
    radius=exp(-grid.north()/ellip->sphere->getpor());
  }
  else
  {
    angle=atan2(grid.east(),poleY-grid.north())/exponent;
    radius=hypot(grid.east(),poleY-grid.north());
    radius=pow(radius/ellip->sphere->getpor()*exponent/coneScale,1/exponent);
  }
  ret.lat=M_PIl/2-2*atan(radius);
  ret.lon=angle+centralMeridian;
  ret=ellip->inverseConformalLatitude(ret);
  return ret;
}

xyz LambertConicEllipsoid::gridToGeocentric(xy grid)
{
  return ellip->geoc(gridToLatlong(grid),0);
}

xy LambertConicEllipsoid::geocentricToGrid(xyz geoc)
{
  latlongelev lle=ellip->geod(geoc);
  latlong ll(lle);
  return latlongToGrid(ll);
}

xy LambertConicEllipsoid::latlongToGrid(latlong ll)
{
  double radius,angle,northing,easting;
  ll=ellip->conformalLatitude(ll);
  radius=tan((M_PIl/2-ll.lat)/2);
  angle=ll.lon-centralMeridian;
  while(angle>M_PIl*2)
    angle-=M_PIl*2;
  while(angle<-M_PIl*2)
    angle+=M_PIl*2;
  /* TODO: if exponent is small, say less than 1/64, then use a complex
   * function similar to expm1 on the Mercator coordinates.
   */
  if (exponent==0)
  {
    easting=angle*ellip->sphere->geteqr();
    northing=-log(radius)*ellip->sphere->getpor();
  }
  else
  {
    radius=pow(radius,exponent)*ellip->sphere->getpor()/exponent*coneScale;
    angle*=exponent;
    easting=radius*sin(angle);
    northing=poleY-radius*cos(angle);
  }
  return xy(easting,northing)*scale+offset;
}

double LambertConicEllipsoid::scaleFactor(xy grid)
{
  return scaleFactor(gridToLatlong(grid));
}

double LambertConicEllipsoid::scaleFactor(latlong ll)
{
  double coneradius,cenconeradius,parradius,cenparradius;
  latlong sphll=ellip->conformalLatitude(ll);
  coneradius=tan((M_PIl/2-sphll.lat)/2);
  cenconeradius=tan((M_PIl/2-centralParallel)/2);
  parradius=(ellip->sphere->geoc(sphll.lat,0.,0.)).getx()/ellip->sphere->geteqr();
  cenparradius=(ellip->sphere->geoc(centralParallel,0.,0.)).getx()/ellip->sphere->geteqr();
  return pow(coneradius/cenconeradius,exponent)*
         cenparradius/parradius*scale/ellip->scaleFactor(ll.lat,sphll.lat);
}

/* North Carolina state plane, original:
 * ellipsoid Clarke
 * central meridian -79°
 * parallels 34°20' and 36°10'
 * 33°45'N 79°W = (609601.219202438405,0)
 * current:
 * ellipsoid GRS80
 * everything else the same
 */

LambertConicEllipsoid *readConformalConic(istream &file)
/* Reads data such as the following from a file and returns a pointer to a
 * new projection.
 * 
 * Ellipsoid:Clarke
 * Meridian:79W
 * Parallel:34°20'N
 * Parallel:36°10'N
 * OriginLL:33°45'N 79°W
 * OriginXY:609601.219202438405,0
 * 
 * If there are two parallels, there must be at least one line after the parallels,
 * else it will return without reading the second parallel. The order of parallels
 * does not matter.
 * 
 * If the parallels are equal except that one is north and the other south, or there
 * is only one parallel which is the equator, the result is a Mercator projection.
 * If the parallel is 90°, the result is a stereographic projection. If one is
 * 90° and the other is anything else, including the other 90°, it's an error.
 */
{
  int fieldsSeen=0;
  ellipsoid *ellip=nullptr;
  size_t hashpos,colonpos;
  string line,tag,value,ellipsoidStr;
  vector<double> parallels;
  double meridian;
  latlong ll,origll;
  xy origxy;
  Measure metric;
  LambertConicEllipsoid *ret=nullptr;
  metric.setMetric();
  metric.setDefaultUnit(LENGTH,1);
  while (fieldsSeen!=0x295 && fieldsSeen!=0x2a5 && (fieldsSeen&0xd4a)==0)
  {
    getline(file,line);
    hashpos=line.find('#');
    if (hashpos==0)
      line="";
    if (line=="")
      fieldsSeen|=800; // blank line is invalid
    else
    {
      colonpos=line.find(':');
      if (colonpos>line.length())
	fieldsSeen=-1;
      else
      {
	tag=line.substr(0,colonpos);
	value=line.substr(colonpos+1);
	if (tag=="Ellipsoid")
	{
	  ellipsoidStr=value;
	  ellip=getEllipsoid(ellipsoidStr);
	  fieldsSeen+=1;
	}
	else if (tag=="Meridian")
	{
	  ll=parselatlong(value,DEGREE);
	  meridian=ll.lon;
	  fieldsSeen+=4;
	}
	else if (tag=="Parallel")
	{
	  ll=parselatlong(value,DEGREE);
	  parallels.push_back(ll.lat);
	  fieldsSeen+=16;
	}
	else if (tag=="OriginLL")
	{
	  origll=parselatlong(value,DEGREE);
	  fieldsSeen+=128;
	}
	else if (tag=="OriginXY")
	{
	  origxy=metric.parseXy(value);
	  fieldsSeen+=512;
	}
	else
	  fieldsSeen+=2048;
      }
    }
  }
  if ((fieldsSeen==0x295 || fieldsSeen==0x2a5) && ellip)
    ret=new LambertConicEllipsoid(ellip,meridian,parallels[0],parallels.back(),origll,origxy);
  return ret;
}

const Quaternion rotateStereographic(1/14.,5/14.,7/14.,11/14.);
const Quaternion unrotateStereographic(-1/14.,5/14.,7/14.,11/14.);
/* This rotates (-96/196,-164/196,-48/196) to the South Pole, which is then
 * projected to infinity. This point is:
 * (-3120489.796,-5330836.735,-1560244.898) in ECEF coordinates,
 * (5,-0.292682926829,0.585365853659) in volleyball coordinates,
 * 14.1758035159S 120.343248884W in lat-long degrees,
 * 14°10'32.9"S 120°20'35.7"W in lat-long DMS,
 * 84561961.799S 717875442.017W in lat-long integer coordinates.
 * This point is in the Pacific Ocean over a megameter from land. It is highly
 * unlikely to be near any geoid file boundary, and neither a boldatni boundary
 * nor a cylinterval boundary can exactly hit it.
 */

StereographicSphere::StereographicSphere():Projection()
{
  rotation=1;
}

StereographicSphere::StereographicSphere(Quaternion Rotation):Projection()
{
  rotation=Rotation;
}

latlong StereographicSphere::gridToLatlong(xy grid)
{
  return gridToGeocentric(grid).latlon();
}

xyz StereographicSphere::gridToGeocentric(xy grid)
{
  double sf=scaleFactor(grid);
  return rotation.conj().rotate(xyz(grid/sf,ellip->getpor()*(2/sf-1)));
}

xy StereographicSphere::geocentricToGrid(xyz geoc)
{
  geoc=rotation.rotate(geoc);
  geoc.normalize();
  return xy(geoc)*ellip->getpor()*2/(geoc.getz()+1);
}

xy StereographicSphere::latlongToGrid(latlong ll)
{
  return geocentricToGrid(ellip->geoc(ll,0));
}

double StereographicSphere::scaleFactor(xy grid)
{
  return 1+sqr(grid.length()/2/ellip->getpor());
}

double StereographicSphere::scaleFactor(latlong ll)
{
  return scaleFactor(latlongToGrid(ll));
}

StereographicSphere sphereStereoArabianSea(rotateStereographic);

bool ProjectionLabel::match(const ProjectionLabel &b,bool prefix)
/* Returns true if b matches this pattern, e.g.
 * ("U","N","","NAD").match(("US","NC","","NAD83"),true)=true
 */
{
  int maxpos;
  if (prefix)
    maxpos=0;
  else
    maxpos=b.country.length()+b.province.length()+b.zone.length()+b.version.length();
  return b.country.find(country)<=maxpos &&
         b.province.find(province)<=maxpos &&
         b.zone.find(zone)<=maxpos &&
         b.version.find(version)<=maxpos;
}

bool operator<(const ProjectionLabel a,const ProjectionLabel b)
{
  if (a.country!=b.country)
    return a.country<b.country;
  else if (a.province!=b.province)
    return a.province<b.province;
  else if (a.zone!=b.zone)
    return a.zone<b.zone;
  else
    return a.version<b.version;
}

ProjectionLabel readProjectionLabel(istream &file)
{
  int fieldsSeen=0;
  size_t hashpos,colonpos;
  string line,tag,value;
  ProjectionLabel ret;
  while (fieldsSeen!=0x55 && (fieldsSeen&0x1aa)==0 && file.good())
  {
    getline(file,line);
    hashpos=line.find('#');
    if (hashpos==0)
      line="";
    if (line=="")
      fieldsSeen*=2; // blank line when some but not all fields are seen is invalid
    else
    {
      colonpos=line.find(':');
      if (colonpos>line.length())
	fieldsSeen=-1;
      else
      {
	tag=line.substr(0,colonpos);
	value=line.substr(colonpos+1);
	if (tag=="Country")
	{
	  ret.country=value;
	  fieldsSeen+=1;
	}
	else if (tag=="State" || tag=="Province" || tag=="Krai" || tag=="Okrug")
	{
	  ret.province=value;
	  fieldsSeen+=4;
	}
	else if (tag=="Zone")
	{
	  ret.zone=value;
	  fieldsSeen+=16;
	}
	else if (tag=="Version")
	{
	  ret.version=value;
	  fieldsSeen+=64;
	}
	else
	  fieldsSeen+=256;
      }
    }
  }
  return ret;
}

g1boundary parseBoundary(string bdy)
{
  string llStr;
  latlong ll;
  size_t spacepos;
  g1boundary ret;
  while (bdy.length())
  {
    spacepos=bdy.find(' ');
    if (spacepos<bdy.length())
      llStr+=bdy.substr(0,++spacepos);
    else
      llStr+=bdy;
    bdy.erase(0,spacepos);
    ll=parselatlong(llStr,DEGREE);
    if (ll.valid()==0)
    {
      cerr<<"Unparsable latlong: "<<llStr<<endl;
      llStr="";
    }
    if (ll.valid()==2)
    {
      ret.push_back(encodedir(Sphere.geoc(ll,0)));
      llStr="";
    }
  }
  return ret;
}

Projection *readProjection(istream &file)
{
  int fieldsSeen=0,projectionType=0;
  size_t hashpos,colonpos;
  string line,tag,value;
  Projection *ret=nullptr;
  while ((fieldsSeen&0x11)==0)
  {
    getline(file,line);
    hashpos=line.find('#');
    if (hashpos==0)
      line="";
    if (line=="")
      fieldsSeen|=16; // blank line when some but not all fields are seen is invalid
    else
    {
      colonpos=line.find(':');
      if (colonpos>line.length())
	fieldsSeen=-1;
      else
      {
	tag=line.substr(0,colonpos);
	value=line.substr(colonpos+1);
	if (tag=="Projection")
	{
	  fieldsSeen|=1;
	  if (value=="CC")
	    projectionType=PROJ_CC;
	  else if (value=="TM")
	    projectionType=PROJ_TM;
	  else if (value=="OM")
	    projectionType=PROJ_OM;
	  else
	    fieldsSeen|=16;
	}
      }
    }
  }
  switch (projectionType)
  {
    case PROJ_CC:
      ret=readConformalConic(file);
      break;
    case PROJ_TM:
      //ret=readTransverseMercator(file);
      break;
    case PROJ_OM:
      //ret=readObliqueMercator(file);
      break;
  }
  getline(file,line);
  colonpos=line.find(':');
  tag=line.substr(0,colonpos);
  value=line.substr(colonpos+1);
  if (ret && tag=="Boundary")
    ret->setBoundary(parseBoundary(value));
  return ret;
}

void ProjectionList::insert(ProjectionLabel label,Projection *proj)
/* Takes ownership of proj. Do not delete proj; the ProjectionList will delete
 * it when the last ProjectionList containing it is destroyed.
 */
{
  projList[label]=shared_ptr<Projection>(proj);
}

Projection *ProjectionList::operator[](int n)
{
  Projection *ret=nullptr;
  map<ProjectionLabel,shared_ptr<Projection> >::iterator i;
  int j;
  for (i=projList.begin(),j=0;i!=projList.end();i++,j++)
    if (j==n)
      ret=i->second.get();
  return ret;
}

ProjectionList ProjectionList::matches(ProjectionLabel pattern)
{
  ProjectionList ret;
  map<ProjectionLabel,shared_ptr<Projection> >::iterator i;
  for (i=projList.begin();i!=projList.end();i++)
    if (pattern.match(i->first))
      ret.projList[i->first]=i->second;
  return ret;
}

ProjectionList ProjectionList::cover(latlong ll)
// Returns a list of projections whose boundaries contain the given point.
{
  ProjectionList ret;
  map<ProjectionLabel,shared_ptr<Projection> >::iterator i;
  for (i=projList.begin();i!=projList.end();i++)
    if (i->second->in(ll))
      ret.projList[i->first]=i->second;
  return ret;
}

ProjectionList ProjectionList::cover(vball v)
{
  ProjectionList ret;
  map<ProjectionLabel,shared_ptr<Projection> >::iterator i;
  for (i=projList.begin();i!=projList.end();i++)
    if (i->second->in(v))
      ret.projList[i->first]=i->second;
  return ret;
}

void ProjectionList::readFile(istream &file)
{
  ProjectionLabel label;
  Projection *proj;
  while (file.good())
  {
    label=readProjectionLabel(file);
    proj=readProjection(file);
    if (proj)
      insert(label,proj);
  }
}
