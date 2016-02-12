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
 * 
 * The default constructor for a Lambert conic produces an axial Mercator
 * centered in the Bight of Benin. The default constructor for a transverse
 * Mercator produces one centered on the prime meridian.
 */
#include <array>
#include "ellipsoid.h"

class Projection
{
public:
  Projection();
  virtual latlong gridToLatlong(xy grid)=0;
  virtual xyz gridToGeocentric(xy grid)=0;
  //virtual xy geocentricToGrid(xyz geoc)=0;
  //geocentricToGrid is commented out until there's a method in ellipsoid to support it.
  virtual xy latlongToGrid(latlong ll)=0;
  /* The grid scale factor is the distance on the grid divided by the distance
   * on the ellipsoid. It is smallest at the center of the grid (central parallel
   * for Lambert conic, central meridian for transverse Mercator) and largest
   * at the periphery.
   * 
   * The scale factor of Lambert is easier to calculate from the latlong, but
   * that of the transverse Mercator may be easier to calculate from the
   * grid coordinates.
   */
  virtual double scaleFactor(xy grid)=0;
  virtual double scaleFactor(latlong ll)=0;
  ellipsoid *ellip;
protected:
  xy offset;
  double scale;
};

class LambertConicSphere: public Projection
{
public:
  LambertConicSphere();
  virtual latlong gridToLatlong(xy grid);
  virtual xyz gridToGeocentric(xy grid);
  //virtual xy geocentricToGrid(xyz geoc);
  virtual xy latlongToGrid(latlong ll);
  virtual double scaleFactor(xy grid);
  virtual double scaleFactor(latlong ll);
protected:
  double centralParallel;
  double centralMeridian;
  double poleY;
  double exponent;
  double coneScale;
};
