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
#include <iostream>
#include <cmath>
#include "except.h"
#include "tinwindow.h"
#include "zoom.h"
#include "test.h"
#include "ldecimal.h"

using namespace std;

TinCanvas::TinCanvas(QWidget *parent):QWidget(parent)
{
  int i;
  doc.ms.setFoot(INTERNATIONAL);
  doc.ms.setMetric();
  doc.ms.setDefaultUnit(LENGTH,0.552); // geometric mean of meter and foot
  doc.ms.setDefaultPrecision(LENGTH,1.746e-3); // g.m. of 1 mm and 0.01 ft
  doc.ms.setDefaultUnit(AREA,0.3048); // for acre/hectare, 6361.5
  doc.ms.setDefaultPrecision(AREA,0.1);
  setAutoFillBackground(true);
  setMouseTracking(true);
  setBackgroundRole(QPalette::Base);
  normalEdgePen=QPen(Qt::black);
  breakEdgePen=QPen(Qt::blue,2); // TODO These colors should be user-selectable.
  flipEdgePen=QPen(Qt::blue,2,Qt::DashLine);
  circlePen[0]=QPen(Qt::red);
  circlePen[1]=QPen(Qt::darkGreen);
  circlePen[2]=QPen(Qt::blue);
  errorMessage=new QErrorMessage(this);
  fileDialog=new QFileDialog(this);
  progressDialog=new QProgressDialog(this);
  progressDialog->reset();
  ciDialog=new ContourIntervalDialog(this);
  timer=new QTimer(this);
  plnum=-1;
  goal=DONE;
  rotation=0;
  //for (i=0;i<doc.pl[1].edges.size();i++)
    //doc.pl[1].edges[i].dump(&doc.pl[1]);
  show();
  sizeToFit();
}

QPointF TinCanvas::worldToWindow(xy pnt)
{
  pnt.roscat(worldCenter,rotation,zoomratio(scale)*windowSize,windowCenter);
  QPointF ret(pnt.getx(),height()-pnt.gety());
  return ret;
}

xy TinCanvas::windowToWorld(QPointF pnt)
{
  xy ret(pnt.x(),height()-pnt.y());
  ret.roscat(windowCenter,-rotation,zoomratio(-scale)/windowSize,worldCenter);
  return ret;
}

double TinCanvas::pixelScale()
{
  return zoomratio(-scale)/windowSize;
}

void TinCanvas::setBrush(const QBrush &qbrush)
{
  brush=qbrush;
}

void TinCanvas::sizeToFit()
{
  double top,left,bottom,right;
  int vscale,hscale;
  if (plnum<0)
  {
    top=1;
    left=-1;
    bottom=-1;
    right=1;
  }
  else
  {
    top=-doc.pl[plnum].dirbound(DEG270-rotation);
    left=doc.pl[plnum].dirbound(-rotation);
    bottom=doc.pl[plnum].dirbound(DEG90-rotation);
    right=-doc.pl[plnum].dirbound(DEG180-rotation);
    if (top<=bottom)
    {
      top=1;
      bottom=-1;
    }
    if (left>=right)
    {
      left=-1;
      right=1;
    }
  }
  worldCenter=xy((left+right)/2,(top+bottom)/2);
  worldCenter.roscat(xy(0,0),-rotation,1,xy(0,0));
  if (windowSize)
  {
    vscale=largestFit(height()/windowSize/(top-bottom));
    hscale=largestFit(width()/windowSize/(right-left));
  }
  else
  { // If the widget size is not known yet, assume it's square.
    vscale=largestFit(M_SQRT2/(top-bottom));
    hscale=largestFit(M_SQRT2/(right-left));
  }
  scale=(hscale<vscale)?hscale:vscale;
  update();
}

void TinCanvas::zoom(int steps)
{
  scale+=steps;
  update();
}

void TinCanvas::zoomm10()
{
  zoom(-10);
}

void TinCanvas::zoomm3()
{
  zoom(-3);
}

void TinCanvas::zoomm1()
{
  zoom(-1);
}

void TinCanvas::zoomp1()
{
  zoom(1);
}

void TinCanvas::zoomp3()
{
  zoom(3);
}

void TinCanvas::zoomp10()
{
  zoom(10);
}

void TinCanvas::rotatecw()
{
  rotation-=DEG45/4;
  update();
}

void TinCanvas::rotateccw()
{
  rotation+=DEG45/4;
  update();
}

void TinCanvas::setMeter()
{
  doc.ms.setMetric();
  measureChanged(doc.ms);
}

void TinCanvas::setFoot()
{
  doc.ms.setCustomary();
  measureChanged(doc.ms);
}

void TinCanvas::setInternationalFoot()
{
  doc.ms.setFoot(INTERNATIONAL);
  measureChanged(doc.ms);
}

void TinCanvas::setUSFoot()
{
  doc.ms.setFoot(USSURVEY);
  measureChanged(doc.ms);
}

void TinCanvas::setIndianFoot()
{
  doc.ms.setFoot(INSURVEY);
  measureChanged(doc.ms);
}

void TinCanvas::updateEdge(edge *e)
{
  QMarginsF marge(1,1,1,1);
  QPointF aWindow=worldToWindow(*e->a);
  QPointF bWindow=worldToWindow(*e->b);
  QRectF rect(aWindow,bWindow);
  rect+=marge;
  update(rect.toAlignedRect());
}

void TinCanvas::updateEdgeNeighbors(edge *e)
{
  QMarginsF marge(1,1,1,1);
  QPointF aWindow=worldToWindow(*e->a);
  QPointF bWindow=worldToWindow(*e->b);
  QPointF cWindow=worldToWindow(*e->nexta->otherend(e->a));
  QPointF dWindow=worldToWindow(*e->nextb->otherend(e->b));
  QRectF rect1(aWindow,bWindow),rect2(cWindow,dWindow);
  QRectF rect=rect1.united(rect2);
  rect+=marge;
  update(rect.toAlignedRect());
}

void TinCanvas::testPatternAster()
{
  doc.makepointlist(1);
  plnum=1;
  aster(doc,100);
  sizeToFit();
}

void TinCanvas::importPnezd()
{
  int dialogResult;
  QStringList files;
  string fileName;
  fileDialog->setWindowTitle(tr("Open PNEZD File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    doc.makepointlist(0);
    doc.pl[0].clear();
    doc.readpnezd(fileName);
    plnum=0;
    sizeToFit();
    surfaceValid=false;
    roughContoursValid=false;
  }
}

void TinCanvas::importCriteria()
{
  int dialogResult;
  QStringList files;
  string fileName;
  fileDialog->setWindowTitle(tr("Open Criteria File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    doc.makepointlist(1);
    doc.pl[1].readCriteria(fileName);
    surfaceValid=false;
    roughContoursValid=false;
  }
}

void TinCanvas::makeTin()
{
  doc.makepointlist(1);
  if (doc.pl[1].size()==0 && doc.pl[0].size()>0)
  {
    if (doc.pl[1].crit.size()==0)
    {
      doc.pl[1].crit.resize(1);
      doc.pl[1].crit[0].istopo=true;
    }
    doc.copytopopoints(1,0);
  }
  plnum=1;
  tinerror=startPointTries=passCount=0;
  if (doc.pl[plnum].size()<3)
    tinerror=notri;
  else
    startPoint=doc.pl[1].points.begin()->second;
  progressDialog->setRange(0,100);
  progressDialog->setWindowModality(Qt::WindowModal);
  progressDialog->setWindowTitle(tr("Making TIN"));
  progressDialog->setLabelText(tr("Sweeping convex hull..."));
  progressDialog->show();
  connect(timer,SIGNAL(timeout()),this,SLOT(tryStartPoint()));
  timer->start(0);
}

void TinCanvas::tryStartPoint()
{
  if (startPointTries>=100)
    tinerror=flattri;
  if (tinerror)
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(tryStartPoint()));
    connect(timer,SIGNAL(timeout()),this,SLOT(flipPass()));
  }
  else
  {
    try
    {
      if (doc.pl[plnum].tryStartPoint(dummyPs,startPoint))
        progressDialog->setValue(++startPointTries);
      else
      {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(tryStartPoint()));
        connect(timer,SIGNAL(timeout()),this,SLOT(flipPass()));
        progressDialog->setRange(0,doc.pl[plnum].edges.size());
        progressDialog->setLabelText(tr("Flipping edges..."));
      }
    }
    catch (int e)
    {
      tinerror=e;
    }
  }
}

void TinCanvas::flipPass()
{
  int nFlip,nGoodEdges,i;
  bool tooLong=false;
  if (passCount*3>doc.pl[plnum].size())
    tooLong=true; // tooLong is not an error. The TIN should be valid; it's just
  if (tinerror || tooLong) // repeatedly flipping edges in a circle.
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(flipPass()));
    connect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
  }
  else
  {
    try
    {
      nFlip=doc.pl[plnum].flipPass(dummyPs,false);
      nGoodEdges=doc.pl[plnum].edges.size()-nFlip;
      progressDialog->setValue(nGoodEdges);
      ++passCount;
      if (!nFlip)
      {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(flipPass()));
        connect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
      }
    }
    catch (int e)
    {
      tinerror=e;
    }
  }
}

void TinCanvas::redoSurface()
/* This can be done as part of making the TIN, or when making the contours it
 * can find that this needs to be done first.
 */
{
  doc.pl[plnum].makegrad(0.15);
  doc.pl[plnum].maketriangles();
  doc.pl[plnum].setgradient();
  doc.pl[plnum].makeqindex();           // These five are all fast. It's finding the
  doc.pl[plnum].findedgecriticalpts();  // critical points of a triangle that's slow.
  progressDialog->setRange(0,doc.pl[plnum].triangles.size());
  progressDialog->setLabelText(tr("Finding critical points..."));
  triCount=0;
  disconnect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
  connect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
}

void TinCanvas::findCriticalPoints()
{
  if (tinerror)
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
    connect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
  }
  else
  {
    try
    {
      doc.pl[plnum].triangles[triCount++].findcriticalpts();
      progressDialog->setValue(triCount);
      if (triCount==doc.pl[plnum].triangles.size())
      {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
        connect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
      }
    }
    catch (int e)
    {
      tinerror=e;
    }
  }
}

void TinCanvas::makeTinFinish()
{
  if (tinerror)
  { // TODO: translate the thrown error into something intelligible
    QString msg=tr("Can't make TIN. Error: ")+QString::fromStdString(to_string(tinerror));
    doc.pl[plnum].clearTin();
    errorMessage->showMessage(msg);
  }
  else
  {
    doc.pl[plnum].addperimeter();
  }
  update();
  disconnect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
  timer->stop();
  progressDialog->reset();
  roughContoursValid=false;
  surfaceValid=true;
}

void TinCanvas::selectContourInterval()
{
  if (plnum>=0 && plnum<doc.pl.size())
    ciDialog->set(&doc.pl[plnum].contourInterval,doc.ms);
  else
    ciDialog->set(nullptr,doc.ms);
  ciDialog->exec();
}

void TinCanvas::roughContours()
{
  double conterval=doc.pl[plnum].contourInterval.fineInterval();
  if (goal==DONE)
  {
    goal=ROUGH_CONTOURS;
    timer->start(0);
    progressDialog->show();
  }
  tinlohi=doc.pl[plnum].lohi();
  elevLo=floor(tinlohi[0]/conterval);
  elevHi=ceil(tinlohi[1]/conterval);
  progInx=elevLo;
  progressDialog->setRange(elevLo,elevHi);
  progressDialog->setValue(progInx);
  progressDialog->setLabelText(tr("Drawing rough contours..."));
  disconnect(timer,SIGNAL(timeout()),this,SLOT(roughContours()));
  if (surfaceValid)
    connect(timer,SIGNAL(timeout()),this,SLOT(rough1Contour()));
  else
    connect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
}

void TinCanvas::rough1Contour()
{
  double conterval=doc.pl[plnum].contourInterval.fineInterval();
  rough1contour(doc.pl[plnum],progInx*conterval);
  if (++progInx>elevHi)
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(rough1Contour()));
    connect(timer,SIGNAL(timeout()),this,SLOT(roughContoursFinish()));
  }
  else
    progressDialog->setValue(progInx);
}

void TinCanvas::roughContoursFinish()
{
  switch (goal)
  {
    case ROUGH_CONTOURS:
      goal=DONE;
      progressDialog->reset();
      timer->stop();
      break;
  }
  disconnect(timer,SIGNAL(timeout()),this,SLOT(roughContoursFinish()));
  roughContoursValid=true;
  update();
}

void TinCanvas::paintEvent(QPaintEvent *event)
{
  int i;
  double r;
  ptlist::iterator j;
  QPainter painter(this);
  segment seg;
  painter.setBrush(brush);
  painter.setRenderHint(QPainter::Antialiasing,true);
  if (plnum<doc.pl.size() && plnum>=0)
    if (doc.pl[plnum].triangles.size())
      for (i=0;plnum>=0 && i<doc.pl[plnum].edges.size();i++)
      {
        seg=doc.pl[plnum].edges[i].getsegment();
        if (doc.pl[plnum].edges[i].delaunay())
          if (doc.pl[plnum].edges[i].broken&1)
            painter.setPen(breakEdgePen);
          else
            painter.setPen(normalEdgePen);
        else
          painter.setPen(flipEdgePen);
        painter.drawLine(worldToWindow(seg.getstart()),worldToWindow(seg.getend()));
      }
    else
      for (j=doc.pl[plnum].points.begin();j!=doc.pl[plnum].points.end();++j)
        for (i=0;i<3;i++)
        {
          painter.setPen(circlePen[i]);
          r=(i+1)*5+sin((double)j->first*(1<<2*i));
          // The radius variation is so that, if two points coincide, it's obvious.
          painter.drawEllipse(worldToWindow(j->second),r,r);
        }
  else
    ; // nothing to paint, since plnum is not the index of a pointlist
}

void TinCanvas::setSize()
{
  windowCenter=xy(width(),height())/2.;
  windowSize=1/sqrt(1/sqr(width())+1/sqr(height()));
}

void TinCanvas::resizeEvent(QResizeEvent *event)
{
  setSize();
  QWidget::resizeEvent(event);
}

void TinCanvas::mousePressEvent(QMouseEvent *event)
{
  xy eventLoc=windowToWorld(event->pos());
  if (event->button()==Qt::LeftButton)
    dragStart=eventLoc;
  //cout<<"mousePress "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseClicked=true;
}

void TinCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
  xy eventLoc=windowToWorld(event->pos());
  //cout<<"mouseDoubleClick "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseDoubleClicked=true;
}

void TinCanvas::mouseMoveEvent(QMouseEvent *event)
{
  xy eventLoc=windowToWorld(event->pos());
  triangleHit hitRec;
  triangle *tri;
  string tipString;
  if (event->buttons()&Qt::LeftButton)
  {
    worldCenter+=dragStart-eventLoc;
    update(); // No need to update dragStart, since it's dragged.
  }
  else if (plnum<doc.pl.size() && plnum>=0)
  {
    tri=doc.pl[plnum].findt(eventLoc,false);
    if (tri)
    {
      hitRec=tri->hitTest(eventLoc);
      tipString=doc.pl[plnum].hitTestString(hitRec);
    }
    else
    {
      tipString=doc.pl[plnum].hitTestPointString(eventLoc,pixelScale()*15);
      //if (tipString=="") // Uncomment these two lines to see the locale bug.
        //tipString=ldecimal(eventLoc.east())+','+ldecimal(eventLoc.north());
    }
    QToolTip::showText(event->globalPos(),QString::fromStdString(tipString),this);
  }
  //cout<<"mouseMove "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseClicked=mouseDoubleClicked=false;
}

void TinCanvas::mouseReleaseEvent(QMouseEvent *event)
{
  triangleHit hitRec;
  triangle *tri=nullptr;
  xy eventLoc=windowToWorld(event->pos());
  if (plnum<doc.pl.size() && plnum>=0)
    tri=doc.pl[plnum].qinx.findt(eventLoc,true);
  if (event->button()&Qt::LeftButton)
  {
    if (mouseDoubleClicked)
    {
      if (tri)
      {
        hitRec=tri->hitTest(eventLoc);
        if (hitRec.edg && hitRec.edg->isFlippable())
        {
          hitRec.edg->flip(&doc.pl[plnum]);
          updateEdgeNeighbors(hitRec.edg);
          roughContoursValid=false;
          surfaceValid=false;
        }
      }
    }
    else if (mouseClicked)
    {
      //cout<<"click"<<endl;
      if (tri)
      {
        hitRec=tri->hitTest(eventLoc);
        if (hitRec.edg)
        {
          hitRec.edg->broken^=1;
          updateEdge(hitRec.edg);
        }
      }
    }
    else
    {
      worldCenter+=eventLoc-dragStart;
      update(); // No need to update dragStart, since it's dragged.
    }
  }
  //cout<<"mouseRelease "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseClicked=mouseDoubleClicked=false;
}

TinWindow::TinWindow(QWidget *parent):QMainWindow(parent)
{
  resize(707,500);
  setWindowTitle(QApplication::translate("main", "ViewTIN"));
  show();
  toolbar=new QToolBar(this);
  addToolBar(Qt::TopToolBarArea,toolbar);
  //toolbar->setIconSize(QSize(40,40));
  canvas=new TinCanvas(this);
  setCentralWidget(canvas);
  canvas->show();
  makeActions();
  canvas->setMeter();
  connect(this,SIGNAL(zoomCanvas(int)),canvas,SLOT(zoom(int)));
}

TinWindow::~TinWindow()
{
  unmakeActions();
  delete canvas;
}

void TinWindow::makeActions()
{
  int i;
  fileMenu=menuBar()->addMenu(tr("&File"));
  editMenu=menuBar()->addMenu(tr("&Edit"));
  viewMenu=menuBar()->addMenu(tr("&View"));
  unitsMenu=menuBar()->addMenu(tr("&Units"));
  contourMenu=menuBar()->addMenu(tr("&Contour"));
  zoomButtons.push_back(new ZoomButton(this,-10));
  zoomButtons.back()->setIcon(QIcon(":/tenth.png"));
  zoomButtons.back()->setText(tr("Zoom out 10"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomm10()));
  zoomButtons.push_back(new ZoomButton(this,-3));
  zoomButtons.back()->setIcon(QIcon(":/half.png"));
  zoomButtons.back()->setText(tr("Zoom out 2"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomm3()));
  zoomButtons.push_back(new ZoomButton(this,-1));
  zoomButtons.back()->setIcon(QIcon(":/four-fifths.png"));
  zoomButtons.back()->setText(tr("Zoom out"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomm1()));
  zoomButtons.push_back(new ZoomButton(this,1));
  zoomButtons.back()->setIcon(QIcon(":/five-fourths.png"));
  zoomButtons.back()->setText(tr("Zoom in"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomp1()));
  zoomButtons.push_back(new ZoomButton(this,3));
  zoomButtons.back()->setIcon(QIcon(":/two.png"));
  zoomButtons.back()->setText(tr("Zoom in 2"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomp3()));
  zoomButtons.push_back(new ZoomButton(this,10));
  zoomButtons.back()->setIcon(QIcon(":/ten.png"));
  zoomButtons.back()->setText(tr("Zoom in 10"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(zoomp10()));
  zoomButtons.push_back(new ZoomButton(this,0,-DEG45/4));
  zoomButtons.back()->setIcon(QIcon(":/cw.png"));
  zoomButtons.back()->setText(tr("Rotate right"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(rotatecw()));
  zoomButtons.push_back(new ZoomButton(this,0,DEG45/4));
  zoomButtons.back()->setIcon(QIcon(":/ccw.png"));
  zoomButtons.back()->setText(tr("Rotate left"));
  connect(zoomButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(rotateccw()));
  for (i=0;i<zoomButtons.size();i++)
  {
    toolbar->addAction(zoomButtons[i]);
    viewMenu->addAction(zoomButtons[i]);
  }
  asterAction=new QAction(this);
  //asterAction->setIcon(QIcon(":/aster.png"));
  asterAction->setText(tr("Test pattern Aster"));
  fileMenu->addAction(asterAction);
  connect(asterAction,SIGNAL(triggered(bool)),canvas,SLOT(testPatternAster()));
  importPnezdAction=new QAction(this);
  //importPnezdAction->setIcon(QIcon(":/pnezd.png"));
  importPnezdAction->setText(tr("Import PNEZD file"));
  fileMenu->addAction(importPnezdAction);
  connect(importPnezdAction,SIGNAL(triggered(bool)),canvas,SLOT(importPnezd()));
  importCriteriaAction=new QAction(this);
  //importPnezdAction->setIcon(QIcon(":/criteria.png"));
  importCriteriaAction->setText(tr("Import Criteria file"));
  fileMenu->addAction(importCriteriaAction);
  connect(importCriteriaAction,SIGNAL(triggered(bool)),canvas,SLOT(importCriteria()));
  makeTinAction=new QAction(this);
  //makeTinAction->setIcon(QIcon(":/maketin.png"));
  makeTinAction->setText(tr("Make TIN"));
  contourMenu->addAction(makeTinAction);
  connect(makeTinAction,SIGNAL(triggered(bool)),canvas,SLOT(makeTin()));
  selectContourIntervalAction=new QAction(this);
  //makeTinAction->setIcon(QIcon(":/selectci.png"));
  selectContourIntervalAction->setText(tr("Select contour interval"));
  contourMenu->addAction(selectContourIntervalAction);
  connect(selectContourIntervalAction,SIGNAL(triggered(bool)),canvas,SLOT(selectContourInterval()));
  roughContoursAction=new QAction(this);
  //makeTinAction->setIcon(QIcon(":/roughcon.png"));
  roughContoursAction->setText(tr("Draw rough contours"));
  contourMenu->addAction(roughContoursAction);
  connect(roughContoursAction,SIGNAL(triggered(bool)),canvas,SLOT(roughContours()));
  measureButtons.push_back(new MeasureButton(this,METER,0));
  measureButtons.back()->setIcon(QIcon(":/meter.png"));
  measureButtons.back()->setText(tr("Meter"));
  toolbar->addAction(measureButtons.back());
  unitsMenu->addAction(measureButtons.back());
  connect(measureButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(setMeter()));
  measureButtons.push_back(new MeasureButton(this,FOOT,0));
  measureButtons.back()->setIcon(QIcon(":/foot.png"));
  measureButtons.back()->setText(tr("Foot"));
  toolbar->addAction(measureButtons.back());
  unitsMenu->addAction(measureButtons.back());
  connect(measureButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(setFoot()));
  measureButtons.push_back(new MeasureButton(this,0,INTERNATIONAL));
  measureButtons.back()->setIcon(QIcon(":/international-foot.png"));
  measureButtons.back()->setText(tr("International foot"));
  toolbar->addAction(measureButtons.back());
  unitsMenu->addAction(measureButtons.back());
  connect(measureButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(setInternationalFoot()));
  measureButtons.push_back(new MeasureButton(this,0,USSURVEY));
  measureButtons.back()->setIcon(QIcon(":/us-foot.png"));
  measureButtons.back()->setText(tr("US survey foot"));
  toolbar->addAction(measureButtons.back());
  unitsMenu->addAction(measureButtons.back());
  connect(measureButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(setUSFoot()));
  measureButtons.push_back(new MeasureButton(this,0,INSURVEY));
  measureButtons.back()->setIcon(QIcon(":/indian-foot.png"));
  measureButtons.back()->setText(tr("Indian survey foot"));
  toolbar->addAction(measureButtons.back());
  unitsMenu->addAction(measureButtons.back());
  connect(measureButtons.back(),SIGNAL(triggered(bool)),canvas,SLOT(setIndianFoot()));
  for (i=0;i<measureButtons.size();i++)
    connect(canvas,SIGNAL(measureChanged(Measure)),measureButtons[i],SLOT(setMeasure(Measure)));
}

void TinWindow::unmakeActions()
{
  int i;
  for (i=0;i<zoomButtons.size();i++)
  {
    toolbar->removeAction(zoomButtons[i]);
    viewMenu->removeAction(zoomButtons[i]);
    delete zoomButtons[i];
  }
  zoomButtons.clear();
  for (i=0;i<measureButtons.size();i++)
  {
    toolbar->removeAction(measureButtons[i]);
    unitsMenu->removeAction(measureButtons[i]);
    delete measureButtons[i];
  }
  measureButtons.clear();
  fileMenu->removeAction(asterAction);
  delete asterAction;
  asterAction=nullptr;
  fileMenu->removeAction(importPnezdAction);
  delete importPnezdAction;
  importPnezdAction=nullptr;
}

void TinWindow::prepareZoomSteps(int steps)
{
  cout<<"prepareZoomSteps "<<steps<<endl;
  preZoomStep=steps;
}

void TinWindow::zoomSteps(bool checked)
{
  zoomCanvas(preZoomStep);
}
