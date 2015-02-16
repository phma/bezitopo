/******************************************************/
/*                                                    */
/* xsection.h - cross sections                        */
/*                                                    */
/******************************************************/

#include <vector>
/* A cross section consists of a central part, which is made of any number
 * of line segments or splines, and two margins, which are slopes extending
 * to infinity. The margins may be empty, single, or double. If they are
 * double, one of them (normally the lower) may start at a point which is
 * not the outermost.
 * 
 * The following operations can be performed on cross sections:
 * • Take a cross section of a TIN surface at a specified point and direction.
 * • Construct a cross section from scratch.
 * • Add a line segment or spline to either side of a cross section.
 * • Add or delete margins.
 * • Interpolate between two cross sections.
 * • Given a cross section with double margins and one with single or empty margins,
 *   extend the one with double margins until it meets the other.
 * • Compute the elevation anywhere along a cross section, including the
 *   margins if and only if they are single.
 */

class xsection
{
private:
  std::vector<double> xs,zs,ctrls;
  int center,lmstart,rmstart;
  double lmargin[2],rmargin[2];
};
