/******************************************************/
/*                                                    */
/* binio.h - binary input/output                      */
/*                                                    */
/******************************************************/
#include <fstream>

void writebeshort(std::ofstream &file,short i);
void writeleshort(std::ofstream &file,short i);
short readbeshort(std::ifstream &file);
short readleshort(std::ifstream &file);
void writebeint(std::ofstream &file,int i);
void writeleint(std::ofstream &file,int i);
int readbeint(std::ifstream &file);
int readleint(std::ifstream &file);
void writebefloat(std::ofstream &file,float f);
void writelefloat(std::ofstream &file,float f);
float readbefloat(std::ifstream &file);
float readlefloat(std::ifstream &file);
void writebedouble(std::ofstream &file,double f);
void writeledouble(std::ofstream &file,double f);
double readbedouble(std::ifstream &file);
double readledouble(std::ifstream &file);
void writegeint(std::fstream &file,int i); // for Bezitopo's geoid files
int readgeint(std::fstream &file);
