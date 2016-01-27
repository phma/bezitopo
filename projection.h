/******************************************************/
/*                                                    */
/* projection.h - map projections                     */
/*                                                    */
/******************************************************/
/* All map projections used in surveying are conformal. These are in use:
 * • Lambert conformal conic
 * • Stereographic at the poles
 * • Axial Mercator, possibly
 * • Transverse Mercator
 * • Oblique Mercator, in the Alaska panhandle
 * 
 * Lambert conformal conic on the ellipsoid can be done by conformally projecting
 * the ellipsoid to a sphere and using the spherical formula for the projection.
 * This does not work for transverse Mercator: the result is conformal, but
 * the scale on the meridian is not constant. As to oblique Mercator, I'll let
 * the Alaskan surveyors figure that one out.
 * 
 * Stereographic and axial Mercator are special cases of conformal conic.
 * Stereographic does not require checking, but Mercator does, otherwise
 * it results in raising a complex number to the 0 power, subtracting 1, and
 * dividing by 0.
 */
#include <array>
#include "ellipsoid.h"

struct latlong
{
  double lat;
  double lon;
};

class Projection
{
public:
  Projection();
  virtual latlong gridToLatlong(xy grid)=0;
  virtual xyz gridToGeocentric(xy grid)=0;
  virtual xy geocentricToGrid(xyz geoc)=0;
  virtual xy latlongToGrid(latlong ll)=0;
protected:
  ellipsoid *ellip;
};
