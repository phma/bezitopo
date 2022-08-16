/******************************************************/
/*                                                    */
/* fileio.cpp - file I/O                              */
/*                                                    */
/******************************************************/
/* Copyright 2019-2022 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <vector>
#include <cmath>
#include <cstring>
#include "dxf.h"
#include "random.h"
#include "boundrect.h"
#include "ldecimal.h"
#include "binio.h"
#include "angle.h"
#include "fileio.h"
using namespace std;

char hexdig[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

const double verticalOffset=0;
/* The vertical offset is added to all points' elevations for debugging
 * the checksums. It is also added to the checksums in a way that depends
 * on the total number of dots. When not debugging, set it to 0.
 */

/* These functions are common to the command-line and GUI programs.
 * In the GUI program, file I/O is done by a thread, one at a time (except
 * that multiple formats of the same TIN may be output by different threads
 * at once), while the program is not working on the TIN.
 */
string noExt(string fileName)
{
  long long slashPos,dotPos; // npos turns into -1, which is convenient
  string between;
  slashPos=fileName.rfind('/');
  dotPos=fileName.rfind('.');
  if (dotPos>slashPos)
  {
    between=fileName.substr(slashPos+1,dotPos-slashPos-1);
    if (between.find_first_not_of('.')>between.length())
      dotPos=-1;
  }
  if (dotPos>slashPos)
    fileName.erase(dotPos);
  return fileName;
}

string extension(string fileName)
{
  long long slashPos,dotPos; // npos turns into -1, which is convenient
  string between;
  slashPos=fileName.rfind('/');
  dotPos=fileName.rfind('.');
  if (dotPos>slashPos)
  {
    between=fileName.substr(slashPos+1,dotPos-slashPos-1);
    if (between.find_first_not_of('.')>between.length())
      dotPos=-1;
  }
  if (dotPos>slashPos)
    fileName.erase(0,dotPos);
  else
    fileName="";
  return fileName;
}

string baseName(string fileName)
{
  long long slashPos;
  slashPos=fileName.rfind('/');
  return fileName.substr(slashPos+1);
}

void deleteFile(string fileName)
{
  remove(fileName.c_str());
}

string randomRenameFile(string fileName)
/* Renames a file, adding a random suffix. The file will be deleted later.
 * In Windows, moving a file on top of an existing file doesn't work.
 * So move the file first, write the new file, then delete the old file.
 */
{
  unsigned int suffixInt=rng.uirandom();
  int i;
  string newName=fileName;
  for (i=0;i<8;i++)
    newName+=hexdig[(suffixInt>>(4*i))&15];
  rename(fileName.c_str(),newName.c_str());
  return newName;
}

void writeDxf(string outputFile,pointlist &pl,bool asc,double outUnit,int flags)
/* Writes TIN and contours in DXF.
 * flags bit 0=write empty triangles; bit 1=write only triangles in boundary;
 * bit 2=don't write any triangles if there are contours.
 */
{
  vector<GroupCode> dxfCodes;
  vector<DxfLayer> dxfLayers;
  map<ContourLayer,int> contourLayers;
  int i,n;
  map<ContourLayer,int>::iterator j;
  map<ContourInterval,vector<polyspiral> >::iterator k;
  DxfLayer layer;
  ContourLayer cl;
  BoundRect br;
  ofstream dxfFile(outputFile,ofstream::binary|ofstream::trunc);
  br.include(&pl);
  contourLayers=pl.contourLayers();
  layer.name="TIN";
  layer.number=1;
  layer.color=1;
  dxfLayers.push_back(layer);
  layer.name="Boundary";
  layer.number=2;
  layer.color=4;
  dxfLayers.push_back(layer);
  for (j=contourLayers.begin();j!=contourLayers.end();++j)
  {
    layer.name="Contour "+j->first.ci.valueToleranceString()+
	       ' '+hexEncodeInt(j->first.tp);
    layer.number=j->second;
    layer.color=(j->first.tp>>8)*2+1;
    dxfLayers.push_back(layer);
  }
  //dxfHeader(dxfCodes,br);
  tableSection(dxfCodes,dxfLayers);
  openEntitySection(dxfCodes);
  for (i=0;i<pl.triangles.size();i++)
    if (pl.triangles[i].ptValid())
      if (pl.shouldWrite(i,flags,contourLayers.size()))
	insertTriangle(dxfCodes,pl.triangles[i],outUnit);
      else;
    else
      cerr<<"Invalid triangle "<<i<<endl;
  cl.ci=pl.contourInterval;
  for (i=0;i<pl.contours.size();i++)
  {
    cl.tp=cl.ci.contourType(pl.contours[i].getElevation());
    n=contourLayers[cl]-1;
    layer=dxfLayers[n];
    insertPolyline(dxfCodes,pl.contours[i],layer,outUnit);
  }
  closeEntitySection(dxfCodes);
  dxfEnd(dxfCodes);
  writeDxfGroups(dxfFile,dxfCodes,asc);
}

void writeStl(string outputFile,pointlist &pl,bool asc,double outUnit,int flags)
/* Unlike the other export functions, setting outUnit to feet does not result
 * in an STL file in feet; the file is always in millimeters. Rather, it means
 * to prefer scales 1:x where x is divisible by 12. Also the flag does not mean
 * to write empty triangles, which must always be written in STL; rather
 * it means to decrease the scale to a round number.
 */
{
  ofstream stlFile(outputFile,asc?ios::trunc:(ios::binary|ios::trunc));
  double bear;
  vector<StlTriangle> stltri;
  //bear=turnFitInPrinter(pl,printer3d);
  //stltri=stlMesh(printer3d,false,false);
  /*if (asc)
    writeStlText(stlFile,stltri);
  else
    writeStlBinary(stlFile,stltri);*/
}
