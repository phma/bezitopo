/******************************************************/
/*                                                    */
/* test.h - test patterns and functions               */
/*                                                    */
/******************************************************/

/* Test patterns:
   1. Asteraceous pattern. r=sqrt(n+0.5), θ=2πφn.
   2. Square root spiral.
   3. Square lattice.
   4. A row of points on one line.
   5. All points except two lie on one line; the last two are on opposite sides
      of the line. This tests that the triangulator works properly when it cannot
      make any triangles until the last two points.
   6. Hexagonal lattice. The Delaunay triangulation is ambiguous.
   7. Points on a circle, angles are multiples of 2πφ. Ditto.
   8. Points on a slightly eccentric ellipse. This usually takes O(n²) flips.
   9. Points on a circle, equally spaced. 
   Test operations:
   1. Rotate points by (0.6,0.8) n times. This introduces LSB errors
      to test numerical stability.
   
   For worst-case time of the Delaunay algorithm, do this:
   cir 1 rot 1 cir 3 rot 1 cir 5 rot 1 ... cir 1999 rot 1
   This produces points in a circle, displaced by a few picometers by the rotation.
   The mean of the points, however, is off by a micrometer. They will be added to
   the triangulation in order of distance from point 1, then all interior lines
   will be flipped many times.
   */

#define RUGAE 0
#define HYPAR 1
#define CIRPAR 2
void setsurface(int surf);
double sqr(double x);
void dumppoints();
void dumppointsvalence();
void aster(int n);
void ring(int n);
void regpolygon(int n);
void ellipse(int n);
void longandthin(int n);
void straightrow(int n);
void lozenge(int n);
void rotate(int n);
void movesideways(double sw);
void enlarge(double sc);
extern xy (*testsurfacegrad)(xy pnt);
