/******************************************************/
/*                                                    */
/* icommon.h - common interactive routines            */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */

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
