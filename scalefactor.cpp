/******************************************************/
/*                                                    */
/* scalefactor.cpp - elevation and grid scale factors */
/*                                                    */
/******************************************************/

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
#include "icommon.h"
#include "bezitopo.h"

using namespace std;

void scalefactor_i(string args)
{
  string llstr,elevstr;
  subcont=true;
  do
  {
    cout<<"LatLong> ";
    cout.flush();
    getline(cin,llstr);
    if (llstr.length())
    {
      cout<<"Elev> ";
      cout.flush();
      getline(cin,elevstr);
      if (elevstr.length())
      {
      }
      else
	subcont=false;
    }
    else
      subcont=false;
  }
  while (subcont);
}
