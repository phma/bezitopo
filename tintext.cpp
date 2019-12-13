/******************************************************/
/*                                                    */
/* tintext.cpp - input TIN in AquaVeo text format     */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
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
// https://www.xmswiki.com/wiki/TIN_Files

#include <fstream>
#include "tintext.h"
#include "binio.h"
#include "textfile.h"
#include "ldecimal.h"
#include "firstarg.h"
using namespace std;

bool readTinText(string inputFile,pointlist &pl,double unit)
{
  ifstream file(inputFile);
  TextFile tfile(file);
  int i,n,tag,ptnum,corners[3];
  int offset; // in case one TIN file contains multiple TINs
  int numPoints,numTriangles;
  int a,b,c;
  vector<string> words;
  double x,y,z;
  bool good=false,cont=true;
  triangle *tri;
  string line,cardType;
  pl.clear();
  while (cont)
  {
    line=tfile.getline();
    cardType=firstarg(line);
    if (cardType=="TIN")
      cout<<"TIN card\n";
    else if (cardType=="BEGT")
      cout<<"BEGT card\n";
    else if (cardType=="TNAM")
      cout<<"TNAM card\n";
    else if (cardType=="TCOL")
      cout<<"TCOL card\n";
    else if (cardType=="MAT")
      cout<<"MAT card\n";
    else if (cardType=="VERT")
    {
      words=splitWords(line);
      if (words.size()==1)
      {
	numPoints=stoi(words[0]);
	for (i=0;i<numPoints;i++)
	{
	  line=tfile.getline();
	  words=splitWords(line);
	  if (words.size()==3 || words.size()==4)
	  {
	    x=stod(words[0])*unit;
	    y=stod(words[1])*unit;
	    z=stod(words[2])*unit;
	  }
	  else
	    good=cont=false;
	}
      }
      cout<<"VERT card "<<numPoints<<endl;;
    }
    else if (cardType=="TRI")
    {
      words=splitWords(line);
      if (words.size()==1)
      {
	numTriangles=stoi(words[0]);
	for (i=0;i<numTriangles;i++)
	{
	  line=tfile.getline();
	  words=splitWords(line);
	  if (words.size()==3)
	  {
	    a=stoi(words[0]);
	    b=stoi(words[1]);
	    c=stoi(words[2]);
	  }
	  else
	    good=cont=false;
	}
      }
      if (numTriangles>0 && numPoints>0)
	good=true;
      cout<<"TRI card "<<numTriangles<<endl;
    }
    else if (cardType=="ENDT")
      cout<<"ENDT card\n";
    else
      good=cont=false;
  }
}
