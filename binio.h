/******************************************************/
/*                                                    */
/* binio.h - binary input/output                      */
/*                                                    */
/******************************************************/
#include <fstream>

void writebeshort(std::fstream &file,short i);
void writeleshort(std::fstream &file,short i);
short readbeshort(std::fstream &file);
short readleshort(std::fstream &file);
void writebeint(std::fstream &file,int i);
void writeleint(std::fstream &file,int i);
int readbeint(std::fstream &file);
int readleint(std::fstream &file);
void writebefloat(std::fstream &file,float f);
void writelefloat(std::fstream &file,float f);
float readbefloat(std::fstream &file);
float readlefloat(std::fstream &file);
void writebedouble(std::fstream &file,double f);
void writeledouble(std::fstream &file,double f);
double readbedouble(std::fstream &file);
double readledouble(std::fstream &file);
void writegeint(std::fstream &file,int i); // for Bezitopo's geoid files
int readgeint(std::fstream &file);
