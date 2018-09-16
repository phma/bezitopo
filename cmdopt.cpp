/******************************************************/
/*                                                    */
/* cmdopt.cpp - command-line options                  */
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
#include "cmdopt.h"
using namespace std;

void argpass1(int argc, char *argv[])
{
  int i,j;
  token tok;
  for (i=1;i<argc;i++)
  {
    tok.optnum=-1;
    tok.nonopt=argv[i];
    for (j=0;j<options.size();j++)
    {
      if (options[j].shopt && argv[i]==string("-")+options[j].shopt)
      {
	tok.optnum=j;
	tok.nonopt="";
	cmdline.push_back(tok);
      }
      else if (options[j].lopt.length() && argv[i]=="--"+options[j].lopt)
      {
	tok.optnum=j;
	tok.nonopt="";
	cmdline.push_back(tok);
      }
    }
    if (tok.optnum<0)
      cmdline.push_back(tok);
  }
}
