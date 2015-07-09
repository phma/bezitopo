/******************************************************/
/*                                                    */
/* contour.h - generates contours                     */
/*                                                    */
/******************************************************/

#include <vector>
#include "tin.h"
#include "polyline.h"

class pointlist;

std::vector<uintptr_t> contstarts(pointlist &pts,double elev);
polyline trace(uintptr_t edgep,double elev);
