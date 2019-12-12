/******************************************************/
/*                                                    */
/* firstarg.cpp - first word of a string              */
/*                                                    */
/******************************************************/
/* Copyright 2019 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "firstarg.h"

using namespace std;

string firstarg(string &args)
{
  size_t pos;
  string ret;
  pos=args.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  args.erase(0,pos);
  pos=args.find(' ');
  ret=args.substr(0,pos);
  args.erase(0,pos);
  pos=args.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  args.erase(0,pos);
  return ret;
}

string trim(string word)
{
  size_t pos;
  pos=word.find_first_not_of(' ');
  if (pos==string::npos)
    pos=0;
  word.erase(0,pos);
  pos=word.find_last_not_of(' ');
  if (pos==string::npos)
    pos=0;
  else
    pos++;
  word.erase(pos,string::npos);
  return word;
}
