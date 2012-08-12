/******************************************************/
/*                                                    */
/* pointlist.cpp - list of points                     */
/*                                                    */
/******************************************************/

#include "point.h"
#include "pointlist.h"

using namespace std;

pointlist surveypoints,topopoints;

void pointlist::clear()
{
  points.clear();
  revpoints.clear();
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

void copytopopoints(criteria crit)
{
}
