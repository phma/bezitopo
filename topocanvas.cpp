/******************************************************/
/*                                                    */
/* topocanvas.cpp - canvas for drawing topography     */
/*                                                    */
/******************************************************/
/* Copyright 2017-2019 Pierre Abbat.
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
#include "topocanvas.h"
#include "zoom.h"
#include "test.h"
#include "ldecimal.h"
#include "color.h"
#include "penwidth.h"
#include "dxf.h"

#define CACHEDRAW

using namespace std;

geoheader ghead;
/* Only one geoid can be loaded at present. Possibly in the future, there will
 * be an array of geoids, with each document holding a pointer to the geoid
 * it is using. The geoid data are in cube.
 */

TopoCanvas::TopoCanvas(QWidget *parent):QWidget(parent)
{
  int i,j,rgb;
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
  contourLineType[0]=16262;
  contourLineType[1]=SOLIDLINE;
  contourLineType[2]=SOLIDLINE;
  contourThickness[0]=-12;
  contourThickness[1]=0;
  contourThickness[2]=7;
  for (i=0;i<3;i++)
    for (j=0;j<20;j++)
    {
      contourColor[j]=RED/39*(39-j)+BLUE/39*j;
      rgb=colorint(contourColor[j]);
      contourPen[i][j]=QPen(QColor((rgb>>16)&0xff,(rgb>>8)&0xff,(rgb>>0)&0xff),
                            (i+1)/2.,i?Qt::SolidLine:Qt::DashLine);
    }
  errorMessage=new QErrorMessage(this);
  fileDialog=new QFileDialog(this);
  progressDialog=new QProgressDialog(this);
  progressDialog->reset();
  ciDialog=new ContourIntervalDialog(this);
  timer=new QTimer(this);
  plnum=-1;
  goal=DONE;
  rotation=0;
  tipXyz=false;
  showDelaunay=true;
  allowFlip=true;
  //for (i=0;i<doc.pl[1].edges.size();i++)
    //doc.pl[1].edges[i].dump(&doc.pl[1]);
  show();
  sizeToFit();
  trianglesShouldBeCurvy=true;
  contoursShouldBeCurvy=true;
}

QPointF TopoCanvas::worldToWindow(xy pnt)
{
  pnt.roscat(worldCenter,rotation,zoomratio(scale)*windowSize,windowCenter);
  QPointF ret(pnt.getx(),height()-pnt.gety());
  return ret;
}

xy TopoCanvas::windowToWorld(QPointF pnt)
{
  xy ret(pnt.x(),height()-pnt.y());
  ret.roscat(windowCenter,-rotation,zoomratio(-scale)/windowSize,worldCenter);
  return ret;
}

double TopoCanvas::pixelScale()
{
  return zoomratio(-scale)/windowSize;
}

double TopoCanvas::viewableRadius()
{
  return zoomratio(-scale)/windowSize*(1+windowDiag/2);
}

void TopoCanvas::setBrush(const QBrush &qbrush)
{
  brush=qbrush;
}

void TopoCanvas::sizeToFit()
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

void TopoCanvas::zoom(int steps)
{
  scale+=steps;
  update();
}

void TopoCanvas::zoomm10()
{
  zoom(-10);
}

void TopoCanvas::zoomm3()
{
  zoom(-3);
}

void TopoCanvas::zoomm1()
{
  zoom(-1);
}

void TopoCanvas::zoomp1()
{
  zoom(1);
}

void TopoCanvas::zoomp3()
{
  zoom(3);
}

void TopoCanvas::zoomp10()
{
  zoom(10);
}

void TopoCanvas::setShowDelaunay(bool showd)
{
  showDelaunay=showd;
}

void TopoCanvas::setAllowFlip(bool allow)
{
  allowFlip=allow;
}

void TopoCanvas::setTipXyz(bool tipxyz)
{
  tipXyz=tipxyz;
}

void TopoCanvas::rotatecw()
{
  rotation-=DEG45/4;
  update();
}

void TopoCanvas::rotateccw()
{
  rotation+=DEG45/4;
  update();
}

void TopoCanvas::setButtonBits(int bits)
{
  trianglesShouldBeCurvy=(bits>>0)&1;
  contoursShouldBeCurvy=(bits>>1)&1;
}

void TopoCanvas::setMeter()
{
  doc.ms.setMetric();
  measureChanged(doc.ms);
}

void TopoCanvas::setFoot()
{
  doc.ms.setCustomary();
  measureChanged(doc.ms);
}

void TopoCanvas::setInternationalFoot()
{
  doc.ms.setFoot(INTERNATIONAL);
  measureChanged(doc.ms);
}

void TopoCanvas::setUSFoot()
{
  doc.ms.setFoot(USSURVEY);
  measureChanged(doc.ms);
}

void TopoCanvas::setIndianFoot()
{
  doc.ms.setFoot(INSURVEY);
  measureChanged(doc.ms);
}

void TopoCanvas::repaintSeldom()
// Spends up to 5% of the time repainting during long operations.
{
  if (lastPaintTime.elapsed()>20*lastPaintDuration)
    repaint();
}

void TopoCanvas::updateEdge(edge *e)
{
  QMarginsF marge(1,1,1,1);
  QPointF aWindow=worldToWindow(*e->a);
  QPointF bWindow=worldToWindow(*e->b);
  QRectF rect(aWindow,bWindow);
  rect+=marge;
  update(rect.toAlignedRect());
}

void TopoCanvas::updateEdgeNeighbors(edge *e)
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

void TopoCanvas::open()
/* For now, this reads a bare TIN from a DXF file.
 * When I implement reading Bezitopo files, I'll duplicate it.
 */
{
  int i,j;
  int dialogResult;
  QStringList files;
  string fileName;
  ifstream file;
  double unit;
  vector<array<xyz,3> > bareTriangles;
  fileDialog->setWindowTitle(tr("Load TIN"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  fileDialog->setNameFilter(tr("(*.dxf);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    doc.pl.clear();
    doc.makepointlist(1);
    unit=doc.ms.toCoherent(1,LENGTH);
    bareTriangles=extractTriangles(readDxfGroups(fileName));
    if (unit!=1)
      for (i=0;i<bareTriangles.size();i++)
	for (j=0;j<3;j++)
	  bareTriangles[i][j]*=unit;
    doc.pl[1].makeBareTriangles(bareTriangles);
    bareTriangles.clear();
    cout<<"Read "<<doc.pl[1].triangles.size()<<" triangles\n";
    doc.pl[1].fillInBareTin();
    cout<<doc.pl[1].triangles.size()<<" triangles after filling in\n";
    doc.pl[1].addperimeter();
    plnum=1;
    sizeToFit();
    pointsValid=true;
    tinValid=true;
    surfaceValid=true;
    roughContoursValid=false;
  }
}

void TopoCanvas::saveAs()
{
  int i;
  int dialogResult;
  QStringList files;
  string fileName;
  ofstream file;
  fileDialog->setWindowTitle(tr("Save Drawing"));
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->setNameFilter(tr("(*.bez);;(*)"));
  fileDialog->setDefaultSuffix(QString("bez"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    docFileName=fileName;
    file.open(fileName,fstream::out);
    doc.writeXml(file);
    file.close();
  }
}

void TopoCanvas::save()
{
  ofstream file;
  if (docFileName.length())
  {
    file.open(docFileName,fstream::out);
    doc.writeXml(file);
    file.close();
  }
  else
    saveAs();
}

void TopoCanvas::testPatternAster()
{
  doc.pl.clear();
  doc.makepointlist(1);
  plnum=1;
  aster(doc,100);
  sizeToFit();
}

void TopoCanvas::importPnezd()
{
  int dialogResult;
  QStringList files;
  string fileName;
  fileDialog->setWindowTitle(tr("Open PNEZD File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  fileDialog->setNameFilter(tr("(*.asc);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    // TODO check whether there are unsaved changes to breaklines
    doc.pl.clear();
    doc.makepointlist(0);
    doc.readpnezd(fileName);
    plnum=0;
    sizeToFit();
    pointsValid=false;
    tinValid=false;
    surfaceValid=false;
    roughContoursValid=false;
  }
}

void TopoCanvas::importCriteria()
{
  int dialogResult;
  QStringList files;
  string fileName;
  fileDialog->setWindowTitle(tr("Open Criteria File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  fileDialog->setNameFilter(tr("(*.crit);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    doc.makepointlist(1);
    doc.pl[1].readCriteria(fileName,doc.ms);
    pointsValid=false;
    tinValid=false;
    surfaceValid=false;
    roughContoursValid=smoothContoursValid=false;
  }
}

void TopoCanvas::importBreaklines()
{
  int i,err=0;
  bool loadAnyway;
  int dialogResult;
  QStringList files;
  QString errMsg;
  string fileName,line;
  fstream file;
  vector<Breakline0> saveBreaklines0;
  fileDialog->setWindowTitle(tr("Open Breakline File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  fileDialog->setNameFilter(tr("(*.brk);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    if (doc.pl[plnum].whichBreak0Valid==2)
    {
      QMessageBox msgBox(this);
      msgBox.setText(tr("You have edited breaklines."));
      msgBox.setInformativeText(tr("Do you want to load?"));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setIcon(QMessageBox::Question);
      msgBox.setDefaultButton(QMessageBox::No);
      loadAnyway=msgBox.exec()==QMessageBox::Yes;
    }
    else
      loadAnyway=true;
    if (loadAnyway)
    {
      files=fileDialog->selectedFiles();
      fileName=files[0].toStdString();
      saveBreaklines0=doc.pl[plnum].type0Breaklines;
      try
      {
        doc.pl[plnum].type0Breaklines.clear();
        file.open(fileName,fstream::in);
        while (!file.eof() && !file.fail())
        {
          getline(file,line);
          doc.pl[plnum].stringToBreakline(line);
        }
        if (!file.eof())
          throw BeziExcept(fileError);
        tinValid=surfaceValid=roughContoursValid=smoothContoursValid=false;
        doc.pl[plnum].whichBreak0Valid=1;
      }
      catch (BeziExcept e)
      {
        err=e.getNumber();
        errMsg=e.message();
      }
      if (err)
      {
        QString msg=tr("Can't read breaklines. Error: ")+errMsg;
        doc.pl[plnum].type0Breaklines=saveBreaklines0;
        errorMessage->showMessage(msg);
      }
    }
  }
}

void TopoCanvas::exportBreaklines()
{
  int i;
  int dialogResult;
  QStringList files;
  string fileName;
  fstream file;
  fileDialog->setWindowTitle(tr("Save Breakline File"));
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->setNameFilter(tr("(*.brk);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    file.open(fileName,fstream::out);
    if (doc.pl[plnum].whichBreak0Valid!=1)
      doc.pl[plnum].edgesToBreaklines();
    for (i=0;i<doc.pl[plnum].type0Breaklines.size();i++)
    {
      doc.pl[plnum].type0Breaklines[i].writeText(file);
      file<<endl;
    }
  }
}

bool TopoCanvas::makeTinCheckEdited()
/* If the user asks to make the TIN, but the edited TIN is more recent
 * than the imported breaklines, pops up a message box and asks if he wants
 * to make the TIN. Else returns true.
 */
{
  bool ret;
  if (doc.pl[plnum].whichBreak0Valid==2)
  {
    QMessageBox msgBox(this);
    msgBox.setText(tr("You have edited breaklines."));
    msgBox.setInformativeText(tr("Do you want to make TIN?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setDefaultButton(QMessageBox::No);
    ret=msgBox.exec()==QMessageBox::Yes;
  }
  else
    ret=true;
  return ret;
}

void TopoCanvas::makeTin()
{
  //cout<<"makeTin"<<endl;
  doc.makepointlist(1);
  if ((doc.pl[1].size()==0 && doc.pl[0].size()>0) || !pointsValid)
  {
    if (doc.pl[1].crit.size()==0)
    {
      doc.pl[1].crit.resize(1);
      doc.pl[1].crit[0].istopo=true;
    }
    if (doc.pl[1].edges.size()==0 && doc.pl[1].type0Breaklines.size()==0
        && doc.pl[0].type0Breaklines.size()>0)
      doc.pl[1].type0Breaklines=doc.pl[0].type0Breaklines;
    doc.copytopopoints(1,0);
    pointsValid=true;
  }
  plnum=1;
  tinerror=startPointTries=passCount=0;
  if (doc.pl[plnum].size()<3)
    tinerror=notri;
  else
    startPoint=doc.pl[1].points.begin()->second;
  if (makeTinCheckEdited())
  {
    try
    {
      doc.pl[plnum].splitBreaklines();
    }
    catch (BeziExcept e)
    {
      tinerror=e.getNumber();
    }
    progressDialog->setRange(0,100);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowTitle(tr("Making TIN"));
    progressDialog->setLabelText(tr("Sweeping convex hull..."));
    if (goal==DONE)
    {
      goal=MAKE_TIN;
      timer->start(0);
      progressDialog->show();
    }
    disconnect(timer,SIGNAL(timeout()),0,0);
    connect(timer,SIGNAL(timeout()),this,SLOT(tryStartPoint()));
    connect(progressDialog,SIGNAL(canceled()),this,SLOT(tinCancel()));
  }
}

void TopoCanvas::tryStartPoint()
{
  //cout<<"tryStartPoint"<<endl;
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
        progressDialog->setWindowTitle(tr("Making TIN"));
        progressDialog->setLabelText(tr("Flipping edges..."));
      }
    }
    catch (BeziExcept e)
    {
      tinerror=e.getNumber();
    }
  }
  repaintSeldom();
}

void TopoCanvas::flipPass()
{
  //cout<<"flipPass"<<endl;
  int nFlip,nGoodEdges,i;
  bool tooLong=false;
  if (passCount*3>doc.pl[plnum].size() && doc.pl[plnum].size()>0)
    tooLong=true; // tooLong is not an error. The TIN should be valid; it's just
  if (tinerror || tooLong) // repeatedly flipping edges in a circle.
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(flipPass()));
    connect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
    if (tooLong)
      errorMessage->showMessage(tr("Took too long to make TIN"));
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
        tinValid=true;
      }
    }
    catch (BeziExcept e)
    {
      tinerror=e.getNumber();
    }
  }
  repaintSeldom();
}

void TopoCanvas::redoSurface()
/* This can be done as part of making the TIN, or when making the contours it
 * can find that this needs to be done first.
 */
{
  //cout<<"redoSurface"<<endl;
  if (tinValid)
  {
    doc.pl[plnum].makegrad(0.15);
    doc.pl[plnum].maketriangles();
    doc.pl[plnum].setgradient(!trianglesShouldBeCurvy);
    doc.pl[plnum].makeqindex();           // These five are all fast. It's finding the
    doc.pl[plnum].findedgecriticalpts();  // critical points of a triangle that's slow.
    trianglesAreCurvy=trianglesShouldBeCurvy;
  }
  progressDialog->setRange(0,doc.pl[plnum].triangles.size());
  progressDialog->setWindowTitle(tr("Making surface"));
    progressDialog->setLabelText(tr("Finding critical points..."));
  triCount=0;
  roughContoursValid=smoothContoursValid=false;
  disconnect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
  if (tinValid || tinerror)
    connect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
  else
    connect(timer,SIGNAL(timeout()),this,SLOT(makeTin()));
}

void TopoCanvas::findCriticalPoints()
{
  //cout<<"findCriticalPoints"<<endl;
  if (tinerror)
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
    connect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
  }
  else
  {
    try
    {
      doc.pl[plnum].triangles[triCount].findcriticalpts();
      doc.pl[plnum].triangles[triCount++].subdivide();
      progressDialog->setValue(triCount);
      if (triCount==doc.pl[plnum].triangles.size())
      {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(findCriticalPoints()));
        connect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
      }
    }
    catch (BeziExcept e)
    {
      tinerror=e.getNumber();
    }
  }
}

void TopoCanvas::makeTinFinish()
{
  //cout<<"makeTinFinish"<<endl;
  if (tinerror)
  { // TODO: translate the thrown error into something intelligible
    QString msg=tr("Can't make TIN. Error: ")+translateException(tinerror);
    doc.pl[plnum].clearTin();
    errorMessage->showMessage(msg);
  }
  else
  {
    doc.pl[plnum].addperimeter();
    doc.pl[plnum].whichBreak0Valid=3;
  }
  update();
  disconnect(timer,SIGNAL(timeout()),this,SLOT(makeTinFinish()));
  switch (goal)
  {
    case MAKE_TIN:
      goal=DONE;
      progressDialog->reset();
      timer->stop();
      break;
    case ROUGH_CONTOURS:
    case SMOOTH_CONTOURS:
      connect(timer,SIGNAL(timeout()),this,SLOT(roughContours()));
      break;
  }
  roughContoursValid=smoothContoursValid=false;
  surfaceValid=true;
}

void TopoCanvas::tinCancel()
{
  goal=DONE;
  progressDialog->reset();
  timer->stop();
  disconnect(timer,SIGNAL(timeout()),0,0);
  update();
}

document *TopoCanvas::getDoc()
{
  return &doc;
}

void TopoCanvas::selectContourInterval()
{
  if (plnum>=0 && plnum<doc.pl.size())
    ciDialog->set(&doc.pl[plnum].contourInterval,doc.ms);
  else
    ciDialog->set(nullptr,doc.ms);
  ciDialog->exec();
}

void TopoCanvas::roughContours()
{
  conterval=doc.pl[plnum].contourInterval.fineInterval();
  if (goal==DONE)
  {
    goal=ROUGH_CONTOURS;
    timer->start(0);
    progressDialog->show();
  }
  if (tinValid)
    tinlohi=doc.pl[plnum].lohi();
  doc.pl[plnum].contours.clear();
  elevLo=floor(tinlohi[0]/conterval);
  elevHi=ceil(tinlohi[1]/conterval);
  progInx=elevLo;
  progressDialog->setRange(elevLo,elevHi);
  progressDialog->setValue(progInx);
  progressDialog->setWindowTitle(tr("Drawing contours"));
  progressDialog->setLabelText(tr("Drawing rough contours..."));
  if (trianglesAreCurvy!=trianglesShouldBeCurvy)
    surfaceValid=false;
  connect(progressDialog,SIGNAL(canceled()),this,SLOT(contoursCancel()));
  disconnect(timer,SIGNAL(timeout()),0,0);
  if (surfaceValid)
    connect(timer,SIGNAL(timeout()),this,SLOT(rough1Contour()));
  else if (tinValid)
    connect(timer,SIGNAL(timeout()),this,SLOT(redoSurface()));
  else
    connect(timer,SIGNAL(timeout()),this,SLOT(makeTin()));
}

void TopoCanvas::rough1Contour()
{
  rough1contour(doc.pl[plnum],progInx*conterval);
  if (++progInx>elevHi)
  {
    disconnect(timer,SIGNAL(timeout()),this,SLOT(rough1Contour()));
    connect(timer,SIGNAL(timeout()),this,SLOT(roughContoursFinish()));
  }
  else
    progressDialog->setValue(progInx);
  repaintSeldom();
}

void TopoCanvas::roughContoursFinish()
{
  disconnect(timer,SIGNAL(timeout()),this,SLOT(roughContoursFinish()));
  switch (goal)
  {
    case ROUGH_CONTOURS:
      goal=DONE;
      progressDialog->reset();
      timer->stop();
      break;
    case SMOOTH_CONTOURS:
      connect(timer,SIGNAL(timeout()),this,SLOT(smoothContours()));
      break;
  }
  roughContoursValid=true;
  smoothContoursValid=false;
  update();
}

void TopoCanvas::contoursCancel()
{
  goal=DONE;
  progressDialog->reset();
  timer->stop();
  disconnect(timer,SIGNAL(timeout()),0,0);
  update();
}

void TopoCanvas::smoothContours()
{
  if (goal==DONE)
  {
    goal=SMOOTH_CONTOURS;
    timer->start(0);
    progressDialog->show();
  }
  progInx=0;
  progressDialog->setRange(0,doc.pl[plnum].contours.size());
  progressDialog->setValue(0);
  progressDialog->setWindowTitle(tr("Drawing contours"));
  progressDialog->setLabelText(tr("Drawing smooth contours..."));
  if (contoursAreCurvy!=contoursShouldBeCurvy || trianglesAreCurvy!=trianglesShouldBeCurvy)
  {
    roughContoursValid=false;
    contoursAreCurvy=contoursShouldBeCurvy;
  }
  connect(progressDialog,SIGNAL(canceled()),this,SLOT(contoursCancel()));
  disconnect(timer,SIGNAL(timeout()),0,0);
  if (roughContoursValid && conterval==doc.pl[plnum].contourInterval.fineInterval())
    connect(timer,SIGNAL(timeout()),this,SLOT(smooth1Contour()));
  else
    connect(timer,SIGNAL(timeout()),this,SLOT(roughContours()));
}

void TopoCanvas::smooth1Contour()
{
  if (progInx<doc.pl[plnum].contours.size())
  {
    smooth1contour(doc.pl[plnum],conterval,progInx,contoursShouldBeCurvy,dummyPs,0,0,0,0);
    progressDialog->setValue(++progInx);
  }
  else
  {
    disconnect(timer,SIGNAL(timeout()),0,0);
    connect(timer,SIGNAL(timeout()),this,SLOT(smoothContoursFinish()));
  }
  repaintSeldom();
}

void TopoCanvas::smoothContoursFinish()
{
  switch (goal)
  {
    case SMOOTH_CONTOURS:
      goal=DONE;
      contoursAreCurvy=contoursShouldBeCurvy;
      progressDialog->reset();
      timer->stop();
      break;
  }
  disconnect(timer,SIGNAL(timeout()),0,0);
  smoothContoursValid=true;
  update();
}

void TopoCanvas::loadGeoid()
{
  int dialogResult,err=0;
  QString errMsg;
  QStringList files;
  string fileName;
  fileDialog->setWindowTitle(tr("Load Geoid File"));
  fileDialog->setFileMode(QFileDialog::ExistingFile);
  fileDialog->setNameFilter(tr("(*.bol);;(*)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    if (fileName.length())
    {
      try
      {
	ifstream geofile(fileName,ios::binary);
	ghead.readBinary(geofile);
	cube.scale=pow(2,ghead.logScale);
	cube.readBinary(geofile);
	cout<<"read "<<fileName<<endl;
      }
      catch(BeziExcept e)
      {
        err=e.getNumber();
        errMsg=e.message();
      }
      if (err)
      {
        QString msg=tr("Can't read geoid. Error: ")+errMsg;
        errorMessage->showMessage(msg);
      }
    }
  }
}

void TopoCanvas::dump()
/* For debugging.
 * This method outputs any state of the program that may be useful for
 * debugging whatever needs to be debugged at the moment.
 */
{
  int i,j;
  cout<<doc.pl.size()<<" pointlists\n";
  for (i=0;i<doc.pl.size();i++)
  {
    cout<<"Pointlist "<<i<<":\n";
    cout<<doc.pl[i].points.size()<<" points\n";
    cout<<doc.pl[i].edges.size()<<" edges\n";
    cout<<doc.pl[i].triangles.size()<<" triangles\n";
    cout<<doc.pl[i].contours.size()<<" contours\n";
    cout<<doc.pl[i].crit.size()<<" criteria\n";
    cout<<doc.pl[i].type0Breaklines.size()<<" breaklines"<<endl;
  }
}

void TopoCanvas::paintEvent(QPaintEvent *event)
{
  int i,k,contourType,renderTime=0,pathTime=0,strokeTime=0;
  double r;
  bezier3d b3d;
  ptlist::iterator j;
  set<edge *>::iterator e;
  RenderItem ri;
  QTime paintTime,subTime;
  QPen itemPen;
  QPainter painter(this);
  QPainterPath path;
  vector<xyz> beziseg;
  segment seg;
  paintTime.start();
  painter.setBrush(brush);
  painter.setRenderHint(QPainter::Antialiasing,true);
  if (plnum<doc.pl.size() && plnum>=0)
  {
    doc.pl[plnum].setLocalSets(worldCenter,viewableRadius());
    if (doc.pl[plnum].triangles.size())
      if (doc.pl[plnum].localEdges.count(nullptr))
	for (i=0;plnum>=0 && i<doc.pl[plnum].edges.size();i++)
	{
	  seg=doc.pl[plnum].edges[i].getsegment();
	  if (seg.length()>pixelScale() && fabs(pldist(worldCenter,seg.getstart(),seg.getend()))<viewableRadius())
	  {
	    if (!showDelaunay || doc.pl[plnum].edges[i].delaunay())
	      if (doc.pl[plnum].edges[i].broken&1)
		painter.setPen(breakEdgePen);
	      else
		painter.setPen(normalEdgePen);
	    else
	      painter.setPen(flipEdgePen);
	    painter.drawLine(worldToWindow(seg.getstart()),worldToWindow(seg.getend()));
	  }
	}
      else
	for (e=doc.pl[plnum].localEdges.begin();e!=doc.pl[plnum].localEdges.end();++e)
	{
	  seg=(*e)->getsegment();
	  if (seg.length()>pixelScale() && fabs(pldist(worldCenter,seg.getstart(),seg.getend()))<viewableRadius())
	  {
	    if (!showDelaunay || (*e)->delaunay())
	      if ((*e)->broken&1)
		painter.setPen(breakEdgePen);
	      else
		painter.setPen(normalEdgePen);
	    else
	      painter.setPen(flipEdgePen);
	    painter.drawLine(worldToWindow(seg.getstart()),worldToWindow(seg.getend()));
	  }
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
#ifdef CACHEDRAW
    contourCache.clearPresent();
    subTime.start();
    for (i=0;i<doc.pl[plnum].contours.size();i++)
    {
      contourType=doc.pl[plnum].contourInterval.contourType(doc.pl[plnum].contours[i].getElevation());
      contourCache.checkInObject(&doc.pl[plnum].contours[i],pixelScale(),
                                 -1,contourColor[contourType&31],contourThickness[contourType>>8],contourLineType[contourType>>8]);
    }
    contourCache.deleteAbsent();
    renderTime+=subTime.elapsed();
    do
    {
      ri=contourCache.nextRenderItem();
      for (i=0;ri.present && i<ri.rendering.size();i++)
      {
        setColor(itemPen,ri.rendering[i].color);
        setWidth(itemPen,ri.rendering[i].width);
        setLineType(itemPen,ri.rendering[i].linetype);
        b3d=ri.rendering[i].path;
        subTime.start();
        path=QPainterPath();
        for (k=0;k<b3d.size();k++)
        {
          beziseg=b3d[k];
          if (k==0)
            path.moveTo(worldToWindow(beziseg[0]));
          path.cubicTo(worldToWindow(beziseg[1]),worldToWindow(beziseg[2]),worldToWindow(beziseg[3]));
        }
        if (!b3d.isopen())
          path.closeSubpath();
        pathTime+=subTime.restart();
        painter.strokePath(path,itemPen);
        strokeTime+=subTime.elapsed();
      }
    } while (ri.present);
#else
    for (i=0;i<doc.pl[plnum].contours.size();i++)
    {
      b3d=doc.pl[plnum].contours[i].approx3d(pixelScale());
      path=QPainterPath();
      for (k=0;k<b3d.size();k++)
      {
        beziseg=b3d[k];
        if (k==0)
          path.moveTo(worldToWindow(beziseg[0]));
        path.cubicTo(worldToWindow(beziseg[1]),worldToWindow(beziseg[2]),worldToWindow(beziseg[3]));
      }
      if (!doc.pl[plnum].contours[i].isopen())
        path.closeSubpath();
      contourType=doc.pl[plnum].contourInterval.contourType(doc.pl[plnum].contours[i].getElevation());
      painter.strokePath(path,contourPen[contourType>>8][contourType&31]);
    }
#endif
  }
  else
    ; // nothing to paint, since plnum is not the index of a pointlist
  //cout<<"Painting took "<<paintTime.elapsed()<<" ms, rendering "<<renderTime<<", paths "<<pathTime<<", stroke "<<strokeTime<<endl;
  lastPaintTime=paintTime;
  lastPaintDuration=paintTime.elapsed();
}

void TopoCanvas::setSize()
{
  windowCenter=xy(width(),height())/2.;
  windowSize=1/sqrt(1/sqr(width())+1/sqr(height()));
  windowDiag=sqrt(sqr(width())+sqr(height()));
}

void TopoCanvas::resizeEvent(QResizeEvent *event)
{
  setSize();
  QWidget::resizeEvent(event);
}

bool TopoCanvas::mouseCheckImported()
/* If the user clicks on an edge to edit the breaklines in the TIN, but the
 * breaklines imported from a file are more recent, pops up a message box
 * and asks if he wants to edit the TIN. Else returns true.
 */
{
  bool ret;
  if (doc.pl[plnum].whichBreak0Valid==1)
  {
    QMessageBox msgBox(this);
    msgBox.setText(tr("You have imported breaklines."));
    msgBox.setInformativeText(tr("Do you want to edit?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setDefaultButton(QMessageBox::No);
    ret=msgBox.exec()==QMessageBox::Yes;
  }
  else
    ret=true;
  return ret;
}

void TopoCanvas::mousePressEvent(QMouseEvent *event)
{
  xy eventLoc=windowToWorld(event->pos());
  if (event->button()==Qt::LeftButton)
    dragStart=eventLoc;
  //cout<<"mousePress "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseClicked=true;
}

void TopoCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
  xy eventLoc=windowToWorld(event->pos());
  //cout<<"mouseDoubleClick "<<eventLoc.east()<<','<<eventLoc.north()<<endl;
  mouseDoubleClicked=true;
}

void TopoCanvas::mouseMoveEvent(QMouseEvent *event)
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
      if (tipXyz)
	tipString=doc.ms.formatMeasurement(eventLoc.east(),LENGTH)+' '+
	          doc.ms.formatMeasurement(eventLoc.north(),LENGTH)+' '+
	          doc.ms.formatMeasurement(doc.pl[plnum].elevation(eventLoc),LENGTH);
      else
      {
	hitRec=tri->hitTest(eventLoc);
	tipString=doc.pl[plnum].hitTestString(hitRec);
      }
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

void TopoCanvas::mouseReleaseEvent(QMouseEvent *event)
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
        if (allowFlip && hitRec.edg && hitRec.edg->isFlippable() && mouseCheckImported())
        {
          hitRec.edg->flip(&doc.pl[plnum]);
          updateEdgeNeighbors(hitRec.edg);
          roughContoursValid=false;
          surfaceValid=false;
          doc.pl[plnum].whichBreak0Valid=2;
        }
      }
    }
    else if (mouseClicked)
    {
      //cout<<"click"<<endl;
      if (tri)
      {
        hitRec=tri->hitTest(eventLoc);
        if (allowFlip && hitRec.edg && mouseCheckImported())
        {
          hitRec.edg->broken^=1;
          updateEdge(hitRec.edg);
          doc.pl[plnum].whichBreak0Valid=2;
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
