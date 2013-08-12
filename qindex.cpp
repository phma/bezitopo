/******************************************************/
/*                                                    */
/* qindex.cpp - quad index to tin                     */
/*                                                    */
/******************************************************/

#include <cmath>
#include "qindex.h"
#include "ps.h"

/* The index enables quickly finding a triangle containing a given point.
   x and y are the bottom left corner. side is always a power of 2,
   and x and y are multiples of side/16.
   The four subsquares are arranged as follows:
   +-------+-------+
   |       |       |
   |   2   |   3   |
   |       |       |
   +-------+-------+
   |       |       |
   |   0   |   1   |
   |       |       |
   +-------+-------+
   A square is subdivided if there are at least three points of the TIN
   in it. A point is considered to be in a square if it is on its
   bottom or left edge, but not if it is on its top or right edge.

   After constructing the tree of squares, the program assigns to each
   leaf square the triangle containing its center, proceeding in
   Hilbert-curve order.
   */
using namespace std;

qindex::qindex()
{
  int i;
  x=y=side=0;
  for (i=0;i<4;i++)
    sub[i]=0;
}

qindex::~qindex()
{
  clear();
}

int qindex::size()
{
  if (sub[3])
    return sub[0]->size()+sub[1]->size()+sub[2]->size()+sub[3]->size()+1;
  else
    return 1;
}

xy qindex::middle()
{return xy(x+side/2,y+side/2);
 }

int qindex::quarter(xy pnt)
{
  int xbit,ybit,i;
  xbit=pnt.x>=x+side/2;
  if (pnt.x>=x+side || pnt.x<x)
    xbit=-1;
  ybit=pnt.y>=y+side/2;
  if (pnt.y>=y+side || pnt.y<y)
    ybit=-1;
  i=(ybit<<1)|xbit;
  return i;
}

triangle *qindex::findt(xy pnt)
{
  int i;
  i=quarter(pnt);
  if (i<0)
    return NULL; // point is outside square
  else if (!sub[3])
    return tri; // square is undivided
  else 
    return sub[i]->findt(pnt);
}

void qindex::sizefit(vector<xy> pnts)
/* Computes size, x, and y such that size is a power of 2, x and y are multiples
 * of size/16, and all points are in the resulting square.
 */
{
  double minx=HUGE_VAL,miny=HUGE_VAL,maxx=-HUGE_VAL,maxy=-HUGE_VAL;
  int i;
  for (i=0;i<pnts.size();i++)
  {
    if (pnts[i].east()>maxx)
      maxx=pnts[i].east();
    if (pnts[i].east()<minx)
      minx=pnts[i].east();
    if (pnts[i].north()>maxy)
      maxy=pnts[i].north();
    if (pnts[i].north()<miny)
      miny=pnts[i].north();
  }
  if (maxy<=miny && maxx<=minx)
    side=0;
  else
  {
    side=(maxx+maxy-minx-miny)/2;
    side/=significand(side);
    x=minx-side;
    y=miny-side;
    while (x+side<maxx || y+side<maxy)
    {
      side*=2;
      x=(rint((minx+maxx)/side*8)-8)*side/16;
      y=(rint((miny+maxy)/side*8)-8)*side/16;
    }
  }
}

void qindex::split(vector<xy> pnts)
/* Computes size, x, and y such that size is a power of 2, x and y are multiples
 * of size/16, and all points are in the resulting square.
 */
{
  vector<xy> subpnts[4];
  int i,q;
  if (pnts.size()>=3)
  {
    for (i=0;i<pnts.size();i++)
    {
      q=quarter(pnts[i]);
      if (q>=0)
	subpnts[q].push_back(pnts[i]);
    }
    for (i=0;i<4;i++)
    {
      sub[i]=new qindex;
      sub[i]->x=x+(i&1)*(side/2);
      sub[i]->y=y+(i>>1)*(side/2);
      sub[i]->side=side/2;
      sub[i]->split(subpnts[i]);
    }
  }
}

void qindex::clear()
{
  int i;
  if (sub[3])
    for (i=0;i<4;i++)
    {
      delete(sub[i]);
      sub[i]=NULL;
    }
}

void qindex::draw(bool root)
{
  int i;
  if (root) // if this is the root of the tree, draw its border
  {
    line2p(xy(x,y),xy(x+side,y));
    line2p(xy(x+side,y),xy(x+side,y+side));
    line2p(xy(x+side,y+side),xy(x,y+side));
    line2p(xy(x,y+side),xy(x,y));
  }
  if (sub[3])
  {
    for (i=0;i<4;i++)
      sub[i]->draw(false);
    line2p(xy(x,y+side/2),xy(x+side,y+side/2));
    line2p(xy(x+side/2,y),xy(x+side/2,y+side));
  }
}
