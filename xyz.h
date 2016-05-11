/******************************************************/
/*                                                    */
/* xyz.h - classes for points and gradients           */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
/* The implementation is in point.cpp. The xyz class has to be separated
 * because it's used in both drawobj and point.
 */

#ifndef XYZ_H
#define XYZ_H
#include <fstream>

class xyz;

class xy
{
public:
  xy(double e,double n);
  xy(xyz point);
  xy();
  double east() const;
  double north() const;
  double getx() const;
  double gety() const;
  double length() const;
  bool isfinite() const;
  bool isnan() const;
  void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
  virtual void roscat(xy tfrom,int ro,double sca,xy tto); // rotate, scale, translate
  virtual void writeXml(std::ofstream &ofile);
  friend xy operator+(const xy &l,const xy &r);
  friend xy operator+=(xy &l,const xy &r);
  friend xy operator-(const xy &l,const xy &r);
  friend xy operator-(const xy &r);
  friend xy operator*(const xy &l,double r);
  friend xy operator*(double l,const xy &r);
  friend xy operator/(const xy &l,double r);
  friend xy operator/=(xy &l,double r);
  friend bool operator!=(const xy &l,const xy &r);
  friend bool operator==(const xy &l,const xy &r);
  friend xy turn90(xy a);
  friend xy turn(xy a,int angle);
  friend double dist(xy a,xy b);
  friend int dir(xy a,xy b);
  friend double dot(xy a,xy b);
  friend double area3(xy a,xy b,xy c);
  friend class triangle;
  friend class point;
  friend class xyz;
  friend class qindex;
protected:
  double x,y;
};

class xyz
{
public:
  xyz(double e,double n,double h);
  xyz();
  xyz(xy en,double h);
  double east() const;
  double north() const;
  double elev() const;
  double getx() const;
  double gety() const;
  double getz() const;
  double lat(); // These assume a sphere and are used for converting geoid files.
  double lon(); // For latitude on the real Earth, see the ellipsoid class.
  bool isfinite() const;
  bool isnan() const;
  int lati();
  int loni();
  double length();
  void _roscat(xy tfrom,int ro,double sca,xy cis,xy tto);
  virtual void roscat(xy tfrom,int ro,double sca,xy tto);
  virtual void writeXml(std::ofstream &ofile);
  void setelev(double h)
  {
    z=h;
  }
  friend class xy;
  friend class triangle;
  friend double dist(xyz a,xyz b);
  friend bool operator==(const xyz &l,const xyz &r);
  friend xyz operator/(const xyz &l,const double r);
  friend xyz operator*=(xyz &l,double r);
  friend xyz operator/=(xyz &l,double r);
  friend xyz operator+=(xyz &l,const xyz &r);
  friend xyz operator-=(xyz &l,const xyz &r);
  friend xyz operator*(const xyz &l,const double r);
  friend xyz operator*(const double l,const xyz &r);
  friend xyz operator*(const xyz &l,const xyz &r); // cross product
  friend xyz operator+(const xyz &l,const xyz &r);
  friend xyz operator-(const xyz &l,const xyz &r);
protected:
  double x,y,z;
};

#endif
 