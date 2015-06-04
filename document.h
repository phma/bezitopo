/******************************************************/
/*                                                    */
/* document.h - main document class                   */
/*                                                    */
/******************************************************/

/* A document contains these parts:
 * • A list of pointlists, some of which may have TINs
 * • A list of alignments
 * • A list of corridors, each of which is linked to an alignment and a pointlist
 * • A list of surfaces, which can be TINs, pointlists, hemispheres,
 *   or differences between surfaces
 * • A list of drawing objects
 * • A set of paper views, which themselves have lists of drawing objects
 * • A list of layers
 * • An origin, which is either a point on the ellipsoid and a map projection
 *   or a point in Cartesian coordinates.
 */

#include "pointlist.h"
#include "layer.h"
#include "objlist.h"

class document
{
  std::vector<objrec> objlist;
  std::vector<layer> layers;
  std::vector<pointlist> pl;
  void copytopopoints(criteria crit);
};
