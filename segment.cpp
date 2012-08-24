/******************************************************/
/*                                                    */
/* segment.cpp - 3d line segment                      */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/

#include "segment.h"

segment::segment()
{
  start=end=xyz(0,0,0);
  control1=control2=0;
}

segment::segment(xyz kra,xyz fam)
{
  start=kra;
  end=fam;
  control1=(2*start.elev()+end.elev())/3;
  control2=(start.elev()+2*end.elev())/3;
}

double segment::length()
{
  return dist(xy(start),xy(end));
}
