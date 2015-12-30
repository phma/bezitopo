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
#include <string>
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
xy cossin(double angle);
xy cossin(int angle);
xy cossinhalf(int angle);

int foldangle(int angle); // if angle is outside [-180°,180°), adds 360°
bool isinsector(int angle,int sectors);

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

/* Angles, azimuths, and bearings are expressed in text as follows:
 * Hex integer  Angle, deg  Angle, gon  Azimuth, deg  Azimuth, gon  Bearing, deg  Bearing, gon
 * 0x00000000   0°00′00″    0.0000      90°00′00″     100.0000      N90°00′00″E   N100.0000E
 * 0x0aaaaaab   30°00′00″   33.3333     60°00′00″     66.6667       N60°00′00″E   N066.6667E
 * 0x15555555   60°00′00″   66.6667     30°00′00″     33.3333       N30°00′00″E   N033.3333E
 * 0x168dfd71   63°26′06″   70.4833	27°33′54″     29.5167       N27°33′54″E   N029.5167E
 * 0x80000000   -360°00′00″ -400.0000   90°00′00″     100.0000      N90°00′00″E   N100.0000E
 * Internally, angles are measured counterclockwise, and azimuths/bearings are
 * counterclockwise from east. For I/O, angles can be measured either way and
 * azimuths/bearings are measured from north.
 * As azimuths or bearings, 0x80000000 and 0x00000000 are equivalent; as deltas they are not.
 */

#define SEC1 1657
#define MIN1 99421
#define DEG1 5965232
#define AT0512 0x80ae90e
// AT0512 is arctangent of 5/12, 22.619865°
#define DEG30 0x0aaaaaab
#define AT34 0x0d1bfae2
// AT34 is arctangent of 3/4, 36.8698976°
#define DEG45 0x10000000
#define DEG60 0x15555555
#define DEG90 0x20000000
#define DEG120 0x2aaaaaab
#define DEG180 0x40000000
#define DEG360 0x80000000

std::string bintoangle(int angle,int unitp);
int parseangle(std::string angstr,int unitp);
/* If parseangle is passed a string containing a degree or minus sign, it interprets
 * the string as degrees even if unitp is GON. If the string contains 'g',
 * it interprets it as gons.
 */
int parsesignedangle(std::string angstr,int unitp);
int parseazimuth(std::string angstr,int unitp);
int parsebearing(std::string angstr,int unitp);


#define DEGREE 0x0038ed00
#define GON 0x00383000
#define RADIAN 0x00381000
#define SEXAG0 0x20
#define SEXAG1 0x21
#define SEXAG2 0x22
#define SEXAG0P1 0x24
#define SEXAG1P1 0x25
#define SEXAG2P1 0x26
#define SEXAG0P2 0x28
#define SEXAG1P2 0x29
#define SEXAG2P2 0x2a
#endif
