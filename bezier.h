/******************************************************/
/*                                                    */
/* bezier.h - Bézier triangles                        */
/*                                                    */
/******************************************************/

#ifndef BEZIER_H
#define BEZIER_H
#include <vector>
#include "cogo.h"
#include "segment.h"
#define M_SQRT_3_4 0.86602540378443864676372317
#define M_SQRT_3 1.73205080756887729352744634
#define M_SQRT_1_3 0.5773502691896257645091487805
#define PT_MIN 0
#define PT_FLAT 1
#define PT_MAX 2
#define PT_SLOPE 3
#define PT_SADDLE 4
#define PT_MONKEY 5
#define PT_GRASS 6

#ifndef NDEBUG
class testfunc
{
public:
  double coeff[4];
  testfunc(double cub,double quad,double lin,double con);
  double operator()(double x);
};
double parabinter(testfunc func,double start,double startz,double end,double endz);
#endif

class triangle
/* A triangle has three corners and seven other control points, arranged as follows:
         a
       0   1
     2   3   4
   b   5   6   c
   The corners are arranged counterclockwise so that the area is always nonnegative
   (it can be zero in pathological cases such as all the points in a line, but then
   the elevation and gradient at a point are impossible to compute). It can compute
   the elevation and gradient at any point, but they are valid only inside the triangle.
   They are computed outside the triangle for finding the extrema or for extrapolating
   the surface outside the convex hull.
   
   There are two ways of computing the elevation and gradient within a triangle:
   1. Interpolate cubically using the ten control points.
   2. Interpolate cubically along a side, and interpolate the sideways slope
   linearly between the two endpoints. Do this for all three sides, then weight
   them by the reciprocal of the distance from the three sides.
   The first method produces slight creases along the sides, The second does not,
   but it is harder to find the extrema, and evaluating it outside the triangle
   can yield infinity.
   
   Point 3 is not stored, but is computed from the other control points.
   */
{
public:
  point *a,*b,*c; //corners
  double ctrl[7]; //There are 10 control points; the corners are three, and these are the elevations of the others.
  double peri,sarea;
  triangle *aneigh,*bneigh,*cneigh;
  int nocubedir; // set to MAXINT if critpoints have not been looked for
  double gradmat[2][3]; // to compute gradient from three partial gradients
  std::vector<xy> critpoints;
  std::vector<segment> subdiv;
  triangle();
  void setneighbor(triangle *neigh);
  double elevation(xy pnt);
  void setgradient(xy pnt,xy grad);
  double ctrlpt(xy pnt1,xy pnt2);
  void flatten();
  xyz gradient3(xy pnt);
  xy gradient(xy pnt);
  bool in(xy pnt);
  bool iscorner(point *v);
  triangle *nexttoward(xy pnt);
  triangle *findt(xy pnt,bool clip=false);
  double area();
  double perimeter();
  xy centroid();
  void setcentercp();
  void setgradmat();
  std::vector<double> xsect(int angle,double offset);
  double spelevation(int angle,double x,double y);
  int findnocubedir();
  double flatoffset();
  std::vector<xyz> slices(bool side);
  //void setctrl(bool flat);
  xy critical_point(double start,double startz,double end,double endz);
  std::vector<xy> criticalpts_side(bool side);
  xy spcoord(double x,double y);
  std::vector<xy> criticalpts_axis();
  void findcriticalpts();
  int pointtype(xy pnt);
  void subdivide();
  void addperimeter();
  void removeperimeter();
  uintptr_t edgepart(int subdir);
  std::vector<double> lohi();
  int subdir(uintptr_t edgepart);
  int proceed(int subdir,double elevation);
  bool crosses(int subdir,double elevation);
private:
  double vtxeloff(double off);
  void setsubslopes(segment &s);
};

double deriv0(std::vector<double> xsect);
double deriv1(std::vector<double> xsect);
double deriv2(std::vector<double> xsect);
double deriv3(std::vector<double> xsect);
double paravertex(std::vector<double> xsect);

#endif
