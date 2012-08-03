/********************************************************/
/*                                                      */
/* bezier.cpp - computes functions of a Bezier triangle */
/*                                                      */
/********************************************************/
#include "bezier.h"

double triangle::area()
{return area3(*a,*b,*c);
 }

double triangle::elevation(xy pnt)
/* Computes the elevation of triangle tri at the point x,y. */
{double p,q,r, // Fraction of distance from a side to opposite corner. p+q+r=1.
        s;   // Area of triangle.
 s=area();
 p=area3(pnt,*b,*c)/s;
 q=area3(*a,pnt,*c)/s;
 r=area3(*a,*b,pnt)/s;
 return q*q*q*b->z+3*q*q*r*ctrl[5]+3*p*q*q*ctrl[2]+
        3*q*r*r*ctrl[6]+6*p*q*r*ctrl[3]+3*p*p*q*ctrl[0]+
        p*p*p*a->z+3*p*p*r*ctrl[1]+3*p*r*r*ctrl[4]+r*r*r*c->z;
 }

bool triangle::in(xy pnt)
{return area3(pnt,*b,*c)>=0 && area3(*a,pnt,*c)>=0 && area3(*a,*b,pnt)>=0;
 }

xy triangle::centroid()
{return (xy(*a)+xy(*b)+xy(*c))/3; //FIXME: check if this affects numerical stability
 }

triangle *triangle::nexttoward(xy pnt)
// If the point is in the triangle, return the same triangle.
// Else return which triangle to look in next.
// If returns NULL, the point is outside the convex hull.
{double p,q,r;
 p=area3(pnt,*b,*c);
 q=area3(*a,pnt,*c);
 r=area3(*a,*b,pnt);
 if (p>=0 && q>=0 && r>=0)
    return this;
 else if (p<q && p<r)
    return aneigh;
 else if (q<r)
    return bneigh;
 else
    return cneigh;
 }
