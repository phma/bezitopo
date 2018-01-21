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
#include "polyline.h"
#include "projection.h"
#include "ellipsoid.h"
#include "ps.h"
#include "vball.h"
#include "manysum.h"

using namespace std;

vector<string> args;

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

void doEllipsoid(ellipsoid &ell,PostScript &ps)
/* Compute approximations to the meridian of the ellipsoid.
 */
{
  int i,nseg;
  vector<polyspiral> apx;
  ps.startpage();
  ps.setscale(0,0,EARTHRAD,EARTHRAD,0);
  for (i=0,nseg=1;i<9;i++,nseg*=3)
    apx.push_back(psApprox(&ell,nseg));
  cout<<ell.getName()<<endl;
  for (i=0;i<apx.size()-1;i++)
    cout<<setw(2)<<i<<setw(12)<<compareLengths(apx[i],apx[i+1])<<
          setw(12)<<apx[i+1].length()-apx[i].length()<<endl;
  ps.spline(apx.back().approx3d(1e3));
  ps.endpage();
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
  return 0;
}
