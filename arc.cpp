/******************************************************/
/*                                                    */
/*                                                    */
/*                                                    */
/******************************************************/

#include "arc.h"

xy arc::center()
{
  return ((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))/2*tanhalf(delta)));
}

/*xy arc::midpoint()
{return ((xy(start)+xy(end))/2+turn90((xy(end)-xy(start))*2*tanquarter(delta)));
 }*/

/* To find the nearest point on the arc to a point:
   If delta is less than 0x1000000 (2°48'45") in absolute value, use linear
   interpolation to find a starting point. If it's between 0x1000000 and
   0x40000000 (180°), use the bearing from the center. Between 0x40000000
   and 0x7f000000 (357°11'15"), use the bearing from the center, but use
   calong() instead of along(). From 0x7f000000 to 0x80000000, use linear
   interpolation and calong(). Then use parabolic interpolation to find
   the closest point on the circle.
   */
