/******************************************************/
/*                                                    */
/* icommon.h - common interactive routines            */
/*                                                    */
/******************************************************/

#include <string>
#include "angle.h"

struct command
{
  std::string word;
  void (*fun)(std::string args);
  std::string desc;
  command(std::string w,void (*f)(std::string args),std::string d)
  {
    word=w;
    fun=f;
    desc=d;
  }
};

struct arangle // absolute or relative angle
{
  int ang;
  bool rel;
};

extern bool subcont; // continue flag within commands

std::string firstarg(std::string &args);
std::string trim(std::string word);
void setfoot_i(std::string args);
void setlengthunit_i(std::string args);
arangle parsearangle(std::string angstr,int unitp);
xy parsexy(std::string xystr);
void subexit(string args);
