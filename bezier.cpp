/********************************************************/
/*                                                      */
/* bezier.cpp - computes functions of a Bézier triangle */
/* For Bézier functions of one variable, see vcurve.cpp.*/
/*                                                      */
/********************************************************/
#include <cstring>
#include <climits>
#include <iostream>
#include "bezier.h"
#include "angle.h"
using namespace std;

triangle::triangle()
{
  a=b=c=NULL;
  aneigh=bneigh=cneigh=NULL;
  memset(ctrl,0,sizeof(ctrl));
  nocubedir=INT_MAX;
}

double triangle::area()
{return area3(*a,*b,*c);
 }

double triangle::perimeter()
{
  return dist(xy(*a),xy(*b))+dist(xy(*c),xy(*a))+dist(xy(*b),xy(*c));
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

xyz triangle::gradient3(xy pnt)
{
  double p,q,r,s,gp,gq,gr;
  s=area();
  p=area3(pnt,*b,*c)/s;
  q=area3(*a,pnt,*c)/s;
  r=area3(*a,*b,pnt)/s;
  gp=3*q*q*ctrl[2]+6*q*r*ctrl[3]+6*p*q*ctrl[0]+3*p*p*a->z+6*p*r*ctrl[1]+3*r*r*ctrl[4];
  gq=3*q*q*b->z+6*q*r*ctrl[5]+6*p*q*ctrl[2]+3*r*r*ctrl[6]+6*p*r*ctrl[3]+3*p*p*ctrl[0];
  gr=3*q*q*ctrl[5]+6*q*r*ctrl[6]+6*p*q*ctrl[3]+3*p*p*ctrl[1]+6*p*r*ctrl[4]+3*r*r*c->z;
  return xyz(gp,gq,gr);
}

bool triangle::in(xy pnt)
{return area3(pnt,*b,*c)>=0 && area3(*a,pnt,*c)>=0 && area3(*a,*b,pnt)>=0;
 }

xy triangle::centroid()
{return (xy(*a)+xy(*b)+xy(*c))/3; //FIXME: check if this affects numerical stability
 }

void triangle::setcentercp()
{
  ctrl[3]=((ctrl[0]+ctrl[1]+ctrl[2]+ctrl[4]+ctrl[5]+ctrl[6])*3-(a->z+b->z+c->z)*2)/12;
}

bool triangle::iscorner(point *v)
{
  return (a==v)||(b==v)||(c==v);
}

void triangle::flatten()
{
  ctrl[0]=(2*a->z+b->z)/3;
  ctrl[1]=(2*a->z+c->z)/3;
  ctrl[2]=(2*b->z+a->z)/3;
  ctrl[3]=(a->z+b->z+c->z)/3;
  ctrl[4]=(2*c->z+a->z)/3;
  ctrl[5]=(2*b->z+c->z)/3;
  ctrl[6]=(2*c->z+b->z)/3;
}

void triangle::setgradient(xy pnt,xy grad)
// Sets the gradient at corner pnt to grad. If pnt is not a corner,
// either sets the nearest corner or does nothing.
{
  int which;
  double crit;
  crit=1/2.0*(1/dist(xy(*a),xy(*b))+1/dist(xy(*c),xy(*a))+1/dist(xy(*b),xy(*c)));
  grad/=3; // control points are 1/3 of the way along sides
  if (dist(pnt,*a)<crit)
  {
    ctrl[0]=a->z+dot(grad,xy(*b)-xy(*a));
    ctrl[1]=a->z+dot(grad,xy(*c)-xy(*a));
  }
  if (dist(pnt,*b)<crit)
  {
    ctrl[5]=b->z+dot(grad,xy(*c)-xy(*b));
    ctrl[2]=b->z+dot(grad,xy(*a)-xy(*b));
  }
  if (dist(pnt,*c)<crit)
  {
    ctrl[4]=c->z+dot(grad,xy(*a)-xy(*c));
    ctrl[6]=c->z+dot(grad,xy(*b)-xy(*c));
  }
  nocubedir=INT_MAX;
}

void triangle::setneighbor(triangle *neigh)
{
  bool sha,shb,shc;
  sha=neigh->iscorner(a);
  shb=neigh->iscorner(b);
  shc=neigh->iscorner(c);
  if (sha&&shb)
    cneigh=neigh;
  if (shb&&shc)
    aneigh=neigh;
  if (shc&&sha)
    bneigh=neigh;
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

triangle *triangle::findt(xy pnt,bool clip)
{
  triangle *here,*there;
  here=there=this;
  while (here && !here->in(pnt))
  {
    here=here->nexttoward(pnt);
    if (here)
      there=here;
  }
  return clip?there:here;
}

xy triangle::spcoord(double x,double y)
/* Given semiperimeter coordinates rotated to nocubedir,
 * returns coordinates in the global coordinate system.
 */
{
  double s;
  xy along,across,cen;
  s=peri/2;
  cen=centroid();
  along=cossin(nocubedir)*s;
  across=cossin(nocubedir+536870912)*s;
  return cen+across*y+along*x;
}

vector<double> triangle::xsect(int angle,double offset)
/* Where s is the semiperimeter, samples the surface at four points,
 * -3s/2, -s/2, s/2, 3s/2, relative to the centroid, offset by offset.
 * offset is multplied by the semiperimeter and ranges from -1.5 to 1.5.
 */
{
  double s;
  int i;
  vector<double> ret;
  xy along,across,cen;
  s=peri/2;
  cen=centroid();
  along=cossin(angle)*s;
  across=cossin(angle+536870912)*s;
  for (i=-3;i<5;i+=2)
    ret.push_back(elevation(cen+across*offset+along*i*0.5));
  return ret;
}

double triangle::spelevation(int angle,double x,double y)
/* Where s is the semiperimeter, samples the surface at a point
 * x along the line in the direction angle offset by y.
 * x and y are multplied by the semiperimeter.
 */
{
  double s;
  xy along,across,cen;
  s=peri/2;
  cen=centroid();
  along=cossin(angle)*s;
  across=cossin(angle+536870912)*s;
  return elevation(cen+across*y+along*x);
}

double deriv0(vector<double> xsect)
{
  return (-xsect[3]+9*xsect[2]+9*xsect[1]-xsect[0])/16;
}

double deriv1(vector<double> xsect)
{
  return (xsect[0]-27*xsect[1]+27*xsect[2]-xsect[3])/24;
}

double deriv2(vector<double> xsect)
{
  return (xsect[3]-xsect[2]-xsect[1]+xsect[0])/2;
}

double deriv3(vector<double> xsect)
{
  return xsect[3]-3*xsect[2]+3*xsect[1]-xsect[0];
}

double paravertex(vector<double> xsect)
// Finds the vertex of the parabola, assuming that it is a parabola (no cubic component).
{
  double d1,d2;
  d1=deriv1(xsect);
  d2=deriv2(xsect);
  return -d1/d2;
}

int triangle::findnocubedir()
/* The range of atan2i is [-0x40000000,0x40000000] ([-180°,180°]).
 * nocubedir is found by adding 0x15555555 (60°) and 0x2aaaaaab (120°)
 * to atan2i (direction of 1st harmonic of 3rd derivative) and searching
 * between them. It is therefore in [-0x2aaaaaab,0x6aaaaaab] and cannot
 * be 0x7fffffff, hence nocubedir is set to 0x7fffffff (MAXINT) to
 * indicate that it has not been computed.
 * 
 * The 3rd derivative as a function of angle has the form a×sin(θ-b)+c×sin(3θ-d).
 * 1. Find b.
 * 2. Compute 3rd deriv at b+60° and b+120°. These add up to 0. Normally they
 * will be opposite in sign, in which case you can look for the zero between them.
 * But if they have the same sign, try b+30 and b+90° If those still have
 * the same sign, try b+90° and b+150. If even those have the same sign,
 * the 3d deriv is identically 0 and you're seeing roundoff error; return b+90°.
 */
{
  int i,b,d,beg,mid,end;
  double d3a45[4],begderiv,midderiv,endderiv,crit;
  //cout<<"findnocubedir ";
  for (i=0;i<4;i++)
  {
    d3a45[i]=deriv3(xsect(i<<28,0)); // every 45°
    //cout<<d3a45[i]<<' ';
  }
  //cout<<hex<<degtobin(330)<<endl;
  b=atan2i(d3a45[2]+M_SQRT1_2*(d3a45[1]+d3a45[3]),d3a45[0]-M_SQRT1_2*(d3a45[3]-d3a45[1]));
  d=atan2i(d3a45[2]-M_SQRT1_2*(d3a45[1]+d3a45[3]),d3a45[0]-M_SQRT1_2*(d3a45[1]-d3a45[3]));
  beg=b+0x15555555;
  end=b+0x2aaaaaab;
  begderiv=deriv3(xsect(beg,0));
  endderiv=deriv3(xsect(end,0));
  if (begderiv*endderiv>0)
  {
    beg=b+0xaaaaaab;
    end=b+0x20000000;
    begderiv=deriv3(xsect(beg,0));
    endderiv=deriv3(xsect(end,0));
  }
  if (begderiv*endderiv>0)
  {
    beg=b+0x20000000;
    end=b+0x35555555;
    begderiv=deriv3(xsect(beg,0));
    endderiv=deriv3(xsect(end,0));
  }
  if (begderiv*endderiv>0)
  {
    beg=end=b+0x10000000;
    begderiv=endderiv=deriv3(xsect(end,0));
  }
  while (end-beg>1)
  {
    if (abs(endderiv)>=10*abs(begderiv) || abs(begderiv)>=10*abs(endderiv) || end-beg<10)
      mid=beg+(end-beg)/2;
    else
      mid=lrint((beg*endderiv-end*begderiv)/(endderiv-begderiv));
    midderiv=deriv3(xsect(mid,0));
    //cout<<beg<<' '<<begderiv<<' '<<mid<<' '<<midderiv<<' '<<end<<' '<<endderiv<<endl;
    crit=midderiv/(endderiv-begderiv);
    if (isnan(crit))
      crit=0;
    if (crit>=0)
    {
      end=mid;
      endderiv=midderiv;
    }
    if (crit<=0)
    {
      beg=mid;
      begderiv=midderiv;
    }
  }
  if (abs(begderiv)>abs(endderiv))
    nocubedir=end;
  else
    nocubedir=beg;
  return nocubedir;
}

double triangle::flatoffset()
/* The offset at which the cross-section in the nocubedir direction has zero quadratic component.
 * If it's in the range [-1.5,1.5], it is valid. If outside this range, it may be made up.
 */
{
  double minusquad,plusquad,offset;
  if (nocubedir==INT_MAX)
    findnocubedir();
  minusquad=deriv2(xsect(nocubedir,-1.5));
  plusquad=deriv2(xsect(nocubedir,1.5));
  offset=1.5*(minusquad+plusquad)/(minusquad-plusquad);
  if (!isfinite(offset))
    offset=-1048576;
  return offset;
}

double triangle::vtxeloff(double off)
{
  double vtx;
  vtx=paravertex(xsect(nocubedir,off));
  return spelevation(nocubedir,vtx,off);
}

vector<xyz> triangle::slices(bool side)
/* The xyz's are in semiperimeter coordinates rotated to nocubedir.
 * side=true for the positive offset side.
 */
{
  int i;
  vector<xyz> tranches;
  double off,vtx,z,flat;
  bool stop=false;
  flat=flatoffset();
  for (i=0,off=side?1.7:-1.7;!stop;i++,off=(off*16+flat)/17)
  {
    vtx=paravertex(xsect(nocubedir,off));
    z=spelevation(nocubedir,vtx,off);
    tranches.push_back(xyz(vtx,off,z));
    if (i)
    {
      if (abs(off-flat)<3e-6)
	stop=true;
      if (abs(tranches[i].x)>abs(tranches[i-1].x) && abs(tranches[i-1].x)>1.5 && (tranches[i].x>0)==(tranches[i].x>0))
	stop=true;
    }
  }
  return tranches;
}

/* Four-point successive parabolic interpolation is done using the paravertex function as follows:
 * x a     b     c     d   -> e     f     a     b
 *   a  x  b     c     d   ->             a e f b
 *   a     b  x  c     d   ->                   b e f c
 *   a     b     c  x  d   ->                         c e f d
 *   a     b     c     d x ->                         c     d     e     f
 */

xy triangle::critical_point(double start,double startz,double end,double endz)
// start and end are offsets perpendicular to nocubedir
{
  int lw=0,lastlw=0;
  double flw,vtx,p;
  vector<double> y(4),z(4); // x and y are semiperimeter coordinates
  y[3]=1;
  y[1]=start;
  y[2]=end;
  z[1]=startz;
  z[2]=endz;
  while (fabs(y[3]-y[0])*peri>1e-6 && lw*lastlw>-4 && lw<100 && fabs(deriv2(z))>1e-9)
  {
    switch (lw)
    {
      case -2:
	y[3]=y[1];
	y[2]=y[0];
	z[3]=z[1];
	z[2]=z[0];
	break;
      case -1:
	y[3]=y[1];
	z[3]=z[1];
	break;
      case 0:
	y[3]=y[2];
	y[0]=y[1];
	z[3]=z[2];
	z[0]=z[1];
	break;
      case 1:
	y[0]=y[2];
	z[0]=z[2];
	break;
      case 2:
	y[0]=y[2];
	y[1]=y[3];
	z[0]=z[2];
	z[1]=z[3];
	break;
    }
    switch (lw)
    {
      case -2:
	y[1]=2*y[2]-y[3];
	y[0]=2*y[1]-y[2];
	z[1]=vtxeloff(y[1]);
	z[0]=vtxeloff(y[0]);
	break;
      case -1:
      case 0:
      case 1:
	y[1]=(2*y[0]+y[3])/3;
	y[2]=(2*y[3]+y[0])/3;
	z[1]=vtxeloff(y[1]);
	z[2]=vtxeloff(y[2]);
	break;
      case 2:
	y[2]=2*y[1]-y[0];
	y[3]=2*y[2]-y[1];
	z[3]=vtxeloff(y[3]);
	z[2]=vtxeloff(y[2]);
	break;
    }
    lastlw=lw; // detect 2 followed by -2, which is an infinite loop that can happen with flat triangles
    flw=rint(p=paravertex(z));
    if (isfinite(flw))
    {
      lw=flw;
      if (lw>2)
	lw=2;
      if (lw<-2)
	lw=-2;
    }
    else
      lw=256;
  }
  vtx=paravertex(xsect(nocubedir,(y[0]*(1.5-p)+y[3]*(1.5+p))/3));
  return spcoord(vtx,(y[0]*(1.5-p)+y[3]*(1.5+p))/3);
}

vector<xy> triangle::criticalpts_side(bool side)
{
  vector<xyz> tranches;
  vector<xy> critpts;
  int i;
  tranches=slices(side);
  for (i=1;i<tranches.size()-1;i++)
    if ((tranches[i-1].elev()<tranches[i].elev())^(tranches[i].elev()<tranches[i+1].elev()))
      critpts.push_back(critical_point(tranches[i-1].north(),tranches[i-1].elev(),tranches[i+1].north(),tranches[i+1].elev()));
  return critpts;
}

vector<xy> triangle::criticalpts_axis()
/* Finds critical points on the flat axis.
 * There are at most two, which are necessarily saddle points.
 * If there is one, it's a monkey saddle point or chair point, and may be hard to recognize.
 */
{
  vector<xy> critpts;
  vector<double> pside,mside,diff,along;
  double flat,pvertex,mvertex,dvertex,a,b,c,disc,velev,around[8];
  int i,signflips;
  flat=flatoffset();
  pside=xsect(nocubedir,flat+3e-06);
  mside=xsect(nocubedir,flat-3e-06);
  for (i=0;i<4;i++)
    diff.push_back(pside[i]-mside[i]);
  pvertex=paravertex(pside);
  mvertex=paravertex(mside);
  if (fabs(pvertex)<1.5 && fabs(mvertex)<1.5)
  {
    c=deriv0(diff);
    b=deriv1(diff);
    a=deriv2(diff)/2;
    disc=b*b-4*a*c;
    if (disc>0 && fabs(sqrt(disc)/a)<1e-5) // sometimes in monkey, the two points are just over 3e-6 apart
      disc=0; // but it's really only one saddle point of multiplicity 2
    if (disc==0)
      along.push_back(-b/(2*a));
    if (disc>0)
    {
      along.push_back((-b+sqrt(disc))/(2*a));
      along.push_back((-b-sqrt(disc))/(2*a));
    }
    /* If no points found, check for a saddle point (which must be a monkey
    * saddle point) at the vertex.
    */
    if (!along.size())
    {
      dvertex=paravertex(diff);
      velev=spelevation(nocubedir,dvertex,flat);
      around[0]=spelevation(nocubedir,dvertex+0.000070,flat+0.000169)-velev;
      around[1]=spelevation(nocubedir,dvertex+0.000169,flat+0.000070)-velev;
      around[2]=spelevation(nocubedir,dvertex+0.000169,flat-0.000070)-velev;
      around[3]=spelevation(nocubedir,dvertex+0.000070,flat-0.000169)-velev;
      around[4]=spelevation(nocubedir,dvertex-0.000070,flat-0.000169)-velev;
      around[5]=spelevation(nocubedir,dvertex-0.000169,flat-0.000070)-velev;
      around[6]=spelevation(nocubedir,dvertex-0.000160,flat+0.000070)-velev;
      around[7]=spelevation(nocubedir,dvertex-0.000070,flat+0.000169)-velev;
      for (i=signflips=0;i<8;i++)
	signflips+=around[i]*around[(i+1)&7]<0;
      if (signflips>=3)
	along.push_back(dvertex);
    }
  }
  for (i=0;i<along.size();i++)
    critpts.push_back(spcoord(along[i],flat));
  return critpts;
}

vector<xy> triangle::criticalpts()
{
  vector<xy> critpts,ret;
  int i;
  critpts=criticalpts_side(false);
  for (i=0;i<critpts.size();i++)
    if (in(critpts[i]))
      ret.push_back(critpts[i]);
  critpts=criticalpts_side(true);
  for (i=0;i<critpts.size();i++)
    if (in(critpts[i]))
      ret.push_back(critpts[i]);
  critpts=criticalpts_axis();
  for (i=0;i<critpts.size();i++)
    if (in(critpts[i]))
      ret.push_back(critpts[i]);
  return ret;
}
