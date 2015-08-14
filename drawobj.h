/******************************************************/
/*                                                    */
/* drawobj.h - drawing object base class              */
/*                                                    */
/******************************************************/

#ifndef DRAWOBJ_H
#define DRAWOBJ_H
#include "xyz.h"

class bsph
{
public:
  xyz center;
  double radius;
};

class hline
{
public:
  xyz dir,pnt;
};

class drawobj
{
public:
  virtual bsph boundsphere();
  virtual bool hittest(hline hitline);
  virtual void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto)
  {
  }
  virtual void roscat(xy tfrom,int ro,double sca,xy tto);
  /* This will also have a method to draw on a device. It will be passed
   * a callback of some sort and will pass bezier3d objects to the callback.
   */
};

#endif
