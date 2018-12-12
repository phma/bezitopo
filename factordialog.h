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
#include "plwidget.h"
#include "llvalidator.h"

class GeoLocalization
/* This localizes a survey on the earth. It has nothing to do with
 * internationalization.
 * This is a stub and will have more members later.
 */
{
public:
  Projection *proj;
  latlong fixedLl;
  xy fixedXy;
  double elev;
  double combFactor;
};

class LatlongFactorDialog: public QDialog
{
  Q_OBJECT
public:
  LatlongFactorDialog(QWidget *parent=0);
  virtual QSize sizeHint() const;
signals:
  void basePointChanged(GeoLocalization local);
public slots:
  virtual void accept();
  void setDoc(document *docu);
  void updateLocationStr(QString text);
  void updateLocation();
  void updateElevationStr(QString text);
  void updateElevation();
  void updateProjection(Projection *proj);
  void updateOutput();
private:
  QLabel *latlongLabel;
  QLineEdit *latlongInput;
  QLabel *elevationLabel;
  QLineEdit *elevationInput;
  ProjListWidget *plWidget;
  std::string locationStr,elevationStr,separationStr,elevFactorStr,gridFactorStr,combFactorStr;
  document *doc;
  latlong location;
  Projection *projection;
  xy gridCoords;
  double separation,elevation,radius,elevfactor,gridfactor;
  QLabel *gridLabel;
  QLineEdit *gridOutput; // "grid" means a conformal coordinate system
  QLabel *separationLabel;
  QLineEdit *separationOutput;
  QLabel *elevFactorLabel;
  QLineEdit *elevFactorOutput;
  QLabel *gridFactorLabel;
  QLineEdit *gridFactorOutput;
  QLabel *combFactorLabel;
  QLineEdit *combFactorOutput;
  QPushButton *okButton,*cancelButton;
  QGridLayout *gridLayout; // "grid" means a lattice arrangement of widgets
  LlValidator *validator;
};

class GridFactorDialog: public QDialog
{
  Q_OBJECT
public:
  GridFactorDialog(QWidget *parent=0);
  virtual QSize sizeHint() const;
signals:
  void basePointChanged(GeoLocalization local);
public slots:
  virtual void accept();
  void setDoc(document *docu);
  void updateGridCoordsStr(QString text);
  void updateGridCoords();
  void updateElevationStr(QString text);
  void updateElevation();
  void updateProjection(Projection *proj);
  void updateOutput();
private:
  QLabel *gridLabel;
  QLineEdit *gridInput; // "grid" means a conformal coordinate system
  QLabel *elevationLabel;
  QLineEdit *elevationInput;
  ProjListWidget *plWidget;
  std::string gridCoordsStr,elevationStr,separationStr,elevFactorStr,gridFactorStr,combFactorStr;
  document *doc;
  latlong location;
  Projection *projection;
  xy gridCoords;
  double separation,elevation,radius,elevfactor,gridfactor;
  QLabel *latlongLabel;
  QLineEdit *latlongOutput;
  QLabel *separationLabel;
  QLineEdit *separationOutput;
  QLabel *elevFactorLabel;
  QLineEdit *elevFactorOutput;
  QLabel *gridFactorLabel;
  QLineEdit *gridFactorOutput;
  QLabel *combFactorLabel;
  QLineEdit *combFactorOutput;
  QPushButton *okButton,*cancelButton;
  QGridLayout *gridLayout; // "grid" means a lattice arrangement of widgets
};
#endif
