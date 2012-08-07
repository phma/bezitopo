/******************************************************/
/*                                                    */
/* pnezd.cpp - file i/o in                            */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <cstdlib>
#include "bezitopo.h"
#include "pnezd.h"
#include "measure.h"
using namespace std;

int readpnezd(string fname,bool overwrite)
{FILE *infile;
 char *lineptr=NULL;
 size_t size=0,pos1,pos2;
 ssize_t len;
 int p,npoints;
 double n,e,z;
 string line,pstr,nstr,estr,zstr,d;
 infile=fopen(fname.c_str(),"r");
 npoints=-(infile==NULL);
 if (infile)
    {do {len=getline(&lineptr,&size,infile);
         if (len>=0)
            {line=lineptr;
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
             addpoint(p,point(e,n,z,d),overwrite);
 	     npoints++;
             //puts(d.c_str());
             }
         } while (len>=0);
     fclose(infile);
    }
 return npoints;
 }
