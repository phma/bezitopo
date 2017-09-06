/******************************************************/
/*                                                    */
/* tinwindow.h - window for viewing TIN               */
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
#include <QMainWindow>
#include <QTimer>
#include <QtWidgets>
#include <QPixmap>
#include "document.h"

extern document doc;

class TinCanvas: public QWidget
{
  Q_OBJECT
public:
  TinCanvas(QWidget *parent=0);
signals:
public slots:
private:
  document doc;
  xy windowCenter,worldCenter;
  int scale;
  /* scale is the logarithm, in major thirds (see zoom), of the number of
   * windowSize lengths in a meter. It is thus usually negative.
   */
  int rotation; // rotation is stepped by compass points (DEG45/4)
  double windowSize; // length of a perpendicular from corner to diagonal
};

class TinWindow: public QMainWindow
{
  Q_OBJECT
public:
  TinWindow(QWidget *parent=0);
  ~TinWindow();
signals:
public slots:
private:
  TinCanvas *canvas;
};
