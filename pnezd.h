/******************************************************/
/*                                                    */
/* pnezd.h - file i/o in                              */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <vector>
#include <string>

std::vector<std::string> csvline(string line);
int readpnezd(std::string fname,bool overwrite=false);
