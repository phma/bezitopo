/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
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
#include <iostream>
#include <ctime>
#include "config.h"
#include "geoid.h"
#include "sourcegeoid.h"
#include "refinegeoid.h"
#include "document.h"
#include "raster.h"
#include "hlattice.h"
#include "relprime.h"
#include "ps.h"
#include "manysum.h"
using namespace std;

document doc;
geoheader hdr;

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 589824/51409
 * 4: 65536/7225
 * 5: 589824/51409
 */

xy unfold(vball pnt)
{
  xy ret(-2,-2);
  switch (pnt.face)
  {
    case 1:
      ret=xy(pnt.x,pnt.y);
      break;
    case 2:
      ret=xy(2-pnt.y,pnt.x);
      break;
    case 3:
      ret=xy(pnt.y,2-pnt.x);
      break;
    case 4:
      ret=xy(-pnt.y,pnt.x-2);
      break;
    case 5:
      ret=xy(-2-pnt.y,pnt.x);
      break;
    case 6:
      ret=xy(4-pnt.x,-pnt.y);
      break;
  }
  return ret;
}

void plotcenter(geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotcenter(*quad.sub[i]);
  else
    dot(unfold(quad.vcenter()));
}

void plotcenters()
{
  setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotcenter(cube.faces[i]);
}

void plotinter(geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotinter(*quad.sub[i]);
  else
  {
    setcolor(0,0,1);
    for (i=0;i<quad.nums.size();i++)
      dot(unfold(vball(quad.face,quad.nums[i])));
    setcolor(1,0,0);
    for (i=0;i<quad.nans.size();i++)
      dot(unfold(vball(quad.face,quad.nans[i])));
  }
}

void plotinters()
{
  setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotinter(cube.faces[i]);
}

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
  cout<<"c: "<<cube.undulation(lat,lon)<<endl;
}

/* Command line syntax:
 * -f format		Puts format first on the list of formats to try.
 * -i file		Reads file.
 * -o file		Sets the output filename. The file is written after
 * 			all input files are read.
 * -c lat long radius	Excerpts a circle from the geoid file.
 * Outputting the KML file is automatic; there is no option for it.
 * 
 * Example:
 * convertgeoid -f ngs -i g2012bu0.bin -c 38N99W 150 -f gsf -o Macksville.gsf
 * Reads the Lower 48 file in NGS format, outputs an excerpt called Macksville.gsf
 * containing a circle of radius 150 km centered at 38N99W in GSF format,
 * and outputs the boundary to file Macksville.gsf.kml .
 */

int main(int argc, char *argv[])
{
  ofstream ofile;
  int i;
  vball v;
  cout<<"Convertgeoid, part of Bezitopo version "<<VERSION<<" © 2016 Pierre Abbat\n"
  <<"Distributed under GPL v3 or later. This is free software with no warranty."<<endl;
  cube.scale=1/65536.;
  hdr.logScale=-16;
  hdr.planet=BOL_EARTH;
  hdr.dataType=BOL_UNDULATION;
  hdr.encoding=BOL_VARLENGTH;
  hdr.ncomponents=1;
  hdr.tolerance=0.03;
  hdr.sublimit=1000;
  hdr.spacing=1e5;
  hdr.namesFormats.push_back("../g2012bu0.bin");
  hdr.namesFormats.push_back("usngs");
  hdr.namesFormats.push_back("../g2012ba0.bin");
  hdr.namesFormats.push_back("usngs");
  hdr.namesFormats.push_back("../g2012bh0.bin");
  hdr.namesFormats.push_back("usngs");
  hdr.namesFormats.push_back("../g2012bg0.bin");
  hdr.namesFormats.push_back("usngs");
  hdr.namesFormats.push_back("../g2012bs0.bin");
  hdr.namesFormats.push_back("usngs");
  hdr.namesFormats.push_back("../g2012bp0.bin");
  hdr.namesFormats.push_back("usngs");
  geo.resize(hdr.namesFormats.size()/2);
  for (i=0;i<geo.size()-1;i++)
    readusngsbin(geo[i],hdr.namesFormats[i*2]);
  //geo[i].settest();
  drawglobecube(1024,62,-7,1,0,"geoid.ppm");
  drawglobemicro(1024,xy(1.3429,0.2848),3e-4,1,0,"geowrangell.ppm");
  /* (-.4304,-.3142,1) 143.86986°W 61.9475°N 143°52'12"W 61°56'51"N
   * This is a local maximum of the geoid in Alaska at Mount Wrangell.
   * It clearly shows artifacts when interpolated bilinearly.
   */
  drawglobemicro(1024,xy(0.384566,1.27153),3e-3,1,0,"geonieves.ppm");
  /* (-.230868,-1,0.45694) 103°W 24°N
   * This is on the boundary of the Lower 48 file.
   */
  drawglobemicro(1024,xy(1.21676,0.294214),3e-3,1,0,"geogranisle.ppm");
  /* (-.411572,-.56648,1) 126°W 55°N
   * This is on the boundary of the overlap between the Alaska file
   * and the Lower 48 file.
   */
  drawglobemicro(1024,xy(3.579192,1.5984015),1e-3,1,0,"geosamoan.ppm");
  drawglobemicro(1024,xy(3.6062785,1.627449),1e-3,1,0,"geosamoae.ppm");
  drawglobemicro(1024,xy(3.579192,1.654771),1e-3,1,0,"geosamoas.ppm");
  drawglobemicro(1024,xy(3.552552,1.6253505),1e-3,1,0,"geosamoaw.ppm");
  //drawglobemicro(1024,xy(1.5,1.5),2e-2,1,0,"geotest.ppm");
  v=encodedir(Sphere.geoc(degtorad(62),degtorad(-144),0.));
  cout<<"Wrangell "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(24),degtorad(-103),0.));
  cout<<"Nieves "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(55),degtorad(-126),0.));
  cout<<"Granisle "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-11),degtorad(-171),0.));
  cout<<"Samoa N "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-14),degtorad(-168),0.));
  cout<<"Samoa E "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-17),degtorad(-171),0.));
  cout<<"Samoa S "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-14),degtorad(-174),0.));
  cout<<"Samoa W "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  for (i=0;i<6;i++)
  {
    //cout<<"Face "<<i+1;
    //cout.flush();
    interroquad(cube.faces[i],3e5);
    /*if (cube.faces[i].isfull()>=0)
      cout<<" has data"<<endl;
    else
      cout<<" is empty"<<endl;*/
    refine(cube.faces[i],cube.scale,hdr.tolerance,hdr.sublimit,hdr.spacing);
  }
  outProgress();
  cout<<endl;
  outund("Green Hill",degtobin(35.4),degtobin(-82.05));
  outund("Charlotte",degtobin(35.22),degtobin(-80.84));
  outund("Kitimat",degtobin(54.0547),degtobin(-128.6578)); // in the overlap of two files
  outund("Denali",degtobin(63.0695),degtobin(-151.0074));
  outund("Haleakala",degtobin(20.7097),degtobin(-156.2533));
  drawglobemicro(1024,xy(1.3429,0.2848),3e-4,2,0,"geowrangellcvt.ppm");
  drawglobemicro(1024,xy(0.384566,1.27153),3e-3,2,0,"geonievescvt.ppm");
  drawglobemicro(1024,xy(1.21676,0.294214),3e-3,2,0,"geogranislecvt.ppm");
  drawglobemicro(1024,xy(3.579192,1.5984015),1e-3,2,0,"geosamoancvt.ppm");
  drawglobemicro(1024,xy(3.6062785,1.627449),1e-3,2,0,"geosamoaecvt.ppm");
  drawglobemicro(1024,xy(3.579192,1.654771),1e-3,2,0,"geosamoascvt.ppm");
  drawglobemicro(1024,xy(3.552552,1.6253505),1e-3,2,0,"geosamoawcvt.ppm");
  //drawglobemicro(1024,xy(1.5,1.5),2e-2,2,0,"geotestcvt.ppm");
  psopen("geoid.ps");
  psprolog();
  startpage();
  plotcenters();
  endpage();
  //startpage();
  //plotinters();
  //endpage();
  pstrailer();
  psclose();
  //hdr.hash=cube.hash();
  ofile.open("geoid.bol");
  hdr.hash=cube.hash();
  hdr.writeBinary(ofile);
  cube.writeBinary(ofile);
  return 0;
}
