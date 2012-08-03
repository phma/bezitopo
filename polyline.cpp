/******************************************************/
/*                                                    */
/*                                                    */
/*                                                    */
/******************************************************/

/* A polyline can be open or closed. Smoothing a polyline does this:
   If it is open, it tries various starting angles until it finds the shortest total length.
   If it is closed and has an odd number of points, it computes the two starting angles
   that make the ending angle the same, and chooses the one with the shortest total length.
   If it is closed and has an even number of points, it distributes the angular misclosure
   evenly among the points, and makes the shortest total length.
   */
