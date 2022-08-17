/******************************************************/
/*                                                    */
/* tinwindow.cpp - window for viewing TIN             */
/*                                                    */
/******************************************************/
/* Copyright 2017-2020,2022 Pierre Abbat.
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
#include <iostream>
#include <cmath>
#include "except.h"
#include "fileio.h"
#include "tinwindow.h"
#include "zoom.h"
#include "test.h"
#include "ldecimal.h"
#include "color.h"
#include "penwidth.h"
#include "dxf.h"
#include "tintext.h"

#define CACHEDRAW

using namespace std;

TinWindow::TinWindow(QWidget *parent):QMainWindow(parent)
{
  resize(707,500);
  showFileLoaded("");
  show();
  toolbar=new QToolBar(this);
  addToolBar(Qt::TopToolBarArea,toolbar);
  //toolbar->setIconSize(QSize(40,40));
  canvas=new TopoCanvas(this);
  setCentralWidget(canvas);
  llDialog=new LatlongFactorDialog(this);
  grDialog=new GridFactorDialog(this);
  canvas->setShowDelaunay(true);
  canvas->show();
  makeActions();
  canvas->setMeter();
  connect(canvas,SIGNAL(fileChanged(std::string)),this,SLOT(showFileLoaded(std::string)));
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
  coordMenu=menuBar()->addMenu(tr("&Coordinates"));
  helpMenu=menuBar()->addMenu(tr("&Help"));
  // View menu
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
  sizeToFitAction=new QAction(this);
  //sizeToFitAction->setIcon(QIcon(":/sizetofit.png"));
  sizeToFitAction->setText(tr("Size to Fit"));
  viewMenu->addAction(sizeToFitAction);
  connect(sizeToFitAction,SIGNAL(triggered(bool)),canvas,SLOT(sizeToFit()));
  // File menu
  openAction=new QAction(this);
  openAction->setIcon(QIcon::fromTheme("document-open"));
  openAction->setText(tr("Open"));
  //fileMenu->addAction(openAction);
  connect(openAction,SIGNAL(triggered(bool)),canvas,SLOT(open()));
  saveAction=new QAction(this);
  saveAction->setIcon(QIcon::fromTheme("document-save"));
  saveAction->setText(tr("Save"));
  fileMenu->addAction(saveAction);
  connect(saveAction,SIGNAL(triggered(bool)),canvas,SLOT(save()));
  saveAsAction=new QAction(this);
  saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
  saveAsAction->setText(tr("Save As"));
  fileMenu->addAction(saveAsAction);
  connect(saveAsAction,SIGNAL(triggered(bool)),canvas,SLOT(saveAs()));
  exportMenu=fileMenu->addMenu(tr("Export"));
  asterAction=new QAction(this);
  //asterAction->setIcon(QIcon(":/aster.png"));
  asterAction->setText(tr("Test pattern Aster"));
  //fileMenu->addAction(asterAction);
  connect(asterAction,SIGNAL(triggered(bool)),canvas,SLOT(testPatternAster()));
  importPnezdAction=new QAction(this);
  //importPnezdAction->setIcon(QIcon(":/pnezd.png"));
  importPnezdAction->setText(tr("Import PNEZD file"));
  fileMenu->addAction(importPnezdAction);
  connect(importPnezdAction,SIGNAL(triggered(bool)),canvas,SLOT(importPnezd()));
  importCriteriaAction=new QAction(this);
  //importCriteriaAction->setIcon(QIcon(":/criteria.png"));
  importCriteriaAction->setText(tr("Import Criteria file"));
  fileMenu->addAction(importCriteriaAction);
  connect(importCriteriaAction,SIGNAL(triggered(bool)),canvas,SLOT(importCriteria()));
  importBreaklinesAction=new QAction(this);
  //importBreaklinesAction->setIcon(QIcon(":/importbreak.png"));
  importBreaklinesAction->setText(tr("Import Breaklines file"));
  fileMenu->addAction(importBreaklinesAction);
  connect(importBreaklinesAction,SIGNAL(triggered(bool)),canvas,SLOT(importBreaklines()));
  exportBreaklinesAction=new QAction(this);
  //exportBreaklinesAction->setIcon(QIcon(":/exportbreak.png"));
  exportBreaklinesAction->setText(tr("Export Breaklines file"));
  fileMenu->addAction(exportBreaklinesAction);
  connect(exportBreaklinesAction,SIGNAL(triggered(bool)),canvas,SLOT(exportBreaklines()));
  exitAction=new QAction(this);
  exitAction->setIcon(QIcon::fromTheme("application-exit"));
  exitAction->setText(tr("Exit"));
  fileMenu->addAction(exitAction);
  connect(exitAction,SIGNAL(triggered(bool)),this,SLOT(close()));
  // Export menu
  exportTinTxtAction=new QAction(this);
  exportTinTxtAction->setText(tr("TIN Text"));
  exportMenu->addAction(exportTinTxtAction);
  connect(exportTinTxtAction,SIGNAL(triggered(bool)),this,SLOT(exportTinTxt()));
  // Contour menu
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
  smoothContoursAction=new QAction(this);
  //smoothContoursAction->setIcon(QIcon(":/smoothcon.png"));
  smoothContoursAction->setText(tr("Draw smooth contours"));
  contourMenu->addAction(smoothContoursAction);
  connect(smoothContoursAction,SIGNAL(triggered(bool)),canvas,SLOT(smoothContours()));
  curvyContourAction=new QAction(this);
  //curvyContourAction->setIcon(QIcon(":/curvycon.png"));
  curvyContourAction->setText(tr("Draw smooth contours with curves"));
  curvyContourAction->setCheckable(true);
  contourMenu->addAction(curvyContourAction);
  connect(curvyContourAction,SIGNAL(triggered(bool)),this,SLOT(changeButtonBits()));
  curvyContourAction->setChecked(true);
#ifndef FLATTRIANGLE
  curvyTriangleAction=new QAction(this);
  //curvyTriangleAction->setIcon(QIcon(":/curvytri.png"));
  curvyTriangleAction->setText(tr("Use curved triangular surfaces"));
  curvyTriangleAction->setCheckable(true);
  contourMenu->addAction(curvyTriangleAction);
  connect(curvyTriangleAction,SIGNAL(triggered(bool)),this,SLOT(changeButtonBits()));
  curvyTriangleAction->setChecked(true);
  connect(this,SIGNAL(buttonBitsChanged(int)),canvas,SLOT(setButtonBits(int)));
#endif
  // Coordinate menu
  loadGeoidAction=new QAction(this);
  //loadGeoidAction->setIcon(QIcon(":/loadgeoid.png"));
  loadGeoidAction->setText(tr("Load geoid file"));
  coordMenu->addAction(loadGeoidAction);
  connect(loadGeoidAction,SIGNAL(triggered(bool)),canvas,SLOT(loadGeoid()));
  gridToLatlongAction=new QAction(this);
  //gridToLatlongAction->setIcon(QIcon(":/gridtoll.png"));
  gridToLatlongAction->setText(tr("Grid to lat/long"));
  coordMenu->addAction(gridToLatlongAction);
  connect(gridToLatlongAction,SIGNAL(triggered(bool)),this,SLOT(gridToLatlong()));
  latlongToGridAction=new QAction(this);
  //latlongToGridAction->setIcon(QIcon(":/lltogrid.png"));
  latlongToGridAction->setText(tr("Lat/long to grid"));
  coordMenu->addAction(latlongToGridAction);
  connect(latlongToGridAction,SIGNAL(triggered(bool)),this,SLOT(latlongToGrid()));
  // Help menu
  aboutProgramAction=new QAction(this);
  //aboutProgramAction->setIcon(QIcon(":/.png"));
  aboutProgramAction->setText(tr("About ViewTIN"));
  helpMenu->addAction(aboutProgramAction);
  connect(aboutProgramAction,SIGNAL(triggered(bool)),this,SLOT(aboutProgram()));
  aboutQtAction=new QAction(this);
  //aboutQtAction->setIcon(QIcon(":/.png"));
  aboutQtAction->setText(tr("About Qt"));
  helpMenu->addAction(aboutQtAction);
  connect(aboutQtAction,SIGNAL(triggered(bool)),this,SLOT(aboutQt()));
  dumpAction=new QAction(this);
  //dumpAction->setIcon(QIcon(":/.png"));
  dumpAction->setText(tr("Dump")); // Dump is for debugging.
  //helpMenu->addAction(dumpAction); // In released versions, it is off the menu.
  connect(dumpAction,SIGNAL(triggered(bool)),canvas,SLOT(dump()));
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

void TinWindow::showFileLoaded(string fileName)
{
  if (fileName.length())
    fileName+=" — ";
  setWindowTitle(QString::fromStdString(fileName)+tr("ViewTIN"));
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

void TinWindow::changeButtonBits()
{
  buttonBitsChanged((curvyTriangleAction->isChecked()<<0)|
                    (curvyContourAction->isChecked()<<1));
}

void TinWindow::exportDxfTxt()
{
  int dialogResult;
  QStringList files;
  string fileName;
  double unit;
  //ThreadAction ta;
  fileDialog=new QFileDialog(this);
  fileDialog->setWindowTitle(tr("Export TIN and Contours as DXF Text"));
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->selectFile(QString::fromStdString(saveFileName+".dxf"));
  fileDialog->setNameFilter(tr("(*.dxf)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    unit=canvas->getDoc()->ms.toCoherent(1,LENGTH);
    writeDxf(fileName,canvas->getDoc()->pl[1],true,unit,0);
    //ta.param1=lengthUnit;
    //ta.filename=fileName;
    //ta.flags=exportEmpty+2*onlyInBoundary;
    //ta.opcode=ACT_WRITE_TIN;
    //enqueueAction(ta);
  }
  delete fileDialog;
  fileDialog=nullptr;
}

void TinWindow::exportDxfBin()
{
  int dialogResult;
  QStringList files;
  string fileName;
  double unit;
  //ThreadAction ta;
  fileDialog=new QFileDialog(this);
  fileDialog->setWindowTitle(tr("Export TIN and Contours as DXF Text"));
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->selectFile(QString::fromStdString(saveFileName+".dxf"));
  fileDialog->setNameFilter(tr("(*.dxf)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    unit=canvas->getDoc()->ms.toCoherent(1,LENGTH);
    writeDxf(fileName,canvas->getDoc()->pl[1],false,unit,0);
    //ta.param1=lengthUnit;
    //ta.filename=fileName;
    //ta.flags=exportEmpty+2*onlyInBoundary;
    //ta.opcode=ACT_WRITE_TIN;
    //enqueueAction(ta);
  }
  delete fileDialog;
  fileDialog=nullptr;
}

void TinWindow::exportTinTxt()
{
  int dialogResult;
  QStringList files;
  string fileName;
  double unit;
  //ThreadAction ta;
  fileDialog=new QFileDialog(this);
  fileDialog->setWindowTitle(tr("Export TIN as Text (AquaVeo)"));
  fileDialog->setFileMode(QFileDialog::AnyFile);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);
  fileDialog->selectFile(QString::fromStdString(saveFileName+".tin"));
  fileDialog->setNameFilter(tr("(*.tin)"));
  dialogResult=fileDialog->exec();
  if (dialogResult)
  {
    files=fileDialog->selectedFiles();
    fileName=files[0].toStdString();
    unit=canvas->getDoc()->ms.toCoherent(1,LENGTH);
    writeTinText(fileName,canvas->getDoc()->pl[1],unit,0);
    //ta.param1=lengthUnit;
    //ta.filename=fileName;
    //ta.flags=exportEmpty+2*onlyInBoundary;
    //ta.opcode=ACT_WRITE_TIN;
    //enqueueAction(ta);
  }
  delete fileDialog;
  fileDialog=nullptr;
}

void TinWindow::gridToLatlong()
{
  grDialog->setDoc(canvas->getDoc());
  grDialog->show();
  grDialog->raise();
  grDialog->activateWindow();
}

void TinWindow::latlongToGrid()
{
  llDialog->setDoc(canvas->getDoc());
  llDialog->show();
  llDialog->raise();
  llDialog->activateWindow();
}

void TinWindow::aboutProgram()
{
  QString progName=tr("ViewTIN, a Bezitopo program");
  QMessageBox::about(this,tr("ViewTIN"),
		     tr("%1\nVersion %2\nCopyright %3 Pierre Abbat\nLicense LGPL 3 or later")
		     .arg(progName).arg(QString(VERSION)).arg(COPY_YEAR));
}

void TinWindow::aboutQt()
{
  QMessageBox::aboutQt(this,tr("ViewTIN"));
}
