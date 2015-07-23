/******************************************************/
/*                                                    */
/* contour.h - generates contours                     */
/*                                                    */
/******************************************************/

#include <vector>
#include "tin.h"
#include "polyline.h"
#define CCHALONG 0.30754991027012474516361707317
// This is sqrt(4/27) of the way from 0.5 to 0. See clampcubic.

class pointlist;

std::vector<uintptr_t> contstarts(pointlist &pts,double elev);
polyline trace(uintptr_t edgep,double elev);
polyline intrace(triangle *tri,double elev);
bool ismarked(uintptr_t ep);
