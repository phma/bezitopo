/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
/*                                                    */
/******************************************************/
#include <iostream>
#include "geoid.h"
#include "sourcegeoid.h"
#include "document.h"
#include "raster.h"
#include "hlattice.h"
#include "relprime.h"
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

/* Check the square for the presence of geoid data by interrogating it with a
 * hexagonal lattice. The size of the hexagon is sqrt(2/3) times the length
 * of the square (sqrt(1/2) to get the half diagonal of the square, sqrt(4/3)
 * to get the radius from the apothem), except for the whole face, where it
 * is (1+sqrt(1/3))/2 times the length of the square, since two sides of the
 * hexagon are parallel to two sides of the square. The process continues
 * until the entire square has been interrogated or there are at least one
 * point in nan and one point in num.
 * 
 * This procedure doesn't return anything. Use geoquad::isfull. It is possible
 * that interrogating finds a square full, but one of the 256 points used to
 * compute the coefficients is NaN.
 */
void interroquad(geoquad &quad,double spacing)
{
  xyz corner(3678298.565,3678298.565,3678298.565),ctr,xvec,yvec,tmp;
  int radius,i,n,rp;
  double qlen,hradius;
  ctr=quad.centeronearth();
  xvec=corner*ctr;
  yvec=xvec*ctr;
  xvec/=xvec.length();
  yvec/=yvec.length();
  tmp=(2+M_SQRT_3)*yvec+xvec;
  xvec-=yvec;
  yvec=tmp/tmp.length();
  xvec/=xvec.length();
  // xvec and yvec are now at 120° to match the components of hvec
  qlen=quad.length();
  if (qlen>1e7)
    hradius=qlen*(1+M_SQRT_1_3)/2;
  else
    hradius=qlen*M_SQRT_2_3;
  if (spacing<1)
    spacing=1;
  radius=rint(hradius/spacing);
  if (radius>26754)
  {
    radius=26754; // largest hexagon with <2147483648 points
    spacing=hradius/radius;
  }
  hlattice hlat(radius);
  xvec*=spacing;
  yvec*=spacing;
  rp=relprime(hlat.nelts);
}

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
}

int main(int argc, char *argv[])
{
  geo.resize(6);
  readusngsbin(geo[0],"../g2012bu0.bin");
  readusngsbin(geo[1],"../g2012ba0.bin");
  readusngsbin(geo[2],"../g2012bh0.bin");
  readusngsbin(geo[3],"../g2012bg0.bin");
  readusngsbin(geo[4],"../g2012bp0.bin");
  readusngsbin(geo[5],"../g2012bs0.bin");
  outund("Green Hill",degtobin(35.4),degtobin(-82.05));
  outund("Charlotte",degtobin(35.22),degtobin(-80.84));
  outund("Kitimat",degtobin(54.0547),degtobin(-128.6578)); // in the overlap of two files
  outund("Denali",degtobin(63.0695),degtobin(-151.0074));
  outund("Haleakala",degtobin(20.7097),degtobin(-156.2533));
  drawglobecube(1024,62,-7,1,0,"geoid.ppm");
  return 0;
}
