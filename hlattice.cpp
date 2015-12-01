/******************************************************/
/*                                                    */
/* hlattice.cpp - hexagonal lattice                   */
/*                                                    */
/******************************************************/
/* Hexagonal vector (Eisenstein or Euler integers) and hexagonal lattice
 * Adapted from Propolis.
 */

#include <cstdio>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cassert>
#include "hlattice.h"
#include "angle.h"

using namespace std;

unsigned long _norm(int x,int y)
{return sqr(x)+sqr(y)-x*y;
 }

unsigned long hvec::norm()
{return sqr(this->x)+sqr(this->y)-this->x*this->y;
 }

hvec hvec::operator+(hvec b)
{
  return hvec(this->x+b.x,this->y+b.y);
}

hvec hvec::operator-()
{
  return hvec(-this->x,-this->y);
}

hvec hvec::operator-(hvec b)
{
  return hvec(this->x-b.x,this->y-b.y);
}

bool hvec::operator==(hvec b)
{
  return this->x==b.x && this->y==b.y;
}

bool hvec::operator!=(hvec b)
{
  return this->x!=b.x || this->y!=b.y;
}

hvec nthhvec(int n,int size,int nelts)
{
  int x,y,row;
  assert (n>=0 && n<nelts);
  n-=nelts/2;
  if (n<0)
  {
    for (n-=size,row=2*size+1,y=0;n<=0;n+=row--,y--)
      ;
    y++;
    n-=++row;
    x=n+y+size;
  }
  else
  {
    for (n+=size,row=2*size+1,y=0;n>=0;n-=row--,y++)
      ;
    y--;
    n+=++row;
    x=n+y-size;
  }
  hvec a(x,y);
  return a;
}

int hvec::pageinx(int size,int nelts)
// Index to a byte within a page of specified size. Used in the inverse
// letter table as well as the paging of harray.
{
  if (y<0)
    return (-y-size)*(-y-3*size-3)/2+x-y;
  else
    return x-y+nelts-(size-y)*(3*size+3-y)/2-1;
}

hlattice::hlattice(int size)
{
  int i,up,dn;
  for (i=0,up=dn=size;i<=size;i++)
  {
    rightedge[up]=hvec(size,i);
    rightedge[dn]=hvec(size-i,-i);
    up+=2*size-i;
    dn-=2*size-i+1;
  }
  nelts=3*size*(size+1)+1;
}

hvec hlattice::nthhvec(int n)
{
  map<int,hvec>::iterator rowend;
  n-=nelts/2;
  rowend=rightedge.lower_bound(n);
  return rowend->second+(n-rowend->first);
}
