/******************************************************/
/*                                                    */
/* except.cpp - exceptions                            */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#include <vector>
#include "except.h"

using namespace std;

char exceptionNames[N_EXCEPTIONS][24]=
{
  QT_TRANSLATE_NOOP("BeziExcept","noerror"),
  QT_TRANSLATE_NOOP("BeziExcept","notri"),
  QT_TRANSLATE_NOOP("BeziExcept","samepnts"),
  QT_TRANSLATE_NOOP("BeziExcept","flattri"),
  QT_TRANSLATE_NOOP("BeziExcept","badheader"),
  QT_TRANSLATE_NOOP("BeziExcept","baddata"),
  QT_TRANSLATE_NOOP("BeziExcept","matrixmismatch"),
  QT_TRANSLATE_NOOP("BeziExcept","singularmatrix"),
  QT_TRANSLATE_NOOP("BeziExcept","unsetgeoid"),
  QT_TRANSLATE_NOOP("BeziExcept","unsetsource"),
  QT_TRANSLATE_NOOP("BeziExcept","badunits"),
  QT_TRANSLATE_NOOP("BeziExcept","badnumber"),
  QT_TRANSLATE_NOOP("BeziExcept","badbreaklineend"),
  QT_TRANSLATE_NOOP("BeziExcept","breaklinescross"),
  QT_TRANSLATE_NOOP("BeziExcept","badbreaklineformat"),
  QT_TRANSLATE_NOOP("BeziExcept","fileerror")
};
vector<QString> translatedExceptions;

BeziExcept::BeziExcept(const BeziExcept &a)
{
  exceptNumber=a.exceptNumber;
  pointNumber[0]=a.pointNumber[0];
  pointNumber[1]=a.pointNumber[1];
}

BeziExcept::BeziExcept(int num)
{
  exceptNumber=num;
  pointNumber[0]=pointNumber[1]=0;
}

const QString &translateException(int n)
{
  return translatedExceptions[n];
}

const QString &BeziExcept::message()
{
  return translatedExceptions[exceptNumber];
}

void initTranslateException()
{
  int i;
  translatedExceptions.clear();
  for (i=0;i<N_EXCEPTIONS;i++)
    translatedExceptions.push_back(BeziExcept::tr(exceptionNames[i]));
}
