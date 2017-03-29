/******************************************************/
/*                                                    */
/* latlong.cpp - latitude-longitude structure         */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include <vector>
#include "angle.h"
#include "latlong.h"

using namespace std;

latlong::latlong()
{
  lat=lon=0;
}

latlong::latlong(int ilat,int ilon)
{
  lat=bintorad(ilat);
  lon=bintorad(ilon);
}

latlong::latlong(double dlat,double dlon)
{
  lat=dlat;
  lon=dlon;
}

latlong::latlong(latlongelev lle)
{
  lat=lle.lat;
  lon=lle.lon;
}

int latlong::valid()
{
  int ret=0;
  if (std::isfinite(lat))
    ret++;
  if (std::isfinite(lon))
    ret++;
  return ret;
}

latlong parselatlong(string angstr,int unitp)
/* Parses a latitude-longitude string. The following are equivalent:
 * 35°N80°W
 * 35°N 80°W
 * 80°W 35°N
 * 80°WN35°
 * N35° 80°W
 * 35° 80°NW
 * But N35°80°W is invalid. The shortest string that is considered valid
 * even though it isn't, because inesw overflows, is 0ENNNENWNWNEESWEE,
 * which is parsed as 0N.
 */
{
  vector<string> nesw,angles;
  string uchar;
  latlong ret(NAN,NAN);
  int inesw=0,i,j,ulen,ctype;
  nesw.push_back("N"); // This may run into trouble in Arabic, where شمال
  nesw.push_back("E"); // and شرق begin with the same letter.
  nesw.push_back("S");
  nesw.push_back("W");
  angles.push_back("");
  for (i=0;i<angstr.length();i++)
  {
    switch (angstr[i]&0xc0)
    {
      case 0:
      case 0x40:
	ulen=1;
	break;
      case 0x80:
	ulen=0;
	break;
      case 0xc0: // first byte of UTF-8
	ulen=angstr[i]&0xff;
	ulen=(ulen>=0x80)+(ulen>=0xc0)+(ulen>=0xe0)+(ulen>=0xf0)+(ulen>=0xf8)+(ulen>=0xfc);
	break;
    }
    uchar=angstr.substr(i,ulen);
    ctype=0;
    if (ulen==1 && isspace(uchar[0]))
      ctype=1;
    for (j=0;j<nesw.size();j++)
      if (uchar==nesw[j])
      {
	inesw=inesw*5+j%4+1;
	ctype=2;
      }
    if (ctype)
      if (angles.back().length())
	angles.push_back("");
      else;
    else
      angles.back()+=uchar;
  }
  if (angles.back().length()==0)
    angles.resize(angles.size()-1);
  switch(inesw)
  {
    case 1: // N
      if (angles.size()==1)
	ret.lat=parseangle(angles[0],unitp);
      break;
    case 2: // E
      if (angles.size()==1)
	ret.lon=parseangle(angles[0],unitp);
      break;
    case 3: // S
      if (angles.size()==1)
	ret.lat=-parseangle(angles[0],unitp);
      break;
    case 4: // W
      if (angles.size()==1)
	ret.lon=-parseangle(angles[0],unitp);
      break;
    case 7: // NE
      if (angles.size()==2)
      {
	ret.lat=parseangle(angles[0],unitp);
	ret.lon=parseangle(angles[1],unitp);
      }
      break;
    case 9: // NW
      if (angles.size()==2)
      {
	ret.lat=parseangle(angles[0],unitp);
	ret.lon=-parseangle(angles[1],unitp);
      }
      break;
    case 11: // EN
      if (angles.size()==2)
      {
	ret.lat=parseangle(angles[1],unitp);
	ret.lon=parseangle(angles[0],unitp);
      }
      break;
    case 13: // ES
      if (angles.size()==2)
      {
	ret.lat=-parseangle(angles[1],unitp);
	ret.lon=parseangle(angles[0],unitp);
      }
      break;
    case 17: // SE
      if (angles.size()==2)
      {
	ret.lat=-parseangle(angles[0],unitp);
	ret.lon=parseangle(angles[1],unitp);
      }
      break;
    case 19: // SW
      if (angles.size()==2)
      {
	ret.lat=-parseangle(angles[0],unitp);
	ret.lon=-parseangle(angles[1],unitp);
      }
      break;
    case 21: // WN
      if (angles.size()==2)
      {
	ret.lat=parseangle(angles[1],unitp);
	ret.lon=-parseangle(angles[0],unitp);
      }
      break;
    case 23: // WS
      if (angles.size()==2)
      {
	ret.lat=-parseangle(angles[1],unitp);
	ret.lon=-parseangle(angles[0],unitp);
      }
      break;
  }
  return ret;
}

string formatlatlong(latlong ll,int unitp)
{
  vector<string> nesw;
  nesw.push_back("N"); // This may run into trouble in Arabic, where شمال
  nesw.push_back("E"); // and شرق begin with the same letter.
  nesw.push_back("S");
  nesw.push_back("W");
  string latstr,lonstr;
  latstr=radtoangle(fabs(ll.lat),unitp)+nesw[signbit(ll.lat)?2:0];
  lonstr=radtoangle(fabs(ll.lon),unitp)+nesw[signbit(ll.lon)?3:1];
  return latstr+' '+lonstr;
}
