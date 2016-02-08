/******************************************************/
/*                                                    */
/* binio.h - binary input/output                      */
/*                                                    */
/******************************************************/
#include <fstream>
#include <string>

void writebeshort(std::ostream &file,short i);
void writeleshort(std::ostream &file,short i);
short readbeshort(std::fstream &file);
short readleshort(std::fstream &file);
void writebeint(std::ostream &file,int i);
void writeleint(std::ostream &file,int i);
int readbeint(std::fstream &file);
int readleint(std::fstream &file);
void writebefloat(std::fstream &file,float f);
void writelefloat(std::fstream &file,float f);
float readbefloat(std::fstream &file);
float readlefloat(std::fstream &file);
void writebedouble(std::ostream &file,double f);
void writeledouble(std::ostream &file,double f);
double readbedouble(std::fstream &file);
double readledouble(std::fstream &file);
void writegeint(std::ostream &file,int i); // for Bezitopo's geoid files
int readgeint(std::fstream &file);
void writeustring(std::ostream &file,std::string s);

