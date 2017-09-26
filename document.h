/******************************************************/
/*                                                    */
/* document.h - main document class                   */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */

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
#define unsetsource 9

class document
{
public:
  int curlayer;
  xyz offset;
  std::vector<objrec> objlist;
  std::vector<layer> layers;
  std::vector<pointlist> pl;
  /* pointlists[0] is the points downloaded from the total station.
   * pointlists[1] and farther are used for surfaces.
   */
  void copytopopoints(criteria crit);
  void copytopopoints(int dst,int src);
  int readpnezd(std::string fname,bool overwrite=false);
  int writepnezd(std::string fname);
  int readpenzd(std::string fname,bool overwrite=false);
  int writepenzd(std::string fname);
  void addobject(drawobj *obj); // obj must be created with new
  virtual void writeXml(std::ofstream &ofile);
  void changeOffset (xyz newOffset);
};

#endif
