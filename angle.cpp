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

int foldangle(int angle)
{
  if (((unsigned)angle>>30)%3)
    angle^=0x80000000;
  return angle;
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
  string ret,unitsign;
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
      unitsign="°";
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
    ret=unitsign+ret;
  else
    ret="."+ret+unitsign;
  sprintf(digit,"%.0f",angmult);
  ret=digit+ret;
  if (sign<0)
    ret="-"+ret;
  return ret;
}

int parseangle(string angstr,int unitp)
{
  double angmult,prec;
  int i,ulen,angle;
  bool point,six;
  string uchar;
  for (point=six=i=0,prec=1;i<angstr.length();i++)
    switch (angstr[i]&0xc0)
    {
      case 0: // digits, point, and hyphen
	if (angstr[i]=='-' || angstr[i]=='\'' || angstr[i]=='"')
	{
	  point=six=true;
	  unitp=DEGREE;
	}
	if (angstr[i]=='.')
	  point=true;
	if (isdigit(angstr[i]))
	{
	  angmult=angmult*(six?6:10)+angstr[i]-'0';
	  if (point)
	    prec*=six?6:10;
	  six=false;
	}
	break;
      case 0x40: // letters
	if (tolower(angstr[i])=='g')
	  unitp=GON;
	break;
      case 0x80: // subsequent bytes of UTF-8: ignore
	break;
      case 0xc0: // first byte of UTF-8: check for degree sign
	ulen=angstr[i]&0xff;
	ulen=(ulen>=0x80)+(ulen>=0xc0)+(ulen>=0xe0)+(ulen>=0xf0)+(ulen>=0xf8)+(ulen>=0xfc);
	uchar=angstr.substr(i,ulen);
	if (uchar=="°" || uchar=="′" || uchar=="″")
	{
	  point=six=true;
	  unitp=DEGREE;
	}
	break;
    }
  switch (unitp&0xffffff00)
  {
    case DEGREE:
      angle=degtobin(angmult/prec);
      break;
    case GON:
      angle=gontobin(angmult/prec);
      break;
    case RADIAN:
      angle=radtobin(angmult/prec);
      break;
    default:
      throw badunits;
  }
  return angle;
}

int parseazimuth(string angstr,int unitp)
{
  return 0x20000000-parseangle(angstr,unitp);
}

int parsebearing(string angstr,int unitp)
{
  int ns,ew,quadrant,angle;
  ns=tolower(angstr[0]);
  angstr.erase(0,1);
  ew=tolower(angstr[angstr.length()-1]);
  angstr.erase(angstr.length()-1,1);
  if (ns=='n' && ew=='e')
    quadrant=1;
  if (ns=='s' && ew=='e')
    quadrant=2;
  if (ns=='s' && ew=='w')
    quadrant=3;
  if (ns=='n' && ew=='w')
    quadrant=4;
  angle=parseazimuth(angstr,unitp);
  switch (quadrant)
  {
    case 2:
      angle=-angle;
      break;
    case 3:
      angle-=0x40000000;
      break;
    case 4:
      angle=0x40000000-angle;
      break;
  }
  return angle;
}

