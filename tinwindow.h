/******************************************************/
/*                                                    */
/* tinwindow.h - window for viewing TIN               */
/*                                                    */
/******************************************************/
/* Copyright 2017-2019 Pierre Abbat.
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
#include <QMainWindow>
#include <QTimer>
#include <QtWidgets>
#include <QPixmap>
#include <array>
#include "document.h"
#include "zoombutton.h"
#include "measurebutton.h"
#include "cidialog.h"
#include "factordialog.h"
#include "rendercache.h"
#include "topocanvas.h"

// goals
#define DONE 0
#define MAKE_TIN 1
#define ROUGH_CONTOURS 2
#define SMOOTH_CONTOURS 3

class TinWindow: public QMainWindow
{
  Q_OBJECT
public:
  TinWindow(QWidget *parent=0);
  ~TinWindow();
  void makeActions();
  void unmakeActions();
signals:
  void zoomCanvas(int steps);
  void buttonBitsChanged(int bits);
public slots:
  void prepareZoomSteps(int steps);
  void zoomSteps(bool checked);
  void changeButtonBits();
  void gridToLatlong();
  void latlongToGrid();
  void aboutProgram();
  void aboutQt();
private:
  TopoCanvas *canvas;
  QToolBar *toolbar;
  QMenu *fileMenu,*editMenu,*viewMenu,*unitsMenu,*contourMenu,*coordMenu,*helpMenu;
  int preZoomStep;
  std::vector<ZoomButton *> zoomButtons;
  std::vector<MeasureButton *> measureButtons;
  LatlongFactorDialog *llDialog;
  GridFactorDialog *grDialog;
  QAction *sizeToFitAction;
  QAction *openAction,*saveAction,*saveAsAction;
  QAction *asterAction,*importPnezdAction,*importCriteriaAction;
  QAction *makeTinAction,*selectContourIntervalAction;
  QAction *roughContoursAction,*smoothContoursAction;
  QAction *importBreaklinesAction,*exportBreaklinesAction;
  QAction *aboutProgramAction,*aboutQtAction,*dumpAction;
  QAction *curvyTriangleAction,*curvyContourAction;
  QAction *loadGeoidAction,*gridToLatlongAction,*latlongToGridAction;
};
