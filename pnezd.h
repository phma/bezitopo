/******************************************************/
/*                                                    */
/* pnezd.h - file i/o in                              */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <vector>
#include <string>

std::vector<std::string> parsecsvline(std::string line);
std::string makecsvline(std::vector<std::string> words);
int readpnezd(std::string fname,bool overwrite=false);
int writepnezd(std::string fname);
