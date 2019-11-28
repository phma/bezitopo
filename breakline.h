/******************************************************/
/*                                                    */
/* breakline.h - breaklines                           */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
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
#ifndef BREAKLINE_H
#define BREAKLINE_H
#include <vector>
#include <array>
#include <iostream>
/* Bezitopo has two types of breaklines. A type-0 breakline is a sequence
 * of point numbers which are forced to be adjacent in the TIN. A type-1
 * breakline is a polyline which crosses some edges in the TIN and makes
 * the computation of the slope at one end of the edge ignore the slope
 * at the other end.
 */

class Breakline0
{
public:
  Breakline0();
  Breakline0(int a,int b);
  Breakline0(std::vector<std::string> numbers); // can throw
  bool isEmpty();
  bool isOpen();
  void normalize();
  int lowEnd();
  int highEnd();
  int size(); // number of segments, not nodes
  void reverse();
  Breakline0& operator<<(int endp);
  std::array<int,2> operator[](int n);
  friend bool jungible(Breakline0 &a,Breakline0 &b);
  friend Breakline0 operator+(Breakline0 &a,Breakline0 &b);
  void writeText(std::ostream &ofile);
  void writeXml(std::ostream &ofile);
private:
  std::vector<int> nodes;
};

std::vector<std::string> parseBreakline(std::string line,char delim);
#endif
