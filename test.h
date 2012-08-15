/******************************************************/
/*                                                    */
/* test.h - test patterns and functions               */
/*                                                    */
/******************************************************/

/* Test patterns:
   1. Asteraceous pattern. r=sqrt(n+0.5), θ=2πφn.
   2. Square root spiral.
   3. Square lattice.
   4. All points except two lie on one line; the last two are on opposite sides
      of the line. This tests that the triangulator works properly when it cannot
      make any triangles until the last two points.
   5. Hexagonal lattice. The Delaunay triangulation is ambiguous.
   6. Points on a circle. Ditto.
   7. Points on a slightly eccentric ellipse. This usually takes O(n²) flips.
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

void dumppoints();
void aster(int n);
void ring(int n);
void ellipse(int n);
void lozenge(int n);
void rotate(int n);
void movesideways(double sw);
