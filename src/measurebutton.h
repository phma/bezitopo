/******************************************************/
/*                                                    */
/* measurebutton.h - buttons for measuring            */
/*                                                    */
/******************************************************/
/* Copyright 2017,2018 Pierre Abbat.
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
#ifndef MEASUREBUTTON_H
#define MEASUREBUTTON_H
#include <QtWidgets>
#include "measure.h"

class MeasureButton: public QAction
{
  Q_OBJECT
public:
  MeasureButton(QObject *parent=nullptr,int64_t lunit=0,int whichFoot=INTERNATIONAL);
  void activate(ActionEvent event);
public slots:
  void setMeasure(Measure newMeasure);
signals:
protected:
private:
  int64_t myUnit;
  int myFoot;
};
#endif
