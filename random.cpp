/******************************************************/
/*                                                    */
/* random.cpp - random numbers                        */
/*                                                    */
/******************************************************/

#include <cstdio>
#include "random.h"

randm::randm()
{
  randfil=fopen("/dev/urandom","rb");
}

randm::~randm()
{
  fclose(randfil);
}

unsigned short randm::usrandom()
{
  unsigned short n;
  fread(&n,1,2,randfil);
  return n;
}

unsigned char randm::ucrandom()
{
  unsigned char n;
  fread(&n,1,1,randfil);
  return n;
}

randm rng;