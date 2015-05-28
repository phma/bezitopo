/******************************************************/
/*                                                    */
/* document.h - main document class                   */
/*                                                    */
/******************************************************/

/* A document contains these parts:
 * • A list of pointlists, some of which may have TINs
 * • A list of alignments, each of which is linked to a pointlist
 * • A list of drawing objects
 * • A list of layers
 * • An origin, which is either a point on the ellipsoid and a map projection
 *   or a point in Cartesian coordinates.
 */
