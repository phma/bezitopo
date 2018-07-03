/******************************************************/
/*                                                    */
/* factordialog.cpp - scale factor dialog             */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include "factordialog.h"
#include "globals.h"
using namespace std;

LatlongFactorDialog::LatlongFactorDialog(QWidget *parent):QDialog(parent)
{
  latlongLabel=new QLabel(tr("Lat/Long"),this);
  latlongInput=new QLineEdit(this);
  elevationLabel=new QLabel(tr("Elevation"),this);
  elevationInput=new QLineEdit(this);
  plWidget=new ProjListWidget(this);
  gridLabel=new QLabel(tr("Grid"),this);
  gridOutput=new QLineEdit(this);
  okButton=new QPushButton(tr("OK"),this);
  cancelButton=new QPushButton(tr("Cancel"),this);
  gridLayout=new QGridLayout(this);
  validator=new LlValidator(this);
  setLayout(gridLayout);
  gridLayout->addWidget(latlongLabel,0,0);
  gridLayout->addWidget(latlongInput,0,1);
  gridLayout->addWidget(elevationLabel,1,0);
  gridLayout->addWidget(elevationInput,1,1);
  gridLayout->addWidget(plWidget,2,0,1,2);
  gridLayout->addWidget(gridLabel,3,0);
  gridLayout->addWidget(gridOutput,3,1);
  gridLayout->addWidget(okButton,4,0);
  gridLayout->addWidget(cancelButton,4,1);
  okButton->setEnabled(false);
  okButton->setDefault(true);
  plWidget->setProjectionList(allProjections);
  plWidget->setPoint(vball(0,xy(0,0)));
  projection=nullptr;
  latlongInput->setValidator(validator);
  connect(okButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
  connect(latlongInput,SIGNAL(textChanged(const QString)),this,SLOT(updateLocationStr(QString)));
  connect(latlongInput,SIGNAL(editingFinished()),this,SLOT(updateLocation()));
  connect(plWidget,SIGNAL(selectedProjectionChanged(Projection *)),this,SLOT(updateProjection(Projection *)));
}

void LatlongFactorDialog::accept()
{
  QDialog::accept();
}

void LatlongFactorDialog::setDoc(document *docu)
{
  doc=docu;
}

void LatlongFactorDialog::updateLocationStr(QString text)
{
  locationStr=text.toStdString();
}

void LatlongFactorDialog::updateLocation()
{
  location=parselatlong(locationStr,DEGREE);
  cout<<formatlatlong(location,DEGREE)<<endl;
  if (location.valid()==2)
    plWidget->setPoint(encodedir(Sphere.geoc(location,0)));
  else
    plWidget->setPoint(vball(0,xy(0,0)));
  updateOutput();
}

void LatlongFactorDialog::updateProjection(Projection *proj)
{
  projection=proj;
  updateOutput();
}

void LatlongFactorDialog::updateOutput()
{
  string gridStr;
  if (projection && location.valid()==2)
    gridCoords=projection->latlongToGrid(location);
  else
    gridCoords=xy(NAN,NAN);
  if (gridCoords.isfinite() && doc)
  {
    gridStr=doc->ms.formatMeasurement(gridCoords.east(),LENGTH)+' '+
	    doc->ms.formatMeasurementUnit(gridCoords.north(),LENGTH);
    gridOutput->setText(QString::fromStdString(gridStr));
  }
  else
    gridOutput->setText("");
}

