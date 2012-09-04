/******************************************************/
/*                                                    */
/* angle.cpp - angles as binary fractions of rotation */
/*                                                    */
/******************************************************/

#include "angle.h"

double sin(int angle)
{return sinl(angle*M_PIl/1073741824.);
 }

double cos(int angle)
{return cosl(angle*M_PIl/1073741824.);
 }

double sinhalf(int angle)
{return sinl(angle*M_PIl/2147483648.);
 }

double coshalf(int angle)
{return cosl(angle*M_PIl/2147483648.);
 }

double tanhalf(int angle)
{return tanl(angle*M_PIl/2147483648.);
 }

double cosquarter(int angle)
{return cosl(angle*M_PIl/4294967296.);
 }

double tanquarter(int angle)
{return tanl(angle*M_PIl/4294967296.);
 }

int atan2i(double y,double x)
{return rint(atan2(y,x)/M_PIl*1073741824.);
 }

int atan2i(xy vect)
{
  return atan2i(vect.north(),vect.east());
}

xy cossin(int angle)
{
  return xy(cos(angle),sin(angle));
}

xy cossinhalf(int angle)
{
  return xy(coshalf(angle),sinhalf(angle));
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

int rottobin(double angle)
{
  double iprt,fprt;
  fprt=2*modf(angle/2,&iprt);
  if (fprt>=1)
    fprt-=2;
  if (fprt<-1)
    fprt+=2;
  return lrint(2147483648.*fprt);
}

int degtobin(double angle)
{
  return rottobin(angle/360);
}

int mintobin(double angle)
{
  return rottobin(angle/21600);
}

int sectobin(double angle)
{
  return rottobin(angle/1296000);
}

int gontobin(double angle)
{
  return rottobin(angle/400);
}

int radtobin(double angle)
{
  return rottobin(angle/M_PIl/2);
}
