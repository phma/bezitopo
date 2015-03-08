/******************************************************/
/*                                                    */
/* pnezd.cpp - file i/o in                            */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <cstdlib>
#include <fstream>
#include "bezitopo.h"
#include "pnezd.h"
#include "measure.h"
#include "pointlist.h"
using namespace std;

/* The file downloaded from the Nikon total station has a first line consisting of
 * column labels, which must be ignored. It is in CSV format; the quotation marks
 * need to be stripped. The file produced by Total Open Station has a last line
 * consisting of ^Z; it must be ignored.
 */
int readpnezd(string fname,bool overwrite)
{
  ifstream infile;
  size_t size=0,pos1,pos2;
  ssize_t len;
  int p,npoints;
  double n,e,z;
  string line,pstr,nstr,estr,zstr,d;
  infile.open(fname);
  npoints=-(!infile.is_open());
  if (infile.is_open())
  {
    do
    {
      getline(infile,line);
      len=line.length();
      if (len>0)
      {
	pos1=line.find_first_of(',');
	pstr=line.substr(0,pos1);
	if (pos1!=string::npos)
	  pos1++;
	pos2=pos1;
	pos1=line.find_first_of(',',pos1);
	nstr=line.substr(pos2,pos1-pos2);
	if (pos1!=string::npos)
	  pos1++;
	pos2=pos1;
	pos1=line.find_first_of(',',pos1);
	estr=line.substr(pos2,pos1-pos2);
	if (pos1!=string::npos)
	  pos1++;
	pos2=pos1;
	pos1=line.find_first_of(',',pos1);
	zstr=line.substr(pos2,pos1-pos2);
	if (pos1!=string::npos)
	  pos1++;
	d=line.substr(pos1);
	d=d.substr(0,d.find_first_of("\r\n"));
	p=atoi(pstr.c_str());
	n=parse_length(nstr.c_str());
	e=parse_length(estr.c_str());
	z=parse_length(zstr.c_str());
	surveypoints.addpoint(p,point(e,n,z,d),overwrite);
	npoints++;
	//puts(d.c_str());
      }
    } while (len>0);
    infile.close();
  }
  return npoints;
}
