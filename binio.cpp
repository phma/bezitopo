/******************************************************/
/*                                                    */
/* binio.cpp - binary input/output                    */
/*                                                    */
/******************************************************/
#include <cstring>
#include "binio.h"
#include "config.h"

void endianflip(void *addr,int n)
{
  int i;
  char *addr2;
  addr2=(char *)addr;
  for (i=0;i<n/2;i++)
  {
    addr2[i]^=addr2[n-1-i];
    addr2[n-1-i]^=addr2[i];
    addr2[i]^=addr2[n-1-i];
  }
}

void writegeint(std::ofstream &file,int i)
/* Numbers in Bezitopo's geoid files are in 65536ths of a meter and are less than 110 m
 * (7208960) in absolute value. They are encoded as follows:
 * gg xx xx where gg is 00-7f		00 gg xx xx
 * gg xx xx where gg is 81-ff		ff gg xx xx
 * 80 gg xx xx xx where gg is not 0	gg xx xx xx with an offset
 * 80 00				80 00 00 00, which means NaN
 */
{
  char buf[8];
  if (i>=(int)0xff810000 && i<=0x7fffff)
  {
    *(int *)buf=i;
#ifndef BIGENDIAN
    endianflip(buf,4);
#endif
    file.write(buf+1,3);
  }
  else if (i<(int)0x80010000)
  {
    buf[0]=0x80;
    buf[1]=0;
    file.write(buf,2);
  }
  else
  {
    if (i<(int)0xff810000)
      *(int *)buf=i-0xff810000;
    if (i>0x7fffff)
      *(int *)buf=i+0x800000;
#ifndef BIGENDIAN
    endianflip(buf,4);
#endif
    memmove(buf+1,buf,4);
    buf[0]=0x80;
    file.write(buf,5);
  }
}

int readgeint(std::ifstream &file)
{
}