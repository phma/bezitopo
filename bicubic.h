/******************************************************/
/*                                                    */
/* bicubic.h - bicubic interpolation                  */
/*                                                    */
/******************************************************/
/* This is used for interpolating latitude-longitude grids (see sourcegeoid.cpp).
 * It may be used later for lattice DEMs, but I don't plan to.
 */

#include <array>
#include "xyz.h"

double beziersquare(std::array<double,16> controlPoints,double x,double y);
double bicubic(double swelev,xy swslope,double seelev,xy seslope,
	       double nwelev,xy nwslope,double neelev,xy neslope,
	       double x,double y);
