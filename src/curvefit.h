/******************************************************/
/*                                                    */
/* curvefit.h - fit polyarc/alignment to points       */
/*                                                    */
/******************************************************/
/* Copyright 2022 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <set>
#include <deque>
#include "circle.h"
#include "polyline.h"
#include "manyarc.h"

struct FitRec
{
  double startOff;
  std::vector<xy> endpoints;
  double endOff;
  double startCur; // used only when fitting a polyspiral
  int startBear;
  double shortDist(Circle startLine,Circle endLine) const;
  bool isnan() const;
  void breakArcs(std::set<int> which,polyarc apx);
};

double diff(const FitRec &a,const FitRec &b,Circle startLine,Circle endLine);

/* Makes the initial curve, given at least two lines. pieces is the number of
 * arcs between lines, e.g. if lines has 5 lines and pieces is 3, you get
 * 12 arcs. As you walk along the path from first to last, the lines are
 * pointing to the right.
 */
FitRec initialCurve(std::deque<Circle> lines,int pieces);

void stepDir();
std::vector<double> curvefitResiduals(polyarc q,std::vector<xy> points);
double curvefitSquareError(polyarc q,std::vector<xy> points);
double curvefitMaxError(polyarc q,std::vector<xy> points);
std::set<int> breakWhich(polyarc q,std::vector<xy> points);
polyarc arcFitApprox(Circle startLine,FitRec fr,Circle endLine);
FitRec adjust1step(std::vector<xy> points,Circle startLine,FitRec fr,Circle endLine,bool twoD);
FitRec adjustArcs(std::vector<xy> points,Circle startLine,FitRec fr,Circle endLine);

/* Fits a polyarc to the points. The initial polyarc is formed by fitting
 * a spiralarc to the midpoints of startLine and endLine perpendicular to both,
 * then approximating it with two arcs. If there are hints, it starts with
 * one more spiralarc than hints. The returned polyarc will start on startLine
 * and end on endLine, but the endpoints on the hints will probably be moved
 * off them.
 */
polyarc fitPolyarc(Circle startLine,std::vector<xy> points,Circle endLine,double toler,std::deque<Circle> hints=std::deque<Circle>(),int pieces=2);
void curvefit_i(std::string args);
