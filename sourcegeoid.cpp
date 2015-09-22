/******************************************************/
/*                                                    */
/* sourcegeoid.cpp - geoidal undulation source data   */
/*                                                    */
/******************************************************/
#include "sourcegeoid.h"
using namespace std;

double geolattice::elev(int lat,int lon)
{
  int easting,northing,eint,nint;
  double epart,npart,ne,nw,se,sw,ret;
  easting=(lon-wbd)&0x7fffffff;
  northing=lat-sbd;
  epart=(double)easting*width/(ebd-wbd);
  npart=(double)northing*height/(nbd-sbd);
  eint=trunc(epart);
  nint=trunc(npart);
  epart-=eint;
  npart-=nint;
  if (eint==width && epart==0)
  {
    eint--;
    epart=1;
  }
  if (nint==height && npart==0)
  {
    nint--;
    npart=1;
  }
  npart=1-npart;
  epart=1-epart;
  npart=1-npart;
  epart=1-epart;
  sw=undula[(width+1)*nint+eint];
  se=undula[(width+1)*nint+eint+1];
  nw=undula[(width+1)*(nint+1)+eint];
  ne=undula[(width+1)*(nint+1)+eint+1];
  if (sw==-2147483648)
    sw=1e30;
  if (se==-2147483648)
    se=1e30;
  if (nw==-2147483648)
    nw=1e30;
  if (ne==-2147483648)
    ne=1e30;
  ret=((sw*(1-epart)+se*epart)*(1-npart)+(nw*(1-epart)+ne*epart)*npart)/65536;
  if (ret>8850 || ret<-11000)
    ret=NAN;
  return ret;
}

double geolattice::elev(xyz dir)
{
  return elev(dir.lati(),dir.loni());
}
