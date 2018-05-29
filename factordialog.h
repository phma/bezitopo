/******************************************************/
/*                                                    */
/* factordialog.h - scale factor dialog               */
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
#ifndef FACTORDIALOG_H
#define FACTORDIALOG_H
#include <vector>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include "projection.h"

class LatlongFactorDialog: public QDialog
{
  Q_OBJECT
public:
  LatlongFactorDialog(QWidget *parent=0);
signals:
public slots:
  virtual void accept();
private:
  QLabel *latlongLabel;
  QLineEdit *latlongInput;
  QLabel *elevationLabel;
  QLineEdit *elevationInput;
  QLabel *gridLabel;
  QLineEdit *gridOutput; // "grid" means a conformal coordinate system
  QPushButton *okButton,*cancelButton;
  QGridLayout *gridLayout; // "grid" means a lattice arrangement of widgets
};
#endif
