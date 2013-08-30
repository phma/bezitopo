/******************************************************/
/*                                                    */
/* bezitopo.cpp - main program                        */
/*                                                    */
/******************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "point.h"
#include "cogo.h"
#include "bezitopo.h"
#include "test.h"
#include "tin.h"
#include "measure.h"
#include "pnezd.h"
#include "angle.h"
#include "pointlist.h"
#include "vcurve.h"
#include "raster.h"
#include "ps.h"

using namespace std;

int main(int argc, char *argv[])
{
  int i,j,itype;
  double w,e,s,n;
  criteria crit;
  criterion crit1;
  set_length_unit(SURVEYFOOT);
  if (readpnezd("topo0.asc")<0)
    readpnezd("../topo0.asc");
  crit1.str="";
  crit1.istopo=true;
  crit.push_back(crit1);
  crit1.str="FH";
  crit1.istopo=false; // The point labeled FH has a nonsensical elevation and must be removed.
  crit.push_back(crit1);
  copytopopoints(crit);
  //rotate(2);
  topopoints.maketin("bezitopo.ps");
  topopoints.makegrad(0.15);
  topopoints.maketriangles();
  topopoints.setgradient(false);
  topopoints.makeqindex();
  w=topopoints.dirbound(degtobin(0));
  s=topopoints.dirbound(degtobin(90));
  e=-topopoints.dirbound(degtobin(180));
  n=-topopoints.dirbound(degtobin(270));
  rasterdraw(topopoints,xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark.ppm");
  topopoints.setgradient(true);
  rasterdraw(topopoints,xy((e+w)/2,(n+s)/2),e-w,n-s,10,0,10,"IndependencePark-flat.ppm");
  return EXIT_SUCCESS;
}
