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

#ifndef DOCUMENT_H
#define DOCUMENT_H
#include "pointlist.h"
#include "layer.h"
#include "objlist.h"

class document
{
public:
  int curlayer;
  std::vector<objrec> objlist;
  std::vector<layer> layers;
  std::vector<pointlist> pl;
  /* pointlists[0] is the points downloaded from the total station.
   * pointlists[1] and farther are used for surfaces.
   */
  void copytopopoints(criteria crit);
  int readpnezd(std::string fname,bool overwrite=false);
  int writepnezd(std::string fname);
  void addobject(drawobj *obj); // obj must be created with new
  virtual void writeXml(std::ofstream &ofile);
};

#endif
