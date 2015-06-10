/******************************************************/
/*                                                    */
/* pnezd.h - file i/o in                              */
/* point-northing-easting-z-description format        */
/*                                                    */
/******************************************************/

#include <vector>
#include <string>

class document;

std::vector<std::string> parsecsvline(std::string line);
std::string makecsvline(std::vector<std::string> words);
int readpnezd(document *doc,std::string fname,bool overwrite=false);
int writepnezd(document *doc,std::string fname);
