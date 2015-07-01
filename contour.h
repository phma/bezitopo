/******************************************************/
/*                                                    */
/* contour.h - generates contours                     */
/*                                                    */
/******************************************************/

#include <vector>
#include "tin.h"

class pointlist;

std::vector<uintptr_t> contstarts(pointlist &pts,double elev);
