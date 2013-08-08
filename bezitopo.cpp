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
#include "ps.h"

using namespace std;

int main(int argc, char *argv[])
{
  int i,j,itype;
  criteria crit;
  criterion crit1;
  set_length_unit(SURVEYFOOT);
  if (readpnezd("topo0.asc")<0)
    readpnezd("../topo0.asc");
  crit1.str="";
  crit1.istopo=true;
  crit.push_back(crit1);
  copytopopoints(crit);
  rotate(2);
  topopoints.maketin("bezitopo.ps");
  return EXIT_SUCCESS;
}
