/******************************************************/
/*                                                    */
/* binio.h - binary input/output                      */
/*                                                    */
/******************************************************/
#include <fstream>
#include <string>

void writebeshort(std::ostream &file,short i);
void writeleshort(std::ostream &file,short i);
short readbeshort(std::istream &file);
short readleshort(std::istream &file);
void writebeint(std::ostream &file,int i);
void writeleint(std::ostream &file,int i);
int readbeint(std::istream &file);
int readleint(std::istream &file);
void writebefloat(std::fstream &file,float f);
void writelefloat(std::fstream &file,float f);
float readbefloat(std::fstream &file);
float readlefloat(std::fstream &file);
void writebedouble(std::ostream &file,double f);
void writeledouble(std::ostream &file,double f);
double readbedouble(std::istream &file);
double readledouble(std::istream &file);
void writegeint(std::ostream &file,int i); // for Bezitopo's geoid files
int readgeint(std::istream &file);
void writeustring(std::ostream &file,std::string s);
std::string readustring(std::istream &file);

