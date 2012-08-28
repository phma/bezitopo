/******************************************************/
/*                                                    */
/* vcurve.h - vertical curve arithmetic               */
/*                                                    */
/******************************************************/

#include <vector>

double vcurve(double a,double b,double c,double d,double x);
double vslope(double a,double b,double c,double d,double x);
double vaccel(double a,double b,double c,double d,double x);
double vlength(double a,double b,double c,double d,double hlength);
std::vector<double> vextrema(double a,double b,double c,double d);