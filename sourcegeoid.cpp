/******************************************************/
/*                                                    */
/* sourcegeoid.cpp - geoidal undulation source data   */
/*                                                    */
/******************************************************/
#include <fstream>
#include <iostream>
#include "sourcegeoid.h"
#include "binio.h"

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

void readusngsbinheaderbe(usngsheader &hdr,fstream &file)
{
  hdr.south=readbedouble(file);
  hdr.west=readbedouble(file);
  hdr.latspace=readbedouble(file);
  hdr.longspace=readbedouble(file);
  hdr.nlat=readbeint(file);
  hdr.nlong=readbeint(file);
  hdr.dtype=readbeint(file);
}

void readusngsbinheaderle(usngsheader &hdr,fstream &file)
{
  hdr.south=readledouble(file);
  hdr.west=readledouble(file);
  hdr.latspace=readledouble(file);
  hdr.longspace=readledouble(file);
  hdr.nlat=readleint(file);
  hdr.nlong=readleint(file);
  hdr.dtype=readleint(file);
}

bool sanitycheck(usngsheader &hdr)
{
  bool ssane,wsane,latsane,longsane,nlatsane,nlongsane,typesane;
  ssane=hdr.south>-360.0001 && hdr.south<360.0001 && (hdr.south==0 || fabs(hdr.south)>0.000001);
  wsane=hdr.west>-360.0001 && hdr.west<360.0001 && (hdr.west==0 || fabs(hdr.west)>0.000001);
  latsane=hdr.latspace>0.000001 && hdr.latspace<190;
  longsane=hdr.longspace>0.000001 && hdr.longspace<190;
  nlatsane=hdr.nlat>0 && hdr.nlat*hdr.latspace<180.000001;
  nlongsane=hdr.nlong>0 && hdr.nlong*hdr.longspace<360.000001;
  typesane=hdr.dtype<256;
  return ssane && wsane && latsane && longsane && nlatsane && nlongsane && typesane;
}

int readusngsbin(geolattice &geo,string filename)
{
  fstream file;
  usngsheader hdr;
  bool bigendian;
  file.open(filename,fstream::in|fstream::binary);
  readusngsbinheaderle(hdr,file);
  if (sanitycheck(hdr))
    bigendian=false;
  else
  {
    file.seekg(0);
    readusngsbinheaderbe(hdr,file);
    bigendian=true;
  }
  if (sanitycheck(hdr))
    cout<<"Header sane"<<endl;
  cout<<"South "<<hdr.south<<" West "<<hdr.west<<endl;
  cout<<"Latitude spacing "<<hdr.latspace<<" Longitude spacing "<<hdr.longspace<<endl;
  cout<<"Rows "<<hdr.nlat<<" Columns "<<hdr.nlong<<endl;
  file.close();
  return 0;
}
