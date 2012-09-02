/******************************************************/
/*                                                    */
/* angle.cpp - angles as binary fractions of rotation */
/*                                                    */
/******************************************************/

#include "angle.h"

double sin(int angle)
{return sinl(angle*M_PIl/2147483648.);
 }

double cos(int angle)
{return cosl(angle*M_PIl/2147483648.);
 }

double sinhalf(int angle)
{return sinl(angle*M_PIl/4294967296.);
 }

double coshalf(int angle)
{return cosl(angle*M_PIl/4294967296.);
 }

double tanhalf(int angle)
{return tanl(angle*M_PIl/4294967296.);
 }

double cosquarter(int angle)
{return cosl(angle*M_PIl/8589934592.);
 }

double tanquarter(int angle)
{return tanl(angle*M_PIl/8589934592.);
 }

int atan2i(double y,double x)
{return rint(atan2(y,x)/M_PIl*2147483648.);
 }

double bintorot(int angle)
{
  return angle/2147483648.;
}

double bintogon(int angle)
{
  return bintorot(angle)*400;
}

double bintodeg(int angle)
{
  return bintorot(angle)*360;
}

double bintomin(int angle)
{
  return bintorot(angle)*21600;
}

double bintosec(int angle)
{
  return bintorot(angle)*1296000;
}

double bintorad(int angle)
{
  return bintorot(angle)*M_PIl*2;
}
