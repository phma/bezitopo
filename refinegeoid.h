/******************************************************/
/*                                                    */
/* refinegeoid.h - refine geoid approximation         */
/*                                                    */
/******************************************************/
#include "geoid.h"

void outProgress();
void interroquad(geoquad &quad,double spacing);
void refine(geoquad &quad,double vscale,double tolerance,double sublimit,double spacing);
