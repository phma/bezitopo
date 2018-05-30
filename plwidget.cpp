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
#include "plwidget.h"
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
}
