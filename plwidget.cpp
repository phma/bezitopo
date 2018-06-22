/******************************************************/
/*                                                    */
/* plwidget.cpp - projection list widget              */
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
#include <QString>
#include "plwidget.h"
#include "projection.h"
using namespace std;

ProjListWidget::ProjListWidget(QWidget *parent):QWidget(parent)
{
  countryBox=new QComboBox(this);
  provinceBox=new QComboBox(this);
  zoneBox=new QComboBox(this);
  versionBox=new QComboBox(this);
  gridLayout=new QGridLayout(this);
  setLayout(gridLayout);
  gridLayout->addWidget(countryBox,0,0);
  gridLayout->addWidget(provinceBox,0,1);
  gridLayout->addWidget(zoneBox,0,2);
  gridLayout->addWidget(versionBox,0,3);
}

void ProjListWidget::setProjectionList(ProjectionList pl)
{
  allProjections=pl;
}

void ProjListWidget::setPoint(vball v)
{
  point=v;
  containingProjections=allProjections.cover(v);
  cout<<allProjections.size()<<" projections total\n";
  cout<<containingProjections.size()<<" projections contain the point\n";
  updateComboBoxes();
}

void ProjListWidget::updateComboBoxes()
{
  ProjectionLabel allSet,anyCountry,anyProvince,anyZone,anyVersion;
  vector<string> stringList;
  int i;
  allSet.country=countryBox->currentText().toStdString();
  allSet.province=provinceBox->currentText().toStdString();
  allSet.zone=zoneBox->currentText().toStdString();
  allSet.version=versionBox->currentText().toStdString();
  if (countryBox->currentIndex()<1)
    allSet.country="";
  if (provinceBox->currentIndex()<1)
    allSet.province="";
  if (zoneBox->currentIndex()<1)
    allSet.zone="";
  if (versionBox->currentIndex()<1)
    allSet.version="";
  anyProvince.country=anyZone.country=anyVersion.country=allSet.country;
  anyZone.province=anyVersion.province=anyCountry.province=allSet.province;
  anyVersion.zone=anyCountry.zone=anyProvince.zone=allSet.zone;
  anyCountry.version=anyProvince.version=anyZone.version=allSet.version;
  stringList=containingProjections.matches(anyCountry).listCountries();
  countryBox->clear();
  countryBox->addItem(QString("—"));
  for (i=0;i<stringList.size();i++)
    countryBox->addItem(QString::fromStdString(stringList[i]));
  countryBox->setCurrentText(QString::fromStdString(allSet.country));
  stringList=containingProjections.matches(anyProvince).listProvinces();
  provinceBox->clear();
  provinceBox->addItem(QString("—"));
  for (i=0;i<stringList.size();i++)
    provinceBox->addItem(QString::fromStdString(stringList[i]));
  provinceBox->setCurrentText(QString::fromStdString(allSet.province));
  stringList=containingProjections.matches(anyZone).listZones();
  zoneBox->clear();
  zoneBox->addItem(QString("—"));
  for (i=0;i<stringList.size();i++)
    zoneBox->addItem(QString::fromStdString(stringList[i]));
  zoneBox->setCurrentText(QString::fromStdString(allSet.zone));
  stringList=containingProjections.matches(anyVersion).listVersions();
  versionBox->clear();
  versionBox->addItem(QString("—"));
  for (i=0;i<stringList.size();i++)
    versionBox->addItem(QString::fromStdString(stringList[i]));
  versionBox->setCurrentText(QString::fromStdString(allSet.version));
}
