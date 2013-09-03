/******************************************************/
/*                                                    */
/* tin.cpp - triangulated irregular network           */
/*                                                    */
/******************************************************/

#include <map>
#include <cmath>
#include <iostream>
#include "bezitopo.h"
#include "tin.h"
#include "ps.h"
#include "point.h"
#include "pointlist.h"

#define THR 16777216
//threshold for goodcenter to determine if a point is sufficiently
//on the good side of a side of a triangle

using std::map;
using std::multimap;
using std::vector;

edge* edge::next(point* end)
{if (end==a)
    return nexta;
 else
    return nextb;
 }

triangle* edge::tri(point* end)
{if (end==a)
    return tria;
 else
    return trib;
 }

void edge::setnext(point* end,edge* enext)
{if (end==a)
    nexta=enext;
 else
    nextb=enext;
 }

point* edge::otherend(point* end)
{
  assert(end==a || end==b);
  if (end==a)
    return b;
  else
    return a;
}

xy edge::midpoint()
{
  return (xy(*a)+xy(*b))/2;
}

void edge::dump(pointlist *topopoints)
{printf("addr=%p a=%d b=%d nexta=%p nextb=%p\n",this,topopoints->revpoints[a],topopoints->revpoints[b],nexta,nextb);
 }

void edge::flip(pointlist *topopoints)
/* Given an edge which is a diagonal of a quadrilateral,
   sets it to the other diagonal.
   */
{edge *temp1,*temp2;
 int i,size;
 size=topopoints->points.size();
 for (i=0;i<size && a->line->next(a)!=this;i++)
    a->line=a->line->next(a);
 assert(i<size); //If this assertion fails, the nexta and nextb pointers are messed up.
 for (i=0;i<size && b->line->next(b)!=this;i++)
    b->line=b->line->next(b);
 assert(i<size);
 // The four edges of the quadrilateral are now nexta, a->line, nextb, and b->line.
 if (nexta->a==a)
    nexta->nextb=this;
 else
    nexta->nexta=this;
 if (nextb->b==b)
    nextb->nexta=this;
 else
    nextb->nextb=this;
 if (a->line->a==a)
    a->line->nexta=nexta;
 else
    a->line->nextb=nexta;
 if (b->line->b==b)
    b->line->nextb=nextb;
 else
    b->line->nexta=nextb;
 temp1=b->line;
 temp2=a->line;
 a=nexta->otherend(a);
 b=nextb->otherend(b);
 nexta=temp1;
 nextb=temp2;
 }

bool edge::isinterior()
// Returns true if this edge is in the interior; false if it's in the convex hull -
// or if the pointers are messed up.
{point *tempa,*tempb;
 tempa=nexta->otherend(a);
 tempb=nextb->otherend(b);
 return nexta->next(tempa)->otherend(tempa)==b && nextb->next(tempb)->otherend(tempb)==a && tempa!=tempb;
 }

bool edge::delaunay()
{point *tempa,*tempb;
 if (nexta==NULL || nextb==NULL)
 {
   std::cerr<<"null next edge in delaunay\n";
   return true;
 }
 tempa=nexta->otherend(a);
 tempb=nextb->otherend(b);
 return (!isinterior()) || ::delaunay(*a,*b,*tempa,*tempb);
 }

multimap<double,point*> convexhull;
// The points are ordered by their azimuth from the starting point.
xy startpnt;
multimap<double,point*> outward;
// The points are ordered by their distance from the starting point.

void dumphull()
{multimap<double,point*>::iterator i;
 printf("dump convex hull:\n");
 for (i=convexhull.begin();i!=convexhull.end();i++)
     printf("az=%f pt=%p\n",i->first,i->second);
 //printf("begin=%p end=%p rbegin=%p rend=%p\n",convexhull.begin(),convexhull.end(),convexhull.rbegin(),convexhull.rend());
 printf("end dump\n");
 }

void pointlist::dumpedges()
{
  map<int,edge>::iterator i;
  printf("dump edges:\n");
  for (i=edges.begin();i!=edges.end();i++)
     i->second.dump(this);
  printf("end dump\n");
}

void pointlist::dumpedges_ps(bool colorfibaster)
{
  map<int,edge>::iterator i;
  int n;
  for (i=edges.begin(),n=0;i!=edges.end();i++,n++)
     line(i->second,n,colorfibaster);
}

void pointlist::dumpnext_ps()
{
  map<int,edge>::iterator i;
  setcolor(0,0.7,0);
  for (i=edges.begin();i!=edges.end();i++)
  {
    if (i->second.nexta)
      line2p(i->second.midpoint(),i->second.nexta->midpoint());
    if (i->second.nextb)
      line2p(i->second.midpoint(),i->second.nextb->midpoint());
  }
}

void dumphull_ps()
{multimap<double,point*>::iterator i;
 xy pnt,pnt1;
 int j;
 widen(5);
 for (i=convexhull.begin(),j=0;i!=convexhull.end();i++,j++)
     {setcolor(0,0,0);
      fprintf(psfile,"%d %d moveto (%.3f) show\n",10,3*j+10,i->first);
      setcolor(1,1,0);
      if (j)
         line2p(pnt,*i->second);
      else
         pnt1=*i->second;
      pnt=*i->second;
      }
 line2p(pnt,pnt1);
 widen(0.2);
 }

double edge::length()
{
  xy c,d;
  c=*a;
  d=*b;
  return dist(c,d);
}

bool goodcenter(xy a,xy b,xy c,xy d)
/* a is the proposed starting point; b, c, and d are the three closest
   points to a. a has to be on the same side of at least two sides as the
   interior of the triangle.

   If a is inside the triangle, the convex hull procedure will succeed (as
   long as no two points coincide). If a is out a corner, the procedure
   is guaranteed to fail. If a is out a side, it may or may not fail. In
   a lozenge, the procedure succeeds when a is out a side and the triangle
   is not straight (the usual cause for rejection in the lozenge is a
   straight triangle). The locus of a which is inside the triangle is
   quite tiny (about 1/4n). In Independence Park, however, the locus of
   a which is inside the triangle is most of the park. Points in the street
   are mostly out the side of the triangle, but result in failure. The best
   way to pick the point, then, is to allow points out the side, but check
   for failure, and if it happens, try another point.
   */
{double A,B,C,D,perim;
 int n;
 A=area3(b,c,d);
 B=area3(a,c,d);
 C=area3(b,a,d);
 D=area3(b,c,a);
 if (A<0)
    {A=-A;
     B=-B;
     C=-C;
     D=-D;
     }
 n=(B>A/THR)+(C>A/THR)+(D>A/THR);
 if (fabs(B/THR)>A || fabs(C/THR)>A || fabs(D/THR)>A)
    n=0;
 perim=dist(b,c)+dist(c,d)+dist(d,b);
 if (A<perim*perim/THR)
    n=0;
 return n>1;
 }

void pointlist::maketin(string filename,bool colorfibaster)
/* Makes a triangulated irregular network. If <3 points, throws notri without altering
   the existing TIN. If two points are equal, or close enough to likely cause problems,
   throws samepnts; the TIN is partially constructed and will have to be destroyed.
   */
{ptlist::iterator i;
 multimap<double,point*>::iterator j,k,inspos,left,right;
 vector<point*> visible; // points of convex hull visible from new point
 int m,m2,n,edgeoff,val,maxedges,flipcount,passcount,cycles;
 bool fail;
 double maxdist,mindist,idist,minx,miny,maxx,maxy;
 xy A,B,C,farthest;
 if (points.size()<3)
    throw notri;
 startpnt=xy(0,0);
 for (i=points.begin();i!=points.end();i++)
     startpnt+=i->second;
 startpnt/=points.size();
 edges.clear();
 //edges.reserve(maxedges=3*points.size()-6); //must reserve space to avoid moving, since edges point to each other
 //FIXME: points will be added for min/max/saddle, and each point will add three more edges.
 //How many extrema can there be, given the number of shot points?
 //startpnt has to be
 //within or out the side of the triangle formed by the three nearest points.
 //In a 100-point asteraceous pattern, the centroid is out one corner, and
 //the first triangle is drawn negative, with point 0 connected wrong.
 startpnt=points.begin()->second;
 if (filename.length())
 {
   psopen(filename.c_str());
   psprolog();
 }
 for (m2=0,fail=true;m2<100 && fail;m2++)
     {edges.clear();
      convexhull.clear();
      for (m=0;m<100;m++)
          {outward.clear();
           for (i=points.begin();i!=points.end();i++)
               outward.insert(ipoint(dist(startpnt,i->second),&i->second));
           for (j=outward.begin(),n=0;j!=outward.end();j++,n++)
               {if (n==0)
                   A=*(j->second);
                if (n==1)
                   B=*(j->second);
                if (n==2)
                   C=*(j->second);
                farthest=*(j->second);
                }
           //printf("m=%d startpnt=(%f,%f)\n",m,startpnt.east(),startpnt.north());
           if (m>0 && goodcenter(startpnt,A,B,C))
	   {
	     //printf("m=%d found good center\n",m);
             break;
	   }
           if (m&4)
	     startpnt=rand2p(startpnt,farthest);
	   else
	     startpnt=rand2p(startpnt,C);
           }
      // The point (-7.8578111411563043,-4.6782453265676276) came up in a run and caused the program to crash.
      //startpnt=xy(-7.8578111411563043,-4.6782453265676276);
      //printf("Took %d tries to choose startpnt=(%f,%f)\n",m,startpnt.east(),startpnt.north());
      miny=maxy=startpnt.north();
      minx=maxx=startpnt.east();
      for (i=points.begin();i!=points.end();i++)
          {if (i->second.east()>maxx)
              maxx=i->second.east();
           if (i->second.east()<minx)
              minx=i->second.east();
           if (i->second.north()>maxy)
              maxy=i->second.north();
           if (i->second.north()<miny)
              miny=i->second.north();
           }
      if (filename.length())
      {
        setscale(minx,miny,maxx,maxy);
        startpage();
        setcolor(0,0,1);
        dot(startpnt);
        setcolor(1,.5,0);
        for (i=points.begin();i!=points.end();i++)
            dot(i->second);
        endpage();
      }
      j=outward.begin();
      //printf("edges %d\n",edges.size());
      //edges.resize(1);
      edges[0].a=j->second;
      j->second->line=&(edges[0]);
      convexhull.insert(ipoint(dir(startpnt,*(j->second)),j->second));
      j++;
      edges[0].b=j->second;
      j->second->line=&(edges[0]);
      edges[0].nexta=edges[0].nextb=&(edges[0]);
      convexhull.insert(ipoint(dir(startpnt,*(j->second)),j->second));
      //printf("edges %d\n",edges.size());
      /* Before:
         A-----B
         |    /|
         |   / |
         |  /  |
         | /   |
         |/    |
         C-----D
         
                  E
         outward=(D,C,B,A,E)
         edges=(DC,BD,BC,AB,AC)
         After:
         edges=(DC,BD,BC,AB,AC,EC,ED,EB)
         */
      for (j++;j!=outward.end();j++)
          {//printf("\nSTART LOOP\n");
           convexhull.insert(ipoint(dir(startpnt,*(j->second)),j->second));
           inspos=convexhull.find(dir(startpnt,*(j->second)));
           // First find how much the convex hull subtends as seen from the new point,
           // expressed as distance from the startpnt to the line between an old point and the new point.
           mindist=dist(startpnt,*(j->second));
           maxdist=-mindist;
           for (left=inspos,idist=n=0;/*idist>=maxdist && */n<convexhull.size();left--,n++)
               {//printf("n=%d left->second=%p ",n,left->second);
                if (left->second==j->second)
                   idist=0;
                else
                   {idist=pldist(startpnt,*(left->second),*(j->second)); //i points to a list of points, j to a map of pointers to points
                    if (idist>maxdist)
                       maxdist=idist;
                    }
                //printf("idist=%f\n",idist);
                if (left==convexhull.begin())
                   left=convexhull.end();
                }
           for (right=inspos,idist=n=0;/*idist<=mindist && */n<convexhull.size();right++,n++)
               {if (right==convexhull.end())
                   right=convexhull.begin();
                if (right->second==j->second)
                   idist=0;
                else
                   {idist=pldist(startpnt,*(right->second),*(j->second));
                    if (idist<mindist)
                       mindist=idist;
                    }
                }
           //printf("maxdist=%f mindist=%f\n",maxdist,mindist);
           // Then find which convex hull points are the rightmost and leftmost that can be seen from the new point.
           for (left=inspos,idist=n=0;n<2||idist<maxdist;left--,n++)
               {//putchar('<');
                if (left->second==j->second)
                   idist=0;
                else
                   idist=pldist(startpnt,*(left->second),*(j->second));
                if (left==convexhull.begin())
                   left=convexhull.end();
                }
           left++;
           for (right=inspos,idist=n=0;n<2||idist>mindist;right++,n++)
               {//putchar('>');
                if (right==convexhull.end())
                   right=convexhull.begin();
                if (right->second==j->second)
                   idist=0;
                else
                   idist=pldist(startpnt,*(right->second),*(j->second));
                }
           right--;
           //putchar('\n');
           /*if (left==convexhull.begin())
              left=convexhull.end();
           if (right==convexhull.end())
              right=convexhull.begin();*/
           // Now make a list of the visible points. There are 3 on average.
           visible.clear();
           edgeoff=edges.size();
           for (k=left,n=0,m=1;m;k++,n++,m++)
               {if (k==convexhull.end())
                   k=convexhull.begin();
                if (k!=inspos) // skip the point just added - don't join it to itself
                   {visible.push_back(k->second);
                    edges[edges.size()].a=j->second; // this adds one element, hence -1 in next line
                    edges[edges.size()-1].b=k->second;
                    printf("Adding edge from %p to %p\n",j->second,k->second);
                    }
                if (k==right || n==maxedges)
		  m=-1;
                }
           val=--n; //subtract one for the point itself
           //printf("%d points visible\n",n);
           // Now delete old convex hull points that are now in the interior.
           for (m=1;m<visible.size()-1;m++)
               if (convexhull.erase(dir(startpnt,*visible[m]))>1)
                  throw(samepnts);
           //dumppoints();
           //dumpedges();
           for (n=0;n<val;n++)
               {edges[(n+1)%val+edgeoff].nexta=&edges[n+edgeoff];
                edges[n+edgeoff].nextb=visible[n]->line->next(visible[n]);
                visible[n]->line->setnext(visible[n],&edges[n+edgeoff]);
                }
           for (fail=false,n=edgeoff;n<edges.size();n++)
               if (edges[n].nexta==NULL)
                  fail=true;
           if (fail)
	   {
	     //dumpedges();
             break;
           }
           //dumpedges();
           j->second->line=&edges[edgeoff];
           visible[val-1]->line=&edges[edgeoff+val-1];
           /*startpage();
           //dumphull();
           dot(startpnt);
           dumphull_ps();
           dumpedges_ps(colorfibaster);
           endpage();*/
           //dumpedges();
           }
      }
 if (fail)
   throw flattri; // Failing to make a proper TIN, after trying a hundred start points, normally means that all triangles are flat.
 if (filename.length())
 {
   startpage();
   dumpedges_ps(colorfibaster);
   //dumpnext_ps();
   dot(startpnt);
   endpage();
 }
 flipcount=passcount=0;
 //debugdel=1;
 /* The flipping algorithm can take quadratic time, but usually does not
    on real-world data. It can also get stuck in a loop because of roundoff error.
    This requires at least five points in a perfect circle with nothing else
    inside. The test datum {ring(1000);rotate(30);} results in flipping edges
    around 13 points forever. To stop this, I put a cap on the number of passes.
    The worst cases are ring with lots of rotation and ellipse. They take about
    280 passes for 1000 points. I think that a cap of 1 pass per 3 points is
    reasonable.
    */
 do {for (m=n=0;n<edges.size();n++)
         if (!edges[n].delaunay())
            {//printf("Flipping edge %d\n",n);
             edges[n].flip(&topopoints);
             m++;
             flipcount++;
             //debugdel=0;
             if (n>680 && n<680)
                {startpage();
                 dumpedges_ps(colorfibaster);
                 endpage();
                 }
             //debugdel=1;
             }
     debugdel=0;
     if (filename.length())
     {
       startpage();
       dumpedges_ps(colorfibaster);
       //dumpnext_ps();
       endpage();
     }
     //debugdel=1;
     passcount++;
     } while (m && passcount*3<=points.size());
 //printf("Total %d edges flipped in %d passes\n",flipcount,passcount);
 if (filename.length())
 {
   startpage();
   dumpedges_ps(colorfibaster);
   dot(startpnt);
   endpage();
   pstrailer();
   psclose();
 }
 }

void pointlist::makegrad(double corr)
// Compute the gradient at each point.
// corr is a correlation factor which is how much the slope
// at one end of an edge affects the slope at the other.
{ptlist::iterator i;
 int n,m;
 edge *e;
 double zdiff,zxtrap,zthere;
 xy gradthere,diff;
 double sum1,sumx,sumy,sumz,sumxx,sumxy,sumxz,sumzz,sumyy,sumyz;
 for (i=points.begin();i!=points.end();i++)
     i->second.gradient=xy(0,0);
 for (n=0;n<10;n++)
     {for (i=points.begin();i!=points.end();i++)
          {i->second.gradient=xy(0,0);
           sum1=sumx=sumy=sumz=sumxx=sumxy=sumxz=sumzz=sumyy=sumyz=0;
           for (m=0,e=i->second.line;m==0 || e!=i->second.line;m++,e=e->next(&i->second))
               if (!e->broken)
                  {gradthere=e->otherend(&i->second)->gradient;
                   diff=(xy)(*e->otherend(&i->second))-(xy)i->second;
                   zdiff=e->otherend(&i->second)->elev()-i->second.elev();
                   zxtrap=zdiff-dot(gradthere,diff);
                   zthere=zdiff+corr*zxtrap;
                   sum1+=1;
                   sumx+=diff.east();
                   sumy+=diff.north();
                   sumz+=zthere;
                   sumxx+=diff.east()*diff.east();
                   sumyy+=diff.north()*diff.north();
                   sumzz+=zthere*zthere;
                   sumxy+=diff.east()*diff.north();
                   sumxz+=diff.east()*zthere;
                   sumyz+=diff.north()*zthere;
                   }
           //printf("point %d sum1=%f zdiff=%f zthere=%f\n",i->first,sum1,zdiff,zthere);
           if (sum1)
              {sum1++; //add the point i to the set
               sumx/=sum1;
               sumy/=sum1;
               sumz/=sum1;
               sumxx/=sum1;
               sumyy/=sum1;
               sumzz/=sum1;
               sumxy/=sum1;
               sumxz/=sum1;
               sumyz/=sum1;
               sumxx-=sumx*sumx;
               sumyy-=sumy*sumy;
               sumzz-=sumz*sumz;
               sumxy-=sumx*sumy;
               sumxz-=sumx*sumz;
               sumyz-=sumy*sumz;
               /* Gradient is computed by this matrix equation:
                  (xx xy)   (gradx)
                  (     ) × (     ) = (xz yz)
                  (xy yy)   (grady) */
               i->second.newgradient=xy(sumxz/sumxx,sumyz/sumyy);
	       /*if (i->first==63)
		 printf("sumxz %f sumxx %f sumyz %f sumyy %f\n",sumxz,sumxx,sumyz,sumyy);*/
               }
           else
              fprintf(stderr,"Warning: point at address %p has no edges that don't cross breaklines\n",&i->second);
           }
      for (i=points.begin();i!=points.end();i++)
          {i->second.oldgradient=i->second.gradient;
           i->second.gradient=i->second.newgradient;
           }
      }
 }

void pointlist::maketriangles()
{
  int i,j;
  point *a,*b,*c,*d;
  edge *e;
  triangle cib,*t;
  triangles.clear();
  triangles.reserve(edges.size()*3/2);
  for (i=0;i<edges.size();i++)
  {
    a=edges[i].a;
    b=edges[i].b;
    e=edges[i].next(b);
    c=e->otherend(b);
    d=e->next(c)->otherend(c);
    if (a<b && a<c && a==d)
    {
      cib.a=c;
      cib.b=b;
      cib.c=a;
      triangles.push_back(cib);
      edges[i].tria=&triangles[triangles.size()-1];
    }
    a=edges[i].b;
    b=edges[i].a;
    e=edges[i].next(b);
    c=e->otherend(b);
    d=e->next(c)->otherend(c);
    if (a<b && a<c && a==d)
    {
      cib.a=c;
      cib.b=b;
      cib.c=a;
      triangles.push_back(cib);
      edges[i].trib=&triangles[triangles.size()-1];
    }
  }
  for (i=0;i<edges.size()*2;i++)
  {
    j=i%edges.size();
    if (!edges[j].tria)
      edges[j].tria=edges[j].nextb->tri(edges[j].b);
    if (!edges[j].trib)
      edges[j].trib=edges[j].nexta->tri(edges[j].a);
  }
  for (i=0;i<edges.size();i++)
    if (edges[i].tria && edges[i].trib)
    {
      edges[i].tria->setneighbor(edges[i].trib);
      edges[i].trib->setneighbor(edges[i].tria);
    }
}
