/******************************************************/
/*                                                    */
/* viewtin.cpp - view triangulated irregular network  */
/*                                                    */
/******************************************************/
/* Copyright 2017-2018 Pierre Abbat.
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
#include <QtGui>
#include <QtWidgets>
#include <QTranslator>
#include "tinwindow.h"
#include "except.h"
#include "globals.h"

using namespace std;
ProjectionList allProjections;

void readAllProjections()
{
  ifstream pfile(string(SHARE_DIR)+"/projections.txt");
  allProjections.readFile(pfile);
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QTranslator translator,qtTranslator;
  if (qtTranslator.load(QLocale(),QLatin1String("qt"),QLatin1String("_"),
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&qtTranslator);
  if (translator.load(QLocale(),QLatin1String("bezitopo"),
                      QLatin1String("_"),QLatin1String(".")))
  {
    //cout<<"Translations found in current directory"<<endl;
    app.installTranslator(&translator);
  }
  else if (translator.load(QLocale(),QLatin1String("bezitopo"),
                      QLatin1String("_"),QLatin1String(SHARE_DIR)))
  {
    //cout<<"Translations found in share directory"<<endl;
    app.installTranslator(&translator);
  }
  initTranslateException();
  readTmCoefficients();
  readAllProjections();
  TinWindow window;
  window.show();
  return app.exec();
}
