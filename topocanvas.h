/******************************************************/
/*                                                    */
/* topocanvas.h - canvas for drawing topography       */
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
#ifndef TOPOCANVAS_H
#define TOPOCANVAS_H
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

// goals
#define DONE 0
#define MAKE_TIN 1
#define ROUGH_CONTOURS 2
#define SMOOTH_CONTOURS 3

class TopoCanvas: public QWidget
{
  Q_OBJECT
public:
  TopoCanvas(QWidget *parent=0);
  void setBrush(const QBrush &qbrush);
  QPointF worldToWindow(xy pnt);
  xy windowToWorld(QPointF pnt);
  double pixelScale();
  double viewableRadius();
  void repaintSeldom();
  bool mouseCheckImported();
  bool makeTinCheckEdited();
  document *getDoc();
signals:
  void measureChanged(Measure newMeasure);
public slots:
  void sizeToFit();
  void zoom(int steps);
  void zoomm10();
  void zoomm3();
  void zoomm1();
  void zoomp1();
  void zoomp3();
  void zoomp10();
  void setShowDelaunay(bool showd);
  void setAllowFlip(bool allow);
  void setTipXyz(bool tipxyz);
  void rotatecw();
  void rotateccw();
  void setButtonBits(int bits);
  void setMeter();
  void setFoot();
  void setInternationalFoot();
  void setUSFoot();
  void setIndianFoot();
  void updateEdge(edge *e);
  void updateEdgeNeighbors(edge *e);
  void open();
  void saveAs();
  void save();
  void testPatternAster();
  void importPnezd();
  void importCriteria();
  void importBreaklines();
  void exportBreaklines();
  void makeTin();
  void tryStartPoint();
  void flipPass();
  void redoSurface();
  void findCriticalPoints();
  void makeTinFinish();
  void tinCancel();
  void selectContourInterval();
  void roughContours();
  void rough1Contour();
  void roughContoursFinish();
  void contoursCancel();
  void smoothContours();
  void smooth1Contour();
  void smoothContoursFinish();
  void loadGeoid();
  void dump();
protected:
  void setSize();
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
  document doc;
  int plnum;
  std::string docFileName;
  QPen normalEdgePen,breakEdgePen,flipEdgePen;
  QPen circlePen[3],contourPen[3][20];
  unsigned short contourColor[20];
  short contourLineType[3];
  unsigned short contourThickness[3];
  QBrush brush;
  QErrorMessage *errorMessage;
  QFileDialog *fileDialog;
  QProgressDialog *progressDialog;
  QTimer *timer;
  ContourIntervalDialog *ciDialog;
  double conterval;
  xy windowCenter,worldCenter,dragStart;
  RenderCache contourCache;
  int scale;
  /* scale is the logarithm, in major thirds (see zoom), of the number of
   * windowSize lengths in a meter. It is thus usually negative.
   */
  int rotation; // rotation is stepped by compass points (DEG45/4)
  double windowSize; // length of a perpendicular from corner to diagonal
  double windowDiag;
  bool mouseClicked,mouseDoubleClicked;
  int tinerror,startPointTries,passCount,triCount;
  PostScript dummyPs;
  xy startPoint;
  int goal;
  int progInx; // used in progress bar loops
  int elevHi,elevLo; // in contour interval unit
  std::array<double,2> tinlohi;
  bool pointsValid; // If false, to make TIN, must first copy points.
  bool tinValid; // If false, to set gradient, must first make TIN.
  bool surfaceValid; // If false, to do rough contours, must first set gradient.
  bool roughContoursValid; // If false, to do smooth contours, must first do rough contours.
  bool smoothContoursValid;
  bool trianglesAreCurvy,trianglesShouldBeCurvy;
  bool contoursAreCurvy,contoursShouldBeCurvy;
  bool showDelaunay; // If true, edges change color and become dashed if not Delaunay.
  bool allowFlip; // If true, clicking on an edge toggles breakline or flips it.
  bool tipXyz;
  /* If true, tooltip shows xyz while cursor is in TIN.
   * If false, shows point numbers.
   */
  QTime lastPaintTime;
  int lastPaintDuration;
};
#endif
