/******************************************************/
/*                                                    */
/* scalefactor.cpp - elevation and grid scale factors */
/*                                                    */
/******************************************************/
/* Copyright 2016,2017 Pierre Abbat.
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

/* This handles just elevation factors so far. For the grid factor, I'll have
 * to write the map projection.
 */

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "point.h"
#include "geoid.h"
#include "angle.h"
#include "ldecimal.h"
#include "icommon.h"
#include "bezitopo.h"

using namespace std;

void scalefactor_i(string args)
{
  string llstr,elevstr;
  latlong ll;
  double separation,elevation,radius,elevfactor,gridfactor;
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
              radius=GRS80.radiusAtLatitude(ll,DEG45);
              cout<<"Average radius of curvature is "<<doc.ms.formatMeasurementUnit(radius,LENGTH)<<endl;
              elevfactor=radius/(radius+elevation+separation);
              cout<<"Elevation factor is "<<ldecimal(elevfactor)<<endl;
            }
            catch (int e)
            {
              cerr<<"Elevation should be a length"<<endl;
            }
	  }
	  else
	    subcont=false;
	}
	else
	  cout<<"I don't know the geoid separation there."<<endl;
      }
      else
	cout<<"Malformatted latitude or longitude"<<endl;
    }
    else
      subcont=false;
  }
  while (subcont);
}
