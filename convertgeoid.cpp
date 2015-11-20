/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
/*                                                    */
/******************************************************/
#define CONVERTGEOID
#include <iostream>
#include "geoid.h"
#include "sourcegeoid.h"
#include "document.h"
#include "raster.h"
using namespace std;

document doc;
vector<geolattice> geo;

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 65536/12937
 * 4: 65536/7225
 * 5: 65536/12937
 */

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
}

int main(int argc, char *argv[])
{
  geo.resize(3);
  readusngsbin(geo[0],"../g2012bu0.bin");
  readusngsbin(geo[1],"../g2012ba0.bin");
  readusngsbin(geo[2],"../g2012bh0.bin");
  outund("Green Hill",degtobin(35.4),degtobin(-82.05));
  outund("Charlotte",degtobin(35.22),degtobin(-80.84));
  outund("Kitimat",degtobin(54.0547),degtobin(-128.6578)); // in the overlap of two files
  outund("Denali",degtobin(63.0695),degtobin(-151.0074));
  outund("Haleakala",degtobin(20.7097),degtobin(-156.2533));
  drawglobecube(100,1,"geoid.ppm");
  return 0;
}