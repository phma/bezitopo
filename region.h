/******************************************************/
/*                                                    */
/* region.h - regions                                 */
/*                                                    */
/******************************************************/
/* A region is some piece of land surrounded by a boundary.
 * It has these methods:
 * + List points on or inside the boundary.
 * + Tell whether a point is in the region.
 * + Tell whether a line segment intersects the boundary.
 * 
 * These kinds of regions are available:
 * + Circular (for testing). The point list consists of the center.
 * + Polyline.
 * + Intersection (for surfaces that are made by subtracting surfaces).
 * 
 * They are used for delimiting areas over which a volume is to be computed
 * and for delimiting areas within which contours are valid.
 * A region can have a hole in it; for instance, a lot with a building,
 * where the contours passing through the building are omitted.
 */
