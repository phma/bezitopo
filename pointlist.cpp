/******************************************************/
/*                                                    */
/* pointlist.cpp - list of points                     */
/*                                                    */
/******************************************************/

#include <cmath>
#include "angle.h"
#include "bezitopo.h"
#include "pointlist.h"

using namespace std;

//vector<pointlist> pointlists;

void pointlist::clear()
{
  points.clear();
  revpoints.clear();
}

void pointlist::clearmarks()
{
  map<int,edge>::iterator e;
  for (e=edges.begin();e!=edges.end();e++)
    e->second.clearmarks();
}

void pointlist::addpoint(int numb,point pnt,bool overwrite)
// If numb<0, it's a point added by bezitopo.
{int a;
 if (points.count(numb))
    if (overwrite)
       points[a=numb]=pnt;
    else
       {if (numb<0)
           {a=points.begin()->first-1;
            if (a>=0)
               a=-1;
            }
        else
           {a=points.rbegin()->first+1;
            if (a<=0)
               a=1;
            }
        points[a]=pnt;
        }
 else
    points[a=numb]=pnt;
 revpoints[&(points[a])]=a;
 }

void pointlist::makeqindex()
{
  vector<xy> plist;
  ptlist::iterator i;
  qinx.clear();
  for (i=points.begin();i!=points.end();i++)
    plist.push_back(i->second);
  qinx.sizefit(plist);
  qinx.split(plist);
  qinx.settri(&triangles[0]);
}

double pointlist::elevation(xy location)
{
  triangle *t;
  t=qinx.findt(location);
  if (t)
    return t->elevation(location);
  else
    return nan("");
}

void pointlist::setgradient(bool flat)
{
  int i;
  for (i=0;i<triangles.size();i++)
    if (flat)
      triangles[i].flatten();
    else
    {
      triangles[i].setgradient(*triangles[i].a,triangles[i].a->gradient);
      triangles[i].setgradient(*triangles[i].b,triangles[i].b->gradient);
      triangles[i].setgradient(*triangles[i].c,triangles[i].c->gradient);
      triangles[i].setcentercp();
    }
}

double pointlist::dirbound(int angle)
/* angle=0x00000000: returns least easting.
 * angle=0x20000000: returns least northing.
 * angle=0x40000000: returns negative of greatest easting.
 */
{
  ptlist::iterator i;
  double bound=HUGE_VAL,turncoord;
  double s=sin(angle),c=cos(angle);
  for (i=points.begin();i!=points.end();i++)
  {
    turncoord=i->second.east()*c+i->second.north()*s;
    if (turncoord<bound)
      bound=turncoord;
  }
  return bound;
}

void pointlist::findcriticalpts()
{
  map<int,edge>::iterator e;
  map<int,triangle>::iterator t;
  for (e=edges.begin();e!=edges.end();e++)
    e->second.findextrema();
  for (t=triangles.begin();t!=triangles.end();t++)
  {
    t->second.findcriticalpts();
    t->second.subdivide();
  }
}

void pointlist::addperimeter()
{
  map<int,triangle>::iterator t;
  for (t=triangles.begin();t!=triangles.end();t++)
    t->second.addperimeter();
}

void pointlist::removeperimeter()
{
  map<int,triangle>::iterator t;
  for (t=triangles.begin();t!=triangles.end();t++)
    t->second.removeperimeter();
}

void pointlist::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<Pointlist>";
  ofile<<"<Contours>";
  for (i=0;i<contours.size();i++)
    contours[i].writeXml(ofile);
  ofile<<"</Contours>";
  ofile<<"</Pointlist>"<<endl;
}

void pointlist::roscat(xy tfrom,int ro,double sca,xy tto)
{
  xy cs=cossin(ro);
  int i;
  ptlist::iterator j;
  for (i=0;i<contours.size();i++)
    contours[i]._roscat(tfrom,ro,sca,cossin(ro)*sca,tto);
  for (j=points.begin();j!=points.end();j++)
    j->second._roscat(tfrom,ro,sca,cossin(ro)*sca,tto);
}

