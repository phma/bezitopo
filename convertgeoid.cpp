/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
/*                                                    */
/******************************************************/
#include <iostream>
#include "geoid.h"
#include "sourcegeoid.h"
#include "document.h"
using namespace std;

document doc;

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 65536/12937
 * 4: 65536/7225
 * 5: 65536/12937
 */

int main(int argc, char *argv[])
{
  geolattice geo;
  readusngsbin(geo,"../g2012bu0.bin");
  cout<<"Undulation in Green Hill is "<<geo.elev(degtobin(35.4),degtobin(-82.05))<<endl;
  cout<<"Undulation in Charlotte is "<<geo.elev(degtobin(35.22),degtobin(-80.84))<<endl;
  return 0;
}