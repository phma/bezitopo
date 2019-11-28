/******************************************************/
/*                                                    */
/* textfile.h - read text files opened in binary      */
/*                                                    */
/******************************************************/
/* Copyright 2018 Pierre Abbat.
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
#include <fstream>
#include <string>

class TextFile
/* Reads a text file opened in binary mode. This is necessary because text
 * DXF files written by ARES on Linux use CRLF line terminators, which is
 * not normal on Linux.
 */
{
private:
  std::istream *file;
  int lineend;
public:
  TextFile(std::istream &fil);
  std::string getline();
  bool good();
};
