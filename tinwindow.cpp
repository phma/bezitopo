/******************************************************/
/*                                                    */
/* tinwindow.cpp - window for viewing TIN             */
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
#include "tinwindow.h"

document doc; // FIXME needs fixing in ps.cpp

TinCanvas::TinCanvas(QWidget *parent):QWidget(parent)
{
}

TinWindow::TinWindow(QWidget *parent):QMainWindow(parent)
{
  resize(707,500);
  setWindowTitle(QApplication::translate("main", "ViewTIN"));
  show();
  canvas=new TinCanvas(this);
  setCentralWidget(canvas);
  canvas->show();
}

TinWindow::~TinWindow()
{
  delete canvas;
}
