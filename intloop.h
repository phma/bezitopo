/******************************************************/
/*                                                    */
/* intloop.h - loops of integers (point numbers)      */
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
#ifndef INTLOOP_H
#define INTLOOP_H
#include <vector>
#include <array>

int inv2adic(int n);

class int1loop
{
private:
  std::vector<int> bdy;
public:
  bool isempty();
  int size();
  void clear();
  void push_back(int x);
  int operator[](int n);
  bool hasMember(int n);
  std::array<int,2> seg(int n);
  std::vector<int> nullSegments();
  void positionSegment(int n);
  void reverse();
  void splice(int1loop &b);
  void split(int n,int1loop &b);
  void splice(int m,int1loop &b,int n);
  void split(int m,int n,int1loop &b);
  void deleteRetrace();
  friend bool operator==(const int1loop l,const int1loop r);
};

class intloop
{
private:
  std::vector<int1loop> bdy;
  int segNum;
public:
  intloop();
  void push_back(int1loop g1);
  int1loop operator[](int n);
  int size() const;
  int totalSegments();
  std::array<int,4> seg(int n);
  std::array<int,4> someSeg();
  std::array<int,4> dupSeg();
  std::array<int,4> pinchPoint();
  void clear();
  void deleteRetrace();
  void deleteNullSegments();
  void deleteEmpty();
  void consolidate();
  void erase(int n);
  friend intloop operator+(const intloop &l,const intloop &r);
};
#endif
