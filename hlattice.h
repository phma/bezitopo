/******************************************************/
/*                                                    */
/* hlattice.h - hexagonal lattice                     */
/*                                                    */
/******************************************************/
/* Hexagonal vector (Eisenstein or Euler integers) is used to scan the area
 * of a geoid file being converted for small islands.
 * The largest hlattice used in this program has a size of 26754
 * and 2147409811 points. The smallest spacing of the lattice is therefore
 * 238 meters when applied to the largest square.
 */
#ifndef HVEC_H
#define HVEC_H

#include <cmath>
#include <cstdlib>
#include <map>
#include <complex>
#define M_SQRT_3_4 0.86602540378443864676372317
// The continued fraction expansion is 0;1,6,2,6,2,6,2,...
#define M_SQRT_3 1.73205080756887729352744634
#define M_SQRT_1_3 0.5773502691896257645091487805

//PAGERAD should be 1 or 6 mod 8, which makes PAGESIZE 7 mod 8.
// The maximum is 147 because of the file format.
#define PAGERAD 6
#define PAGESIZE (PAGERAD*(PAGERAD+1)*3+1)

class hvec
{
private:
  int x,y; // x is the real part, y is at 120°
public:
  hvec()
  {x=y=0;}
  hvec(int xa)
  {x=xa;
   y=0;
   }
  hvec(int xa,int ya)
  {x=xa;
   y=ya;
   }
  hvec operator+(hvec b);
  hvec operator-();
  hvec operator-(hvec b);
  bool operator==(hvec b);
  bool operator!=(hvec b);
  unsigned long norm();
  int pageinx(int size,int nelts);
  int pageinx();
  int getx()
  {return x;
   }
  int gety()
  {return y;
   }
  operator std::complex<double>() const
  {
    return std::complex<double>(x-y/2.,y*M_SQRT_3_4);
  }
};

hvec start(int n);
hvec nthhvec(int n,int size,int nelts);

class hlattice
{
public:
  std::map<int,hvec> rightedge;
  int nelts;
  hlattice(int size);
  hvec nthhvec(int n);
};

#endif
