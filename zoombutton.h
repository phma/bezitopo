/******************************************************/
/*                                                    */
/* zoombutton.h - buttons for zooming                 */
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
#ifndef ZOOMBUTTON_H
#define ZOOMBUTTON_H
#include <QtWidgets>

class ZoomButton: public QAction
{
  Q_OBJECT
public:
  ZoomButton(QObject *parent=nullptr,int steps=0,int angle=0);
  void activate(ActionEvent event);
public slots:
signals:
  void zoomSteps(int steps); // emitted on activate(triggered)
protected:
  bool event(QEvent *e);
private:
  int mySteps;
  int myAngle;
};
#endif
