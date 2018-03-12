/******************************************************/
/*                                                    */
/* transmer.cpp - series for transverse Mercator      */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <fftw3.h>
#include "polyline.h"
#include "projection.h"
#include "ellipsoid.h"
#include "ps.h"
#include "vball.h"
#include "manysum.h"

using namespace std;

vector<string> args;
map<int,fftw_plan> plans;
map<int,double *> inmem,outmem;

fftw_plan makePlan(int n)
// fftw_plan is a pointer to plan_s, so it can be checked as bool
{
  if (!plans[n])
  {
    inmem[n]=fftw_alloc_real(n);
    outmem[n]=fftw_alloc_real(n);
    plans[n]=fftw_plan_r2r_1d(n,inmem[n],outmem[n],FFTW_RODFT10,FFTW_ESTIMATE);
  }
  return plans[n];
}

void destroyPlans()
{
  map<int,fftw_plan>::iterator i;
  map<int,double *>::iterator j;
  for (i=plans.begin();i!=plans.end();i++)
    fftw_destroy_plan(i->second);
  for (j=inmem.begin();j!=inmem.end();j++)
    fftw_free(j->second);
  for (j=outmem.begin();j!=outmem.end();j++)
    fftw_free(j->second);
  plans.clear();
  inmem.clear();
  outmem.clear();
}

vector<double> fft(vector<double> input)
/* The output is calibrated so that the frequency-domain terms are independent
 * of the size of the input.
 */
{
  vector<double> output(input);
  int i,sz=input.size();
  fftw_plan plan=makePlan(sz);
  memcpy(inmem[sz],&input[0],sz*sizeof(double));
  fftw_execute(plan);
  for (i=0;i<sz;i++)
    output[i]=outmem[sz][i]/sz;
  return output;
}

polyspiral psApprox(ellipsoid *ell,int n)
/* Computes an n-piece approximation to a quadrant of meridian of the ellipsoid.
 * The ellipse is a quadratic algebraic curve, but its length is not in closed form.
 * The spiral's length is simply its parameter. Its position is not in closed form,
 * but I've already solved that problem with series. So I approximate the ellipse
 * with spiralarcs.
 * 
 * The purpose of this is to compute the function from length along the ellipsoid's
 * meridian to length along the sphere's meridian under the conformal map. This is
 * the same as the function from the ellipsoidal transverse Mercator map to the
 * spherical transverse Mercator map along the central meridian. The rest of the map
 * follows by conformality.
 */
{
  int i;
  polyspiral ret;
  xyz meridianPoint;
  vector<int> latSplit;
  ret.smooth();
  for (i=0;i<=n;i++)
  {
    latSplit.push_back(rint((double)DEG90*i/n));
    meridianPoint=ell->geoc(latSplit[i],0,0);
    ret.insert(xy(meridianPoint.getx(),meridianPoint.getz()));
  }
  ret.open();
  ret.setlengths();
  for (i=0;i<=n;i++)
    ret.setbear(i,latSplit[i]+DEG90);
  for (i=0;i<n;i++)
    ret.setspiral(i);
  ret.setlengths();
  return ret;
}

double compareLengths(polyspiral fewer,polyspiral more)
{
  int i,j,ratio;
  vector<double> diff;
  double sum;
  ratio=more.size()/fewer.size();
  for (i=0;i<fewer.size();i++)
  {
    for (sum=j=0;j<ratio;j++)
      sum+=more.getspiralarc(i*ratio+j).length();
    diff.push_back(sqr(sum-fewer.getspiralarc(i).length()));
  }
  return pairwisesum(diff);
}

array<double,3> compareTransforms(vector<double> fewer,vector<double> more)
/* ret[0] is the total square difference between fewer and the first half of more.
 * ret[1] is the total square of the last half of more.
 * ret[2] is the largest absolute value of the last half of more.
 * ret[1] and ret[2] are estimates of the noise floor.
 */
{
  vector<double> squares;
  array<double,3> ret;
  ret[2]=0;
  int i;
  for (i=0;i<fewer.size();i++)
    squares.push_back(sqr(fewer[i]-more[i]));
  ret[0]=pairwisesum(squares);
  squares.clear();
  for (;i<more.size();i++)
  {
    squares.push_back(sqr(more[i]));
    if (fabs(more[i])>ret[2])
      ret[2]=fabs(more[i]);
  }
  ret[1]=pairwisesum(squares);
  return ret;
}

vector<array<double,2> > projectForward(ellipsoid *ell,polyspiral apx,int n)
/* Projects n points (n is a power of 2) from the sphere to the ellipsoid,
 * returning a vector of lengths along the meridian. The vector has size n+1;
 * the last member is the North Pole, i.e. the total length of the meridian.
 * Sphere to ellipsoid is forward because that is used when projecting
 * from the ellipsoid to the plane.
 */
{
  int i;
  latlong llSphere,llEllipsoid;
  xyz meridianPoint;
  vector<array<double,2> > ret;
  array<double,2> totalLength,projPair;
  totalLength[0]=ell->sphere->geteqr()*M_PI/2;
  totalLength[1]=apx.length();
  for (i=0;i<n;i++)
  {
    llSphere=latlong(i*(DEG90/n)+DEG45/n,0);
    llEllipsoid=ell->inverseConformalLatitude(llSphere);
    meridianPoint=ell->geoc(llEllipsoid,0);
    projPair[0]=llSphere.lat*ell->sphere->geteqr();
    projPair[1]=apx.closest(xy(meridianPoint.getx(),meridianPoint.getz()));
    ret.push_back(projPair);
  }
  ret.push_back(totalLength);
  return ret;
}

vector<array<double,2> > projectBackward(ellipsoid *ell,polyspiral apx,int n)
/* Projects n points (n is a power of 2) from the ellipsoid to the sphere,
 * returning a vector of lengths along the meridian. The vector has size n+1;
 * the last member is the North Pole, i.e. the total length of the meridian.
 */
{
  int i;
  latlong llSphere,llEllipsoid;
  latlongelev lleEllipsoid;
  xyz meridianPoint;
  vector<array<double,2> > ret;
  array<double,2> totalLength,projPair;
  totalLength[1]=ell->sphere->geteqr()*M_PI/2;
  totalLength[0]=apx.length();
  for (i=0;i<n;i++)
  {
    projPair[0]=((i+0.5)/n)*totalLength[0];
    meridianPoint=apx.station(projPair[0]);
    lleEllipsoid=ell->geod(xyz(meridianPoint.getx(),0,meridianPoint.gety()));
    assert(fabs(lleEllipsoid.elev)<0.018/243);
    /* 18 mm is 1/2 angle ulp; 243 is the number of spiralarcs.
     */
    llEllipsoid=lleEllipsoid;
    llSphere=ell->conformalLatitude(llEllipsoid);
    projPair[1]=llSphere.lat*ell->sphere->geteqr();
    ret.push_back(projPair);
  }
  ret.push_back(totalLength);
  return ret;
}

vector<double> exeutheicity(vector<array<double,2> > proj)
/* The amount by which something deviates from a straight line, from Greek
 * εξ + ευθεια, by analogy with "eccentricity".
 * proj[i][0] should be equally spaced, except the last, which gives the range.
 */
{
  vector<double> ret;
  int i;
  for (i=0;i<proj.size()-1;i++)
    ret.push_back(proj[i][1]/proj.back()[1]-proj[i][0]/proj.back()[0]);
  return ret;
}

void doEllipsoid(ellipsoid &ell,PostScript &ps)
/* Compute approximations to the meridian of the ellipsoid.
 */
{
  int i,j,nseg;
  bool done=false;
  int goodForwardTerms,goodReverseTerms,forwardNoiseFloor,reverseNoiseFloor;
  vector<polyspiral> apx;
  vector<array<double,2> > forwardLengths,reverseLengths;
  vector<double> forwardTransform,reverseTransform,lastForwardTransform,lastReverseTransform;
  array<double,3> forwardDifference,reverseDifference;
  ps.startpage();
  ps.setscale(0,0,EARTHRAD,EARTHRAD,0);
  for (i=0,nseg=1;i<7;i++,nseg*=3)
    apx.push_back(psApprox(&ell,nseg));
  cout<<ell.getName()<<endl;
  for (i=0;i<apx.size()-1;i++)
    cout<<setw(2)<<i<<setw(12)<<compareLengths(apx[i],apx[i+1])<<
          setw(12)<<apx[i+1].length()-apx[i].length()<<endl;
  ps.spline(apx.back().approx3d(1e3));
  for (i=0,nseg=1;i<12 && !done;i++,nseg*=2)
  {
    forwardLengths=projectForward(&ell,apx[5],nseg);
    reverseLengths=projectBackward(&ell,apx[5],nseg);
    forwardTransform=fft(exeutheicity(forwardLengths));
    reverseTransform=fft(exeutheicity(reverseLengths));
    if (lastForwardTransform.size())
    {
      forwardDifference=compareTransforms(lastForwardTransform,forwardTransform);
      reverseDifference=compareTransforms(lastReverseTransform,reverseTransform);
      cout<<setw(2)<<i<<setw(14)<<forwardDifference[0]<<setw(12)<<forwardDifference[1]<<setw(12)<<forwardDifference[2];
      cout<<setw(14)<<reverseDifference[0]<<setw(12)<<reverseDifference[1]<<setw(12)<<reverseDifference[2]<<endl;
      done=forwardDifference[0]<2*forwardDifference[1] && reverseDifference[0]<2*reverseDifference[1];
      goodForwardTerms=goodReverseTerms=0;
      forwardNoiseFloor=reverseNoiseFloor=0;
      for (j=0;j<lastForwardTransform.size();j++)
      {
	if (fabs(forwardTransform[j]-lastForwardTransform[j])<
	    fabs(forwardTransform[j]+lastForwardTransform[j])/256 && goodForwardTerms>=j)
	  goodForwardTerms++;
	if (fabs(reverseTransform[j]-lastReverseTransform[j])<
	    fabs(reverseTransform[j]+lastReverseTransform[j])/256 && goodReverseTerms>=j)
	  goodReverseTerms++;
	if (fabs(forwardTransform[j])>2*forwardDifference[2])
	  forwardNoiseFloor=j+1;
	if (fabs(reverseTransform[j])>2*reverseDifference[2])
	  reverseNoiseFloor=j+1;
      }
      cout<<"Forward "<<goodForwardTerms<<" good, noise "<<forwardNoiseFloor<<"   ";
      cout<<"Reverse "<<goodReverseTerms<<" good, noise "<<reverseNoiseFloor<<endl;
      if (goodForwardTerms<forwardNoiseFloor-1 || goodReverseTerms<reverseNoiseFloor-1)
	done=false;
    }
    lastForwardTransform=forwardTransform;
    lastReverseTransform=reverseTransform;
  }
  for (i=0;i<0;i++)
    cout<<setw(2)<<i+1<<setw(12)<<forwardTransform[i]<<setw(12)<<reverseTransform[i]<<endl;
  ps.endpage();
}

void calibrate()
{
  int i,sz=32;
  vector<double> input,output;
  for (i=0;i<sz;i++)
    input.push_back(sin(DEG180/(2*sz)*(2*i+1)));
  output=fft(input);
  cout<<output[0]<<endl;
}

int main(int argc, char *argv[])
{
  int i;
  PostScript ps;
  for (i=1;i<argc;i++)
    args.push_back(argv[i]);
  ps.open("transmer.ps");
  ps.setpaper(papersizes["A4 portrait"],0);
  ps.prolog();
  for (i=0;i<countEllipsoids();i++)
    doEllipsoid(getEllipsoid(i),ps);
  ps.trailer();
  ps.close();
  calibrate();
  destroyPlans();
  return 0;
}
