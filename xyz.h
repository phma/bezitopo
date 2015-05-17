/******************************************************/
/*                                                    */
/* xyz.h - classes for points and gradients           */
/*                                                    */
/******************************************************/
/* The implementation is in point.cpp. The xyz class has to be separated
 * because it's used in both drawobj and point.
 */

#ifndef XYZ_H
#define XYZ_H

class xyz;

class xy
{
public:
  xy(double e,double n);
  xy(xyz point);
  xy();
  double east();
  double north();
  double length();
  friend xy operator+(const xy &l,const xy &r);
  friend xy operator+=(xy &l,const xy &r);
  friend xy operator-(const xy &l,const xy &r);
  friend xy operator*(const xy &l,double r);
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
  double east();
  double north();
  double elev();
  double length();
  void setelev(double h)
  {
    z=h;
  }
  friend class xy;
  friend class triangle;
  friend double dist(xyz a,xyz b);
  friend bool operator==(const xyz &l,const xyz &r);
  friend xyz operator/(const xyz &l,const double r);
  friend xyz operator*(const xyz &l,const double r);
  friend xyz operator*(const double l,const xyz &r);
  friend xyz operator+(const xyz &l,const xyz &r);
  friend xyz operator-(const xyz &l,const xyz &r);
protected:
  double x,y,z;
};

#endif
 