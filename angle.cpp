/******************************************************/
/*                                                    */
/* angle.cpp - angles as binary fractions of rotation */
/*                                                    */
/******************************************************/

#include <cstring>
#include <cstdio>
#include "angle.h"

double sqr(double x)
{
  return x*x;
}

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

string bintoangle(int angle,int unitp)
{
  double angmult,prec;
  string ret;
  char digit[8];
  int i,base,sign,dig;
  if (!compatible_units(unitp,ANGLE))
    throw badunits;
  base=unitp&0xf0;
  switch (base)
  {
    case 0:
      base=10;
      break;
    case 16:
      base=2;
      break;
    case 32:
      base=60;
      break;
    default:
      base=0;
  }
  switch (unitp&0xffffff00)
  {
    case DEGREE:
      angmult=bintodeg(angle);
      break;
    case GON:
      angmult=bintogon(angle);
      break;
    case RADIAN:
      angmult=bintorad(angle);
      break;
    default:
      throw badunits;
  }
  angmult=rint((prec=precision(unitp))*angmult);
  sign=1;
  if (angmult<0)
  {
    angmult=-angmult;
    sign=-1;
  }
  for (;base>10 && (int)prec%59>1;prec/=10)
  {
    dig=angmult-10*trunc(angmult/10);
    angmult=trunc(angmult/10);
    sprintf(digit,"%01d",dig);
    ret=digit+ret;
  }
  if (ret.length())
    ret="."+ret;
  for (;prec>1;prec/=base)
  {
    dig=angmult-base*trunc(angmult/base);
    angmult=trunc(angmult/base);
    sprintf(digit,(base>10)?"%02d":"%01d",dig);
    if (base>10)
      if (ret.substr(0,1)==".")
	ret+=(prec>60)?"″":"′";
      else
	strcat(digit,(prec>60)?"″":"′");
    ret=digit+ret;
  }
  if (base>10)
    ret="°"+ret;
  else
    ret="."+ret+"°";
  sprintf(digit,"%.0f",angmult);
  ret=digit+ret;
  return ret;
}
