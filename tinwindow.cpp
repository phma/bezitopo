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
#include "tinwindow.h"
#include "zoom.h"
#include "test.h"
#include "ldecimal.h"

using namespace std;

TinCanvas::TinCanvas(QWidget *parent):QWidget(parent)
{
  int i;
  setAutoFillBackground(true);
  setMouseTracking(true);
  setBackgroundRole(QPalette::Base);
  normalEdgePen=QPen(Qt::black);
  breakEdgePen=QPen(Qt::darkBlue);
  flipEdgePen=QPen(Qt::blue);
  doc.pl.resize(2);
  aster(doc,100);
  doc.pl[1].maketin("",false);
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  //for (i=0;i<doc.pl[1].edges.size();i++)
    //doc.pl[1].edges[i].dump(&doc.pl[1]);
  sizeToFit();
  show();
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

void TinCanvas::setBrush(const QBrush &qbrush)
{
  brush=qbrush;
}

void TinCanvas::sizeToFit()
{
  double top,left,bottom,right;
  int plnum;
  int vscale,hscale;
  plnum=doc.pl.size()-1;
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

void TinCanvas::updateEdge(edge *e)
{
  QPointF aWindow=worldToWindow(*e->a);
  QPointF bWindow=worldToWindow(*e->b);
  QRectF rect(aWindow,bWindow);
  update(rect.toRect());
}

void TinCanvas::updateEdgeNeighbors(edge *e)
{
  QPointF aWindow=worldToWindow(*e->a);
  QPointF bWindow=worldToWindow(*e->b);
  QPointF cWindow=worldToWindow(*e->nexta->otherend(e->a));
  QPointF dWindow=worldToWindow(*e->nextb->otherend(e->b));
  QRectF rect1(aWindow,bWindow),rect2(cWindow,dWindow);
  QRectF rect=rect1.united(rect2);
  update(rect.toRect());
}

void TinCanvas::paintEvent(QPaintEvent *event)
{
  int i,plnum;
  QPainter painter(this);
  segment seg;
  painter.setBrush(brush);
  painter.setRenderHint(QPainter::Antialiasing,true);
  plnum=doc.pl.size()-1;
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
  int plnum;
  string tipString;
  if (event->buttons()&Qt::LeftButton)
  {
    worldCenter+=dragStart-eventLoc;
    update(); // No need to update dragStart, since it's dragged.
  }
  else
  {
    plnum=doc.pl.size()-1;
    tri=doc.pl[plnum].findt(eventLoc,true); // Change this to false to see the locale bug.
    if (tri)
    {
      hitRec=tri->hitTest(eventLoc);
      tipString=doc.pl[plnum].hitTestString(hitRec);
    }
    else
      tipString=ldecimal(eventLoc.east())+','+ldecimal(eventLoc.north());
    QToolTip::showText(event->globalPos(),QString::fromStdString(tipString),this);
  }
  //cout<<"mouseMove "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseClicked=mouseDoubleClicked=false;
}

void TinCanvas::mouseReleaseEvent(QMouseEvent *event)
{
  int plnum;
  triangleHit hitRec;
  triangle *tri;
  plnum=doc.pl.size()-1;
  xy eventLoc=windowToWorld(event->pos());
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
  canvas=new TinCanvas(this);
  setCentralWidget(canvas);
  canvas->show();
  makeActions();
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
  zoomButtons[0]=new ZoomButton(this,-10);
  zoomButtons[0]->setIcon(QIcon(":/tenth.png"));
  connect(zoomButtons[0],SIGNAL(triggered(bool)),canvas,SLOT(zoomm10()));
  zoomButtons[1]=new ZoomButton(this,-3);
  zoomButtons[1]->setIcon(QIcon(":/half.png"));
  connect(zoomButtons[1],SIGNAL(triggered(bool)),canvas,SLOT(zoomm3()));
  zoomButtons[2]=new ZoomButton(this,-1);
  zoomButtons[2]->setIcon(QIcon(":/four-fifths.png"));
  connect(zoomButtons[2],SIGNAL(triggered(bool)),canvas,SLOT(zoomm1()));
  zoomButtons[3]=new ZoomButton(this,1);
  zoomButtons[3]->setIcon(QIcon(":/five-fourths.png"));
  connect(zoomButtons[3],SIGNAL(triggered(bool)),canvas,SLOT(zoomp1()));
  zoomButtons[4]=new ZoomButton(this,3);
  zoomButtons[4]->setIcon(QIcon(":/two.png"));
  connect(zoomButtons[4],SIGNAL(triggered(bool)),canvas,SLOT(zoomp3()));
  zoomButtons[5]=new ZoomButton(this,10);
  zoomButtons[5]->setIcon(QIcon(":/ten.png"));
  connect(zoomButtons[5],SIGNAL(triggered(bool)),canvas,SLOT(zoomp10()));
  for (i=0;i<6;i++)
  {
    toolbar->addAction(zoomButtons[i]);
  }
}

void TinWindow::unmakeActions()
{
  int i;
  for (i=0;i<6;i++)
  {
    toolbar->removeAction(zoomButtons[i]);
    delete zoomButtons[i];
    zoomButtons[i]=nullptr;
  }
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
