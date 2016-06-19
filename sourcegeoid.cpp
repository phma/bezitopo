/******************************************************/
/*                                                    */
/* sourcegeoid.cpp - geoidal undulation source data   */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#include <fstream>
#include <iostream>
#include <iomanip>
#include "sourcegeoid.h"
#include "binio.h"
#include "bicubic.h"
#include "manysum.h"

using namespace std;
vector<geolattice> geo;
map<int,matrix> quadinv;

bool smooth5(unsigned n)
/* Used for deciding the number of divisions of a circle in a lat-long grid.
 * This is in practice always a 5-smooth number, such as 21600.
 * 21600 (1 arc minute) is between 20736 and 21870.
 * 1440 (1/4 degree, 1 time minute) is between 1350 and 1458.
 */
{
  if (n==0)
    n=7; // 0 is not smooth, as it is divisible by all primes. It also cannot be the number of divisions.
  while ((n%2)==0)
    n/=2;
  while ((n%3)==0)
    n/=3;
  while ((n%5)==0)
    n/=5;
  return n==1;
}

string readword(istream &file)
{
  int ch;
  string ret;
  int state=0;
  do
  {
    ch=file.get();
    switch (state)
    {
      case 0:
	if (!isspace(ch))
	  state=1;
	if (ch<0)
	  state=2;
	break;
      case 1:
	if (isspace(ch) || ch<0)
	  state=2;
    }
    if (state==1)
      ret+=ch;
  } while (state<2);
  return ret;
}

double readdouble(istream &file)
// This can throw.
{
  string str;
  str=readword(file);
  return stod(str);
}

double geolattice::elev(int lat,int lon)
{
  int easting,northing,eint,nint;
  double epart,npart,ne,nw,se,sw,ret;
  xy neslp,nwslp,seslp,swslp;
  easting=(lon-wbd)&0x7fffffff;
  northing=lat-sbd;
  epart=(double)easting*width/(ebd-wbd);
  npart=(double)northing*height/(nbd-sbd);
  eint=floor(epart);
  nint=floor(npart);
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
  if (eint>=0 && eint<width && nint>=0 && nint<height)
  {
    sw=undula[(width+1)*nint+eint];
    se=undula[(width+1)*nint+eint+1];
    nw=undula[(width+1)*(nint+1)+eint];
    ne=undula[(width+1)*(nint+1)+eint+1];
    swslp=xy(eslope[(width+1)*nint+eint],nslope[(width+1)*nint+eint])/2;
    seslp=xy(eslope[(width+1)*nint+eint+1],nslope[(width+1)*nint+eint+1])/2;
    nwslp=xy(eslope[(width+1)*(nint+1)+eint],nslope[(width+1)*(nint+1)+eint])/2;
    neslp=xy(eslope[(width+1)*(nint+1)+eint+1],nslope[(width+1)*(nint+1)+eint+1])/2;
  }
  else
    sw=se=nw=ne=-2147483648;
  if (sw==-2147483648)
    sw=1e30;
  if (se==-2147483648)
    se=1e30;
  if (nw==-2147483648)
    nw=1e30;
  if (ne==-2147483648)
    ne=1e30;
  //ret=((sw*(1-epart)+se*epart)*(1-npart)+(nw*(1-epart)+ne*epart)*npart)/65536;
  ret=bicubic(sw,swslp,se,seslp,nw,nwslp,ne,neslp,epart,npart)/65536;
  if (ret>8850 || ret<-11000)
    ret=NAN;
  return ret;
}

double geolattice::elev(xyz dir)
{
  return elev(dir.lati(),dir.loni());
}

void geolattice::dump()
{
  int i,j;
  cout<<"undula:"<<endl;
  for (i=0;i<height+1;i++)
  {
    for (j=0;j<width+1;j++)
      cout<<setw(11)<<undula[i*(width+1)+j];
    cout<<endl;
  }
  cout<<"eslope:"<<endl;
  for (i=0;i<height+1;i++)
  {
    for (j=0;j<width+1;j++)
      cout<<setw(11)<<eslope[i*(width+1)+j];
    cout<<endl;
  }
  cout<<"nslope:"<<endl;
  for (i=0;i<height+1;i++)
  {
    for (j=0;j<width+1;j++)
      cout<<setw(11)<<nslope[i*(width+1)+j];
    cout<<endl;
  }
}

void geolattice::setslopes()
/* Given points a,b,c spaced 1 apart in order:
 * Slope at b is sl(a,b)+sl(b,c)-sl(a,c). This is just sl(a,c)=(c-a)/2.
 * (2b-2a+2c-2b+a-c)/2=(c-a)/2
 * The division by 2 is done in elev.
 * Slope at c (the edge) is sl(b,c)+sl(c,a)-sl(a,b). This is (c-b)+(c-a)/2-(b-a)
 * =(2c-2b+c-a-2b+2a)/2=(3c-4b+a)/2
 */
{
  int i,j;
  for (i=0;i<height+1;i++)
    for (j=1;j<width;j++)
      eslope[i*(width+1)+j]=undula[i*(width+1)+j+1]-undula[i*(width+1)+j-1];
  if (width>1) // TODO: handle the case of full 360° latitude
    for (i=0;i<height+1;i++)
    {
      eslope[i*(width+1)]=4*undula[i*(width+1)+1]-undula[i*(width+1)+2]-3*undula[i*(width+1)];
      eslope[(i+1)*(width+1)-1]=3*undula[(i+1)*(width+1)-1]-4*undula[(i+1)*(width+1)-2]+undula[(i+1)*(width+1)-3];
    }
  for (i=1;i<height;i++)
    for (j=0;j<width+1;j++)
      nslope[i*(width+1)+j]=undula[(i+1)*(width+1)+j]-undula[(i-1)*(width+1)+j];
  if (height>1)
    for (j=0;j<width+1;j++)
    {
      nslope[j]=4*undula[(width+1)+j]-undula[2*(width+1)+j]-3*undula[j];
      nslope[height*(width+1)+j]=3*undula[height*(width+1)+j]-4*undula[(height-1)*(width+1)+j]+undula[(height-2)*(width+1)+j];
    }
  if (height<=16 && width<=16)
    dump();
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
  nlatsane=hdr.nlat>0 && (hdr.nlat-1)*hdr.latspace<180.000001;
  nlongsane=hdr.nlong>0 && (hdr.nlong-1)*hdr.longspace<360.000001;
  typesane=hdr.dtype<256;
  return ssane && wsane && latsane && longsane && nlatsane && nlongsane && typesane;
}

bool sanitycheck(carlsongsfheader &hdr)
{
  bool ssane,wsane,nsane,esane,latsane,nlatsane,nlongsane;
  ssane=hdr.south>-360.0001 && hdr.south<360.0001 && (hdr.south==0 || fabs(hdr.south)>0.000001);
  wsane=hdr.west>-360.0001 && hdr.west<360.0001 && (hdr.west==0 || fabs(hdr.west)>0.000001);
  nsane=hdr.north>-360.0001 && hdr.north<360.0001 && (hdr.north==0 || fabs(hdr.north)>0.000001);
  esane=hdr.east>-360.0001 && hdr.east<360.0001 && (hdr.east==0 || fabs(hdr.east)>0.000001);
  latsane=hdr.south<hdr.north;
  nlatsane=hdr.nlat>0 && hdr.nlat<=2000000;
  nlongsane=hdr.nlong>0 && hdr.nlong<=4000000;
  return ssane && wsane && latsane && nlatsane && nlongsane;
}

void geolattice::setheader(usngsheader &hdr)
{
  sbd=degtobin(hdr.south);
  wbd=degtobin(hdr.west);
  nbd=degtobin(hdr.south+(hdr.nlat-1)*hdr.latspace);
  ebd=degtobin(hdr.west+(hdr.nlong-1)*hdr.longspace);
  width=hdr.nlong-1;
  height=hdr.nlat-1;
  undula.resize((width+1)*(height+1));
  eslope.resize((width+1)*(height+1));
  nslope.resize((width+1)*(height+1));
}

void geolattice::setheader(carlsongsfheader &hdr)
{
  sbd=degtobin(hdr.south);
  wbd=degtobin(hdr.west);
  nbd=degtobin(hdr.north);
  ebd=degtobin(hdr.east);
  if (wbd-ebd>=0)
    ebd+=DEG360;
  width=hdr.nlong;
  height=hdr.nlat;
  undula.resize((width+1)*(height+1));
  eslope.resize((width+1)*(height+1));
  nslope.resize((width+1)*(height+1));
}

void geolattice::settest()
{
  int i,j;
  sbd=wbd=degtobin(-2);
  nbd=ebd=degtobin(2);
  width=height=4;
  undula.resize((width+1)*(height+1));
  eslope.resize((width+1)*(height+1));
  nslope.resize((width+1)*(height+1));
  for (i=0;i<5;i++)
    for (j=0;j<5;j++)
      undula[i+5*j]=61000*(i-2)+4096*sqr(i-2)+37700*(j-2)-2048*sqr(j-2);
  setslopes();
}

void readusngatxt(geolattice &geo,string filename)
/* This geoid file has order-360 harmonics, but is sampled every 0.25°,
 * so it may not interpolate accurately. It would be better to compute
 * the geoid from the coefficients; this requires making sense of a
 * Fortran program.
 * http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/egm96.html
 * http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html
 */
{
}

void readcarlsongsfheader(carlsongsfheader &hdr,istream &file)
{
  double dnlong,dnlat;
  try
  {
    hdr.south=readdouble(file);
    hdr.west=readdouble(file);
    hdr.north=readdouble(file);
    hdr.east=readdouble(file);
    dnlong=readdouble(file);
    dnlat=readdouble(file);
    /* The numbers of rows and columns must be integers,
     * but are written as 118.0 in gsf files.
     */
    hdr.nlong=dnlong;
    hdr.nlat=dnlat;
  }
  catch (...)
  {
    throw badheader;
  }
  if (hdr.nlong!=dnlong || hdr.nlat!=dnlat)
    throw badheader;
}

int readcarlsongsf(geolattice &geo,string filename)
/* This is a text file used by Carlson software.
 * https://update.carlsonsw.com/kbase_attach/716/Geoid Separation File Format.pdf
 */
{
  int i,j,ret=0;
  fstream file;
  carlsongsfheader hdr;
  file.open(filename,fstream::in|fstream::binary);
  if (file.is_open())
  {
    try
    {
      readcarlsongsfheader(hdr,file);
    }
    catch (int e)
    {
      ret=-e;
    }
    if (ret==0 && sanitycheck(hdr))
    {
      cout<<"Header sane"<<endl;
      cout<<"South "<<hdr.south<<" West "<<hdr.west<<endl;
      cout<<"North "<<hdr.north<<" East "<<hdr.east<<endl;
      cout<<"Rows "<<hdr.nlat<<" Columns "<<hdr.nlong<<endl;
      geo.setheader(hdr);
      for (i=0;i<geo.height+1;i++)
	for (j=0;j<geo.width+1;j++)
	  geo.undula[i*(geo.width+1)+j]=rint(65536*(readdouble(file)));
      if (file.fail())
	ret=1;
      else
      {
	ret=2;
	geo.setslopes();
      }
    }
    else
      ret=1;
    file.close();
  }
  else
    ret=0;
  return ret;
}

int readusngsbin(geolattice &geo,string filename)
{
  int i,j,ret;
  fstream file;
  usngsheader hdr;
  bool bigendian;
  file.open(filename,fstream::in|fstream::binary);
  if (file.is_open())
  {
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
    {
      cout<<"Header sane"<<endl;
      cout<<"South "<<hdr.south<<" West "<<hdr.west<<endl;
      cout<<"Latitude spacing "<<hdr.latspace<<" Longitude spacing "<<hdr.longspace<<endl;
      cout<<"Rows "<<hdr.nlat<<" Columns "<<hdr.nlong<<endl;
      geo.setheader(hdr);
      for (i=0;i<geo.height+1;i++)
	for (j=0;j<geo.width+1;j++)
	  geo.undula[i*(geo.width+1)+j]=rint(65536*(bigendian?readbefloat(file):readlefloat(file)));
      if (file.fail())
	ret=1;
      else
      {
	ret=2;
	geo.setslopes();
      }
    }
    else
      ret=1;
    file.close();
  }
  else
    ret=0;
  return ret;
}

double avgelev(xyz dir)
{
  int i,n;
  double u,sum;
  for (sum=i=n=0;i<geo.size();i++)
  {
    u=geo[i].elev(dir);
    if (std::isfinite(u))
    {
      sum+=u;
      n++;
    }
  }
  return sum/n;
}

matrix autocorr(double qpoints[][16])
/* Autocorrelation of the six undulation components, masked by which of qpoints
 * are finite. When all are finite, the matrix is diagonal-dominant, but when
 * only half are finite, it often isn't.
 */
{
  geoquad unitquad[6];
  int i,j,k,l;
  matrix ret(6,6);
  manysum sum;
  for (i=0;i<6;i++)
    for (j=0;j<6;j++)
      unitquad[i].und[j]=i==j;
  for (i=0;i<6;i++)
    for (j=0;j<=i;j++)
    {
      sum.clear();
      for (k=0;k<16;k++)
	for (l=0;l<16;l++)
	  if (std::isfinite(qpoints[k][l]))
	    sum+=unitquad[i].undulation(-0.9375+0.125*k,-0.9375+0.125*l)
	        *unitquad[j].undulation(-0.9375+0.125*k,-0.9375+0.125*l);
      ret[i][j]=ret[j][i]=sum.total();
    }
  return ret;
}

void dump256(double qpoints[][16])
{
  int i,j;
  for (i=0;i<16;i++)
  {
    for (j=0;j<16;j++)
      if (std::isfinite(qpoints[i][j]))
	cout<<" *";
      else
	cout<<" -";
    cout<<endl;
  }
}

array<double,6> correction(geoquad &quad,double qpoints[][16])
{
  array<double,6> ret;
  matrix preret(6,1);
  int i,j,k,qhash;
  double diff;
  geoquad unitquad;
  qhash=quadhash(qpoints);
  if (quadinv.count(qhash)==0)
    quadinv[qhash]=invert(autocorr(qpoints));
  for (i=0;i<6;i++)
    ret[i]=0;
  for (i=0;i<16;i++)
    for (j=0;j<16;j++)
      if (std::isfinite(qpoints[i][j]))
      {
	diff=qpoints[i][j]-quad.undulation(-0.9375+0.125*i,-0.9375+0.125*j);
	for (k=0;k<6;k++)
	{
	  unitquad.und[k]=1;
	  unitquad.und[(k+5)%6]=0;
	  preret[k][0]+=diff*unitquad.undulation(-0.9375+0.125*i,-0.9375+0.125*j);
	}
      }
  /*ret[0]=preret[0][0]/256;
  ret[1]=preret[1][0]/85;
  ret[2]=preret[2][0]/85;
  ret[3]=preret[3][0]*2304/51409;
  ret[4]=preret[4][0]*256/7225;
  ret[5]=preret[5][0]*2304/51409;*/
  preret=quadinv[qhash]*preret;
  for (i=0;i<6;i++)
    ret[i]=preret[i][0];
  return ret;
}

int quadhash(double qpoints[][16])
/* Used to remember inverses of matrices for patterns of points in a geoquad
 * inside and outside the area being converted. Most of them can be formed by
 * running a straight line through a 16×16 lattice of points and taking all
 * those on one side. There are 20173 such patterns, all of which have different
 * hashes. This fills the hash table only 0.00044, so other patterns will
 * probably not collide with them.
 */
{
  int i,j,ret;
  for (ret=i=0;i<16;i++)
    for (j=0;j<16;j++)
      if (std::isfinite(qpoints[i][j]))
	ret=(2*ret)%HASHPRIME;
      else
	ret=(2*ret+1)%HASHPRIME;
  return ret;
}

double maxerror(geoquad &quad,double qpoints[][16])
{
  double ret=0;
  int i,j;
  double diff;
  geoquad unitquad;
  for (i=0;i<16;i++)
    for (j=0;j<16;j++)
      if (std::isfinite(qpoints[i][j]))
      {
	diff=fabs(qpoints[i][j]-quad.undulation(-0.9375+0.125*i,-0.9375+0.125*j));
	if (diff>ret)
	  ret=diff;
      }
  //cout<<"maxerror "<<ret<<endl;
  return ret;
}
