/******************************************************/
/*                                                    */
/* spiral.h - Cornu or Euler spirals                  */
/*                                                    */
/******************************************************/

#include "point.h"

xy cornu(double t);
xy cornu(double t,double curvature,double clothance);
double spiralbearing(double t,double curvature,double clothance);
int ispiralbearing(double t,double curvature,double clothance);
double spiralcurvature(double t,double curvature,double clothance);
