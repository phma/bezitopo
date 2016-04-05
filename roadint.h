/******************************************************/
/*                                                    */
/* roadint.h - road intersections                     */
/*                                                    */
/******************************************************/
#include "polyline.h"

/* A road intersection is placed at the intersection of two or more alignments.
 * Its boundary consists of alternating line segments, which have cross-sections
 * fitted to those of the alignments, and circular arcs concave outward.
 * The region is star-shaped about a point, normally the intersection of all
 * the roads. The surface is computed by some combination of filleting
 * the alignments and connecting them to the center point.
 */
