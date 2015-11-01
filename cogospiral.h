/******************************************************/
/*                                                    */
/* cogospiral.h - intersections of spirals            */
/*                                                    */
/******************************************************/
/* To find the intersection of two spirals, given two points near it on each:
 * 1. Find the intersection of the lines defined by the points.
 * 2. Interpolate or extrapolate a new point on each curve.
 * 3. Discard the farthest pair of points where one is on one curve and the other
 *    is on the other. If the farthest points are both the new points, or there
 *    are no new points because the lines are parallel, give up.
 * 4. If the closest points are identical, or on both curves the points are so
 *    close together that no point between them can be computed, you're done.
 *    Otherwise return to step 1.
 * To find all the intersections, pick points on both curves as in segment::closest,
 * then pick pairs of points on each curve and run the above algorithm.
 * 
 * As segments and arcs are special cases of spiralarcs, the algorithm works
 * for them too. Intersections of segments and arcs can be computed by formula;
 * intersections involving a spiral must be computed iteratively, since the
 * spiral cannot be written in closed form. Intersections involving an arc
 * of very small delta should be done iteratively, to avoid loss of precision.
 */
