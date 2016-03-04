/******************************************************/
/*                                                    */
/* histogram.cpp - adaptive histogram                 */
/*                                                    */
/******************************************************/
#include <cstring>
#include "angle.h"
#include "histogram.h"
using namespace std;

histogram::histogram()
{
  bin.push_back(0);
  bin.push_back(1);
  count.push_back(0);
  total=0;
}

histogram::histogram(double least,double most)
{
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  total=0;
}

void histogram::clear()
{
  double least,most;
  least=bin[0];
  most=bin[count.size()];
  bin.clear();
  count.clear();
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  total=0;
}

void histogram::clear(double least,double most)
{
  bin.clear();
  count.clear();
  bin.push_back(least);
  bin.push_back(most);
  count.push_back(0);
  total=0;
}

void histogram::clearcount()
{
  int i;
  for (i=0;i<count.size();i++)
    count[i]=0;
  total=0;
}

void histogram::split(int n)
{
  bin.push_back(0);
  count.push_back(0);
  memmove(&bin[n+1],&bin[n],sizeof(double)*(bin.size()-n-1));
  memmove(&count[n+1],&count[n],sizeof(double)*(count.size()-n-1));
  count[n+1]=count[n]/2;
  count[n]-=count[n+1];
  bin[n+1]=(bin[n]+bin[n+2])/2;
}

int histogram::find(double val)
{
  int lower=0,upper=count.size();
  if (val<bin[lower])
    return -1;
  else if (val>=bin[upper])
    return upper+1;
  else while (upper-lower>1)
  {
    if (val<bin[(upper+lower)/2])
      upper=(upper+lower)/2;
    else
      lower=(upper+lower)/2;
  }
  return lower;
}

histogram& histogram::operator<<(double val)
{
  int theBin;
  double newlimit;
  theBin=find(val);
  if (theBin>count.size())
  {
    newlimit=val+(bin[count.size()]-bin[count.size()-1]);
    bin.push_back(newlimit);
    count.push_back(0);
  }
  if (theBin<0)
  {
    newlimit=val+(bin[0]-bin[1]);
    bin.push_back(0);
    count.push_back(0);
    memmove(&bin[1],&bin[0],sizeof(double)*(bin.size()-1));
    memmove(&count[1],&count[0],sizeof(double)*(count.size()-1));
    bin[0]=newlimit;
    count[0]=0;
    theBin=0;
  }
  count[theBin]++;
  total++;
  if ((count[theBin]&1)==0 && sqr(count[theBin])>total)
    split(theBin);
  return *this;
}

unsigned histogram::nbars()
{
  return count.size();
}

histobar histogram::getbar(unsigned n)
{
  histobar ret;
  ret.start=bin[n];
  ret.end=bin[n+1];
  ret.count=count[n];
  return ret;
}
