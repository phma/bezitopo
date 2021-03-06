/******************************************************/
/*                                                    */
/* rawdata.h - raw data of traverse or GPS            */
/*                                                    */
/******************************************************/
/* Copyright 2020 Pierre Abbat.
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

#include <vector>
#include <string>

class RawField
{
public:
  std::string fieldString;
  union
  {
    double fieldReal;
    int64_t fieldInt;
  };
  int tag;
};

/* Tags are 2 bytes with the high nybble meaning the following:
 * 0	1 byte integer
 * 1	2 byte integer
 * 2	4 byte integer
 * 3	8 byte integer
 * 4	double
 * 5	string
 */

class RawRecord
{
public:
  int recType;
  std::vector<RawField> fields;
};
