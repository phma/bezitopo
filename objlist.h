/******************************************************/
/*                                                    */
/* objlist.h - list of drawing objects                */
/*                                                    */
/******************************************************/

#include <vector>
#include "drawobj.h"
#include "layer.h"
#include "color.h"

class objrec
{
public:
  drawobj *obj;
  unsigned short layr,ltype,colr,thik;
  //unsigned short getlinetype();
  //unsigned short getcolor();
  //unsigned short getthickness();
};
