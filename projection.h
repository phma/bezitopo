/******************************************************/
/*                                                    */
/* projection.h - map projections                     */
/*                                                    */
/******************************************************/
/* Copyright 2012,2016-2018 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */
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
  virtual xy geocentricToGrid(xyz geoc)=0;
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
  LambertConicSphere(double Meridian,double Parallel);
  LambertConicSphere(double Meridian,double Parallel0,double Parallel1);
  virtual latlong gridToLatlong(xy grid);
  virtual xyz gridToGeocentric(xy grid);
  virtual xy geocentricToGrid(xyz geoc);
  virtual xy latlongToGrid(latlong ll);
  virtual double scaleFactor(xy grid);
  virtual double scaleFactor(latlong ll);
protected:
  double centralParallel;
  double centralMeridian;
  double poleY;
  double exponent;
  double coneScale;
  void setParallel(double Parallel);
  double scaleRatioLog(double Parallel0,double Parallel1);
};

class StereographicSphere: public Projection
/* This is used to tell whether a point is inside a g1boundary,
 * by projecting the sphere so that a point that's highly unlikely
 * to be on a g1boundary is mapped to infinity.
 */
{
public:
  StereographicSphere();
  StereographicSphere(Quaternion Rotation);
  virtual latlong gridToLatlong(xy grid);
  virtual xyz gridToGeocentric(xy grid);
  virtual xy geocentricToGrid(xyz geoc);
  virtual xy latlongToGrid(latlong ll);
  virtual double scaleFactor(xy grid);
  virtual double scaleFactor(latlong ll);
protected:
  Quaternion rotation;
};

extern StereographicSphere sphereStereoArabianSea;

/* This is how I'll do transverse Mercator:
 * Transverse Mercator on a sphere is easy. Transverse Mercator on an ellipsoid
 * can be computed as the convolution of two conformal maps: transverse Mercator
 * on a sphere and a map that corrects the length along the central meridian.
 * The length along the central meridian of an ellipsoid is an elliptic integral,
 * which can be approximated with many sufficiently small pieces. Then compute
 * the function to correct the length as a Fourier series. It is automatically
 * analytic, hence conformal, and the scale can be computed by differentiating it.
 */
