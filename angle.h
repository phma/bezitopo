/******************************************************/
/*                                                    */
/* angle.h - angles as binary fractions of rotation   */
/*                                                    */
/******************************************************/

// Angles are represented as integers, with 2147483648 representing 360°.
// This file overloads the functions sin and cos.

#ifndef ANGLE_H
#define ANGLE_H
#include <cmath>
#include "measure.h"
#include "point.h"

#ifndef M_PIl
#define M_PIl M_PI
#endif

double sqr(double x);

double sin(int angle);
double cos(int angle);
double sinhalf(int angle);
double coshalf(int angle);
double tanhalf(int angle);
double cosquarter(int angle);
double tanquarter(int angle);
int atan2i(double y,double x);
int atan2i(xy vect); // range is [-536870912,536870912]
xy cossin(int angle);
xy cossinhalf(int angle);

double bintorot(int angle);
double bintogon(int angle);
double bintodeg(int angle);
double bintomin(int angle);
double bintosec(int angle);
double bintorad(int angle);
int rottobin(double angle);
int degtobin(double angle);
int mintobin(double angle);
int sectobin(double angle);
int gontobin(double angle);
int radtobin(double angle);

#define DEGREE 0x0038ed00
#define GON 0x00383000
#define SEXAG0 0x20
#define SEXAG1 0x21
#define SEXAG2 0x22
#endif
