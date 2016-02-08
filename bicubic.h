/******************************************************/
/*                                                    */
/* bicubic.h - bicubic interpolation                  */
/*                                                    */
/******************************************************/
/* This is used for interpolating latitude-longitude grids (see sourcegeoid.cpp).
 * It may be used later for lattice DEMs, but I don't plan to.
 */

#include <array>

double beziersquare(array<double,16> controlPoints,double x,double y);
