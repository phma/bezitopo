/******************************************************/
/*                                                    */
/* rootfind.h - root-finding methods                  */
/*                                                    */
/******************************************************/
/* Copyright 2016,2017 Pierre Abbat.
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

double invquad(double x0,double y0,double x1,double y1,double x2,double y2);

class brent
{
public:
  double init(double x0,double y0,double x1,double y1,bool intmode=false);
  double step(double y);
  bool finished()
  {
    return !((side&3)%3);
  }
  void setdebug(bool dbg)
  {
    debug=dbg;
  }
private:
  double a,fa,b,fb,c,fc,d,fd,x;
  int side;
  bool mflag,imode,debug;
  bool between(double s);
};
