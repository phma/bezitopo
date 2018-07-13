/******************************************************/
/*                                                    */
/* textfile.cpp - read text files opened in binary    */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include "textfile.h"
using namespace std;

TextFile::TextFile(istream &fil)
{
  file=&fil;
  lineend=-2;
}

string TextFile::getline()
{
  string ret;
  int ch;
  bool done=false;
  while (file->good() && !done)
  {
    ch=file->get();
    if (ch==lineend)
      done=true;
    else if (ch=='\n' || ch=='\r')
      if (lineend<0)
      {
	lineend=ch;
	done=true;
      }
      else;
    else
      ret+=ch;
  }
  return ret;
}

bool TextFile::good()
{
  return file->good();
}
