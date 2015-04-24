/******************************************************/
/*                                                    */
/* pnezd.cpp - file i/o in                            */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <cstdlib>
#include <fstream>
#include <iostream>
#include "bezitopo.h"
#include "pnezd.h"
#include "measure.h"
#include "pointlist.h"
#include "ldecimal.h"
using namespace std;

/* The file produced by Total Open Station has a first line consisting of column
 * labels, which must be ignored. It is in CSV format; the quotation marks need
 * to be stripped. The file downloaded from the Nikon total station has a last line
 * consisting of ^Z; it must be ignored.
 */

vector<string> parsecsvline(string line)
{
  bool inquote,endquote;
  size_t pos;
  int ch;
  vector<string> ret;
  inquote=endquote=false;
  while (line.length())
  {
    if (ret.size()==0)
      ret.push_back("");
    pos=line.find_first_of("\",");
    if (pos<line.length())
      ch=line[pos];
    else
    {
      ch=-512;
      pos=line.length();
    }
    if (pos>0)
    {
      ret.back()+=line.substr(0,pos);
      line.erase(0,pos);
      endquote=false;
    }
    if (ch==',')
    {
      if (inquote)
	ret.back()+=ch;
      else
	ret.push_back("");
      endquote=false;
    }
    if (ch=='"')
    {
      if (inquote)
      {
	inquote=false;
	endquote=true;
      }
      else
      {
	if (endquote)
	  ret.back()+=ch;
	endquote=false;
	inquote=true;
      }
    }
    if (line.length())
      line.erase(0,1);
  }
  return ret;
}

string makecsvword(string word)
{
  string ret;
  size_t pos;
  if (word.find_first_of("\",")==string::npos)
    ret=word;
  else
  {
    ret="\"";
    while (word.length())
    {
      pos=word.find_first_of('"');
      if (pos==string::npos)
      {
	ret+=word;
	word="";
      }
      else
      {
	ret+=word.substr(0,pos+1)+'"';
	word.erase(0,pos+1);
      }
    }
    ret+='"';
  }
  return ret;
}

string makecsvline(vector<string> words)
{
  string ret;
  int i;
  if (words.size()==1 && words[0].length()==0)
    ret="\"\""; // special case: a single empty word must be quoted
  for (i=0;i<words.size();i++)
  {
    if (i)
      ret+=',';
    ret+=makecsvword(words[i]);
  }
  return ret;
}

int readpnezd(string fname,bool overwrite)
{
  ifstream infile;
  size_t size=0,pos1,pos2;
  ssize_t len;
  int p,npoints;
  double n,e,z;
  vector<string> words;
  string line,pstr,nstr,estr,zstr,d;
  infile.open(fname);
  npoints=-(!infile.is_open());
  if (infile.is_open())
  {
    do
    {
      getline(infile,line);
      words=parsecsvline(line);
      if (words.size()==5)
      {
	pstr=words[0];
	nstr=words[1];
	estr=words[2];
	zstr=words[3];
	d=words[4];
	if (zstr!="z")
	{
	  p=atoi(pstr.c_str());
	  n=parse_length(nstr.c_str());
	  e=parse_length(estr.c_str());
	  z=parse_length(zstr.c_str());
	  pointlists[0].addpoint(p,point(e,n,z,d),overwrite);
	  npoints++;
	}
	//puts(d.c_str());
      }
      else if (words.size()==0 || (words.size()==1 && words[0].length() && words[0][0]<32))
	; // blank line or end-of-file character
      else
	cerr<<"Ignored line: "<<line<<endl;
    } while (infile.good());
    infile.close();
  }
  return npoints;
}

int writepnezd(string fname)
{
  ofstream outfile;
  size_t size=0,pos1,pos2;
  ssize_t len;
  int p,npoints;
  double n,e,z;
  ptlist::iterator i;
  vector<string> words;
  string line,pstr,nstr,estr,zstr,d;
  outfile.open(fname);
  npoints=-(!outfile.is_open());
  if (outfile.is_open())
  {
    for (i=pointlists[0].points.begin();i!=pointlists[0].points.end();i++)
    {
      p=i->first;
      n=i->second.north();
      e=i->second.east();
      z=i->second.elev();
      d=i->second.note;
      pstr=to_string(p);
      nstr=ldecimal(from_coherent_length(n));
      estr=ldecimal(from_coherent_length(e));
      zstr=ldecimal(from_coherent_length(z));
      words.clear();
      words.push_back(pstr);
      words.push_back(nstr);
      words.push_back(estr);
      words.push_back(zstr);
      words.push_back(d);
      line=makecsvline(words);
      outfile<<line<<endl;
      if (outfile.good())
	npoints++;
    }
    outfile.close();
  }
  return npoints;
}
