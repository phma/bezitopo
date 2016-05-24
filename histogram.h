/******************************************************/
/*                                                    */
/* histogram.h - streaming histogram                  */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include <vector>

struct histobar
{
  double start,end;
  unsigned count;
};

class histogram
{
private:
  std::vector<double> bin;
  std::vector<unsigned> count;
  double discrete;
  unsigned total;
  void split(int n);
public:
  histogram();
  histogram(double least,double most);
  void setdiscrete(double d);
  void clear(); // leaves least and most intact, makes a single empty bin
  void clear(double least,double most);
  void clearcount(); // leaves bin widths intact, just clears all their counts
  int find(double val);
  histogram& operator<<(double val);
  unsigned nbars();
  histobar getbar(unsigned n);
  unsigned gettotal();
  void dump();
};
#endif
