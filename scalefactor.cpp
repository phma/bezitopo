/******************************************************/
/*                                                    */
/* scalefactor.cpp - elevation and grid scale factors */
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

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "point.h"
#include "geoid.h"
#include "angle.h"
#include "ldecimal.h"
#include "projection.h"
#include "icommon.h"
#include "bezitopo.h"
#include "except.h"

using namespace std;

string oneString_i(vector<string> strList)
// May make this use unbuffered input later.
{
  int i,chosen=0;
  string line;
  if (strList.size()>1)
    do
    {
      for (i=0;i<strList.size();i++)
	cout<<i<<". "<<strList[i]<<'\n';
      cout<<"? ";
      cout.flush();
      getline(cin,line);
      chosen=stod(line);
    } while (chosen<0 || chosen>=strList.size());
  if (strList.size())
    return strList[chosen];
  else
    return "";
}

Projection *oneProj(ProjectionList projList)
{
  ProjectionList subList;
  ProjectionLabel label;
  vector<string> strList;
  strList=projList.listCountries();
  label.country=oneString_i(strList);
  subList=projList.matches(label);
  strList=subList.listProvinces();
  if (strList.size()>1)
    cout<<label.country<<"-\n";
  label.province=oneString_i(strList);
  subList=projList.matches(label);
  strList=subList.listZones();
  if (strList.size()>1)
    cout<<label.country<<'-'<<label.province<<"-\n";
  label.zone=oneString_i(strList);
  subList=projList.matches(label);
  strList=subList.listVersions();
  if (strList.size()>1)
    cout<<label.country<<'-'<<label.province<<'-'<<label.zone<<"-\n";
  label.version=oneString_i(strList);
  subList=projList.matches(label);
  return subList[0];
}

void scalefactorll_i(string args)
{
  string llstr,elevstr;
  latlong ll;
  double separation,elevation,radius,elevfactor,gridfactor;
  ProjectionList possibleProjections;
  Projection *chosenProjection;
  ellipsoid *ellip=&WGS84;
  xy gridCoords;
  subcont=true;
  do
  {
    cout<<"LatLong> ";
    cout.flush();
    getline(cin,llstr);
    if (llstr.length())
    {
      ll=parselatlong(llstr,DEGREE);
      if (std::isfinite(ll.lat) && std::isfinite(ll.lon))
      {
	chosenProjection=oneProj(allProjections.cover(ll));
	elevfactor=gridfactor=NAN;
	separation=cube.undulation(ll);
	if (std::isfinite(separation))
	{
	  cout<<"Elev> ";
	  cout.flush();
	  getline(cin,elevstr);
	  if (elevstr.length())
	  {
            try
            {
              elevation=doc.ms.parseMeasurement(elevstr,LENGTH).magnitude;
              cout<<"Geoid is "<<doc.ms.formatMeasurementUnit(separation,LENGTH)<<" above ellipsoid"<<endl;
	      if (chosenProjection)
		ellip=chosenProjection->ellip;
              radius=ellip->radiusAtLatitude(ll,DEG45);
              cout<<"Average radius of curvature is "<<doc.ms.formatMeasurementUnit(radius,LENGTH)<<endl;
              elevfactor=radius/(radius+elevation+separation);
              cout<<"Elevation factor is "<<ldecimal(elevfactor)<<endl;
            }
            catch (BeziExcept e)
            {
              cerr<<"Elevation should be a length"<<endl;
            }
	  }
	  else
	    subcont=false;
	}
	else
	  cout<<"I don't know the geoid separation there."<<endl;
	if (chosenProjection)
	{
	  gridCoords=chosenProjection->latlongToGrid(ll);
	  gridfactor=chosenProjection->scaleFactor(ll);
	  cout<<"Grid coordinates are "<<doc.ms.formatMeasurement(gridCoords.east(),LENGTH)
	      <<','<<doc.ms.formatMeasurementUnit(gridCoords.north(),LENGTH)<<endl;
	  cout<<"Grid factor is "<<ldecimal(gridfactor)<<endl;
	}
	if (std::isfinite(gridfactor*elevfactor))
	  cout<<"Combined factor is "<<ldecimal(gridfactor*elevfactor)<<endl;
      }
      else
	cout<<"Malformatted latitude or longitude"<<endl;
    }
    else
      subcont=false;
  }
  while (subcont);
}

void scalefactorxy_i(string args)
{
  string xystr,elevstr;
  latlong ll;
  double separation,elevation,radius,elevfactor,gridfactor;
  ProjectionList possibleProjections;
  Projection *chosenProjection;
  ellipsoid *ellip=&WGS84;
  xy gridCoords;
  subcont=true;
  chosenProjection=oneProj(allProjections);
  do
  {
    cout<<"Grid> ";
    cout.flush();
    getline(cin,xystr);
    if (xystr.length())
    {
      gridCoords=parsexy(xystr);
      if (gridCoords.isnan())
	cout<<"Malformatted grid coordinates\n";
      else
      {
	elevfactor=gridfactor=NAN;
	if (chosenProjection)
	{
	  ll=chosenProjection->gridToLatlong(gridCoords);
	  gridfactor=chosenProjection->scaleFactor(gridCoords);
	  cout<<formatlatlong(ll,DEGREE+SEXAG2P2)<<"  "<<formatlatlong(ll,DEGREE+6)<<endl;
	  cout<<"Grid factor is "<<ldecimal(gridfactor)<<endl;
	  if (!chosenProjection->in(ll))
	    cout<<"Point is not in the projection's boundary\n";
	}
	else
	{
	  cout<<"Projection file is missing\n";
	  subcont=false;
	}
	separation=cube.undulation(ll);
	if (std::isfinite(separation))
	{
	  cout<<"Elev> ";
	  cout.flush();
	  getline(cin,elevstr);
	  if (elevstr.length())
	  {
            try
            {
              elevation=doc.ms.parseMeasurement(elevstr,LENGTH).magnitude;
              cout<<"Geoid is "<<doc.ms.formatMeasurementUnit(separation,LENGTH)<<" above ellipsoid"<<endl;
	      if (chosenProjection)
		ellip=chosenProjection->ellip;
              radius=ellip->radiusAtLatitude(ll,DEG45);
              cout<<"Average radius of curvature is "<<doc.ms.formatMeasurementUnit(radius,LENGTH)<<endl;
              elevfactor=radius/(radius+elevation+separation);
              cout<<"Elevation factor is "<<ldecimal(elevfactor)<<endl;
            }
            catch (BeziExcept e)
            {
              cerr<<"Elevation should be a length"<<endl;
            }
	  }
	}
	else
	  cout<<"I don't know the geoid separation there."<<endl;
	if (std::isfinite(gridfactor*elevfactor))
	  cout<<"Combined factor is "<<ldecimal(gridfactor*elevfactor)<<endl;
      }
    }
    else
      subcont=false;
  }
  while (subcont);
}
