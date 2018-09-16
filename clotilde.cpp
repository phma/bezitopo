/******************************************************/
/*                                                    */
/* clotilde.cpp - tables of approximations to spirals */
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
/* This program is named Clotilde for similarity to "clotoide", the Spanish
 * word for the Euler spiral.
 */
#include <iostream>
#include "manyarc.h"
#include "vball.h"
using namespace std;
vector<string> args;

void startHtml(spiralarc s,Measure ms)
{
  cout<<"<html><head><title>Approximation ";
  cout<<ms.formatMeasurementUnit(s.length(),LENGTH,0,0.522)<<"</title></head><body>\n";
}

void endHtml()
{
  cout<<"</body></html>\n";
}

void outSpiral(spiralarc s,Measure ms)
{
  double startCur=s.curvature(0),endCur=s.curvature(s.length());
  if (fabs(startCur)<1/EARTHRAD)
    startCur=0;
  if (fabs(endCur)<1/EARTHRAD)
    endCur=0;
  cout<<"<table border><tr><td><h1>Arc length: "<<ms.formatMeasurementUnit(s.length(),LENGTH)<<"</h1></td>\n";
  cout<<"<td><h1>Chord length: "<<ms.formatMeasurementUnit(s.chordlength(),LENGTH)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Start curvature: "<<formatCurvature(startCur,ms)<<"</h1></td>\n";
  cout<<"<td><h1>End curvature: "<<formatCurvature(endCur,ms)<<"</h1></td></tr>\n";
  cout<<"<tr><td><h1>Start radius: "<<ms.formatMeasurementUnit(1/startCur,LENGTH)<<"</h1></td>\n";
  cout<<"<td><h1>End radius: "<<ms.formatMeasurementUnit(1/endCur,LENGTH)<<"</h1></td></tr>\n";
  cout<<"<tr><td colspan=2><h1>Delta: "<<ms.formatMeasurementUnit(s.getdelta(),ANGLE_B)<<"</h1></td></tr></table>\n";
}

void outArc(arc oneArc,Measure ms)
{
  double relprec=abs(oneArc.getdelta());
  if (relprec==0)
    relprec=1;
  cout<<"<tr><td colspan=4>"<<ms.formatMeasurementUnit(oneArc.length(),LENGTH)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.chordlength(),LENGTH)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.getdelta(),ANGLE_B)<<"</td>";
  cout<<"<td colspan=4>"<<formatCurvature(oneArc.curvature(0),ms,oneArc.curvature(0)/relprec)<<"</td>";
  cout<<"<td colspan=4>"<<ms.formatMeasurementUnit(oneArc.radius(0),LENGTH,0,oneArc.radius(0)/relprec)<<"</td></tr>\n";
}

void outPoint(xy pnt,spiralarc s,Measure ms)
{
  int sb=s.startbearing(),eb=s.endbearing();
  xy sp=s.getstart(),ep=s.getend();
  cout<<"<tr><td colspan=5>"<<ms.formatMeasurementUnit((sp==pnt)?0:(dir(sp,pnt)-sb),ANGLE_B)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit(dist(sp,pnt),LENGTH)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit((pnt==ep)?0:(dir(pnt,ep)-eb),ANGLE_B)<<"</td>";
  cout<<"<td colspan=5>"<<ms.formatMeasurementUnit(dist(pnt,ep),LENGTH)<<"</td></tr>\n";
}

void outApprox(polyarc approx,spiralarc s,Measure ms)
{
  int i;
  arc oneArc;
  double err=maxError(approx,s);
  cout<<"<table border><tr><th colspan=20>"<<approx.size()<<" arcs, error "
    <<ms.formatMeasurementUnit(err,LENGTH,0,err/32)<<"</th></tr>\n";
  for (i=0;i<approx.size();i++)
  {
    oneArc=approx.getarc(i);
    outPoint(oneArc.getstart(),s,ms);
    outArc(oneArc,ms);
  }
  outPoint(oneArc.getend(),s,ms);
  cout<<"</table>\n";
}

/* Ways to specify the spiralarc to be approximated:
 * • Start radius, end radius, arc length
 * • Start curvature, end curvature, arc length
 * • Start radius, end radius, chord length
 * • Start curvature, end curvature, chord length
 * Curvature may be specified in diopters or degrees; if in degrees, the length
 * is assumed to be 100 unless otherwise specified.
 */
int main(int argc, char *argv[])
{
  int i;
  spiralarc s;
  spiralarc trans(xyz(0,0,0),0,0.003,xyz(500,0,0));
  polyarc approx;
  Measure ms;
  ms.setMetric();
  ms.setDefaultUnit(LENGTH,0.552);
  ms.setDefaultPrecision(LENGTH,2e-6);
  ms.setDefaultUnit(CURVATURE,0.001);
  ms.setDefaultPrecision(CURVATURE,2e-9);
  ms.setDefaultPrecision(ANGLE_B,1);
  ms.addUnit(ARCSECOND_B+DECIMAL+FIXLARGER);
  ms.setDefaultPrecision(ANGLE,bintorad(1));
  ms.addUnit(ARCSECOND+DECIMAL+FIXLARGER);
  for (i=1;i<argc;i++)
    args.push_back(argv[i]);
  startHtml(trans,ms);
  outSpiral(trans,ms);
  i=2;
  do
  {
    approx=manyArc(trans,i);
    outApprox(approx,trans,ms);
    i++;
  } while (maxError(approx,trans)>0.01);
  endHtml();
  return 0;
}
