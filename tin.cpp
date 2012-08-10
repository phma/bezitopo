/******************************************************/
/*                                                    */
/* tin.cpp - triangulated irregular network           */
/*                                                    */
/******************************************************/

#include <map>
#include <vector>
#include <cmath>
#include "bezitopo.h"
#include "tin.h"
#include "ps.h"
#include "pointlist.h"

#define THR 16777216
//for goodcenter

using std::map;
using std::multimap;
using std::vector;

edge* edge::next(point* end)
{if (end==a)
    return nexta;
 else
    return nextb;
 }

void edge::setnext(point* end,edge* enext)
{if (end==a)
    nexta=enext;
 else
    nextb=enext;
 }

point* edge::otherend(point* end)
{if (end==a)
    return b;
 else
    return a;
 }

void edge::dump()
{printf("addr=%p a=%d b=%d nexta=%p nextb=%p\n",this,revpoints[a],revpoints[b],nexta,nextb);
 }

void edge::flip()
/* Given an edge which is a diagonal of a quadrilateral,
   sets it to the other diagonal.
   */
{edge *temp1,*temp2;
 int i,size;
 size=points.size();
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
 tempa=nexta->otherend(a);
 tempb=nextb->otherend(b);
 return (!isinterior()) || ::delaunay(*a,*b,*tempa,*tempb);
 }

multimap<double,point*> convexhull;
// The points are ordered by their azimuth from the starting point.
xy startpnt;
multimap<double,point*> outward;
// The points are ordered by their distance from the starting point.
vector<edge> edgelist;

void dumphull()
{multimap<double,point*>::iterator i;
 printf("dump convex hull:\n");
 for (i=convexhull.begin();i!=convexhull.end();i++)
     printf("az=%f pt=%p\n",i->first,i->second);
 //printf("begin=%p end=%p rbegin=%p rend=%p\n",convexhull.begin(),convexhull.end(),convexhull.rbegin(),convexhull.rend());
 printf("end dump\n");
 }

void dumpedges()
{vector<edge>::iterator i;
 printf("dump edges:\n");
 for (i=edgelist.begin();i!=edgelist.end();i++)
     i->dump();
 printf("end dump\n");
 }

void dumpedges_ps()
{vector<edge>::iterator i;
 int n;
 for (i=edgelist.begin(),n=0;i!=edgelist.end();i++,n++)
     line(*i,n);
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

void maketin()
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
 edgelist.clear();
 edgelist.reserve(maxedges=3*points.size()-6); //must reserve space to avoid moving, since edges point to each other
 //FIXME: points will be added for min/max/saddle, and each point will add three more edges.
 //How many extrema can there be, given the number of shot points?
 //startpnt has to be
 //within or out the side of the triangle formed by the three nearest points.
 //In a 100-point asteraceous pattern, the centroid is out one corner, and
 //the first triangle is drawn negative, with point 0 connected wrong.
 startpnt=points.begin()->second;
 psopen("bezitopo.ps");
 psprolog();
 for (m2=0,fail=true;m2<100 && fail;m2++)
     {edgelist.clear();
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
           printf("m=%d startpnt=(%f,%f)\n",m,startpnt.east(),startpnt.north());
           if (m>3 && goodcenter(startpnt,A,B,C))
              break;
           startpnt=rand2p(startpnt,farthest);
           }
      printf("startpnt=(%f,%f)\n",startpnt.east(),startpnt.north());
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
      setscale(minx,miny,maxx,maxy);
      startpage();
      setcolor(0,0,1);
      dot(startpnt);
      setcolor(1,.5,0);
      for (i=points.begin();i!=points.end();i++)
          dot(i->second);
      endpage();
      j=outward.begin();
      printf("edgelist %d\n",edgelist.size());
      edgelist.resize(1);
      edgelist[0].a=j->second;
      j->second->line=&(edgelist[0]);
      convexhull.insert(ipoint(dir(startpnt,*(j->second)),j->second));
      j++;
      edgelist[0].b=j->second;
      j->second->line=&(edgelist[0]);
      edgelist[0].nexta=edgelist[0].nextb=&(edgelist[0]);
      convexhull.insert(ipoint(dir(startpnt,*(j->second)),j->second));
      printf("edgelist %d\n",edgelist.size());
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
         edgelist=(DC,BD,BC,AB,AC)
         After:
         edgelist=(DC,BD,BC,AB,AC,EC,ED,EB)
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
           edgeoff=edgelist.size();
           for (k=left,n=0,m=1;m;k++,n++,m++)
               {if (k==convexhull.end())
                   k=convexhull.begin();
                if (k!=inspos) // skip the point just added - don't join it to itself
                   {visible.push_back(k->second);
                    edgelist.resize(edgelist.size()+1);
                    edgelist[edgelist.size()-1].a=j->second;
                    edgelist[edgelist.size()-1].b=k->second;
                    //printf("Adding edge from %p to %p\n",j->second,k->second);
                    }
                if (k==right || n==maxedges) m=-1;
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
               {edgelist[(n+1)%val+edgeoff].nexta=&edgelist[n+edgeoff];
                edgelist[n+edgeoff].nextb=visible[n]->line->next(visible[n]);
                visible[n]->line->setnext(visible[n],&edgelist[n+edgeoff]);
                }
           for (fail=false,n=edgeoff;n<edgelist.size();n++)
               if (edgelist[n].nexta==NULL)
                  fail=true;
           if (fail)
              {dumpedges();
               break;
               }
           //dumpedges();
           j->second->line=&edgelist[edgeoff];
           visible[val-1]->line=&edgelist[edgeoff+val-1];
           /*startpage();
           //dumphull();
           dot(startpnt);
           dumphull_ps();
           dumpedges_ps();
           endpage();*/
           //dumpedges();
           }
      }
 startpage();
 dumpedges_ps();
 dot(startpnt);
 endpage();
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
 do {for (m=n=0;n<edgelist.size();n++)
         if (!edgelist[n].delaunay())
            {//printf("Flipping edge %d\n",n);
             edgelist[n].flip();
             m++;
             flipcount++;
             //debugdel=0;
             if (n>680 && n<680)
                {startpage();
                 dumpedges_ps();
                 endpage();
                 }
             //debugdel=1;
             }
     debugdel=0;
     startpage();
     dumpedges_ps();
     endpage();
     //debugdel=1;
     passcount++;
     } while (m && passcount*3<=points.size());
 printf("Total %d edges flipped in %d passes\n",flipcount,passcount);
 startpage();
 dumpedges_ps();
 dot(startpnt);
 endpage();
 //edgelist[3].flip();
 pstrailer();
 psclose();
 }

void makegrad(double corr)
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
                   sumz+=zdiff;
                   sumxx+=diff.east()*diff.east();
                   sumyy+=diff.north()*diff.north();
                   sumzz+=zdiff*zdiff;
                   sumxy+=diff.east()*diff.north();
                   sumxz+=diff.east()*zdiff;
                   sumyz+=diff.north()*zdiff;
                   }
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
               i->second.newgradient=xy(sumxz/sqrt(sumxx),sumyz/sqrt(sumyy));
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
