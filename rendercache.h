/******************************************************/
/*                                                    */
/* rendercache.h - cache of renderings of drawobjs    */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#include <map>
#include "drawobj.h"
#include "halton.h"
#include "random.h"

/* The RenderCache is used for all objects visible in a canvas EXCEPT:
 * • points, which are drawn as three concentric circles sized in pixels;
 * • edges, which are not drawing objects and which are drawn as just one (sp)line.
 * If an object hasn't changed (as shown by its hash), and the pixel scale
 * is the same as when it was last rendered, the rendering is left alone.
 * If the scale has changed, it is rerendered with a probability depending
 * on the ratio. If the object has changed, it is always rerendered.
 * 
 * This works well, except in some cases involving blocks. If a block in the
 * drawing layer has a line in the setback layer, and you hide the setback
 * layer, the line will remain visible until you rerender everything or the
 * block is rerendered because the scale changes.
 */

class RenderItem
{
public:
  unsigned short colr;
  short thik;
  unsigned short ltype;
  bool present;
  unsigned hash;
  double pixelScale;
  std::vector<drawingElement> rendering;
};

class RenderCache
{
private:
  halton subrand;
  std::map<drawobj *,RenderItem> renderMap;
  bool shouldRerender(double oldScale,double newScale);
  std::map<drawobj *,RenderItem>::iterator next;
public:
  void clear();
  void clearPresent();
  void deleteAbsent();
  void checkInObject(drawobj *obj,double pixelScale,int layr,int colr,int thik,int ltype);
  RenderItem nextRenderItem();
};
