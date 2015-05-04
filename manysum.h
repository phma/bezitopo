/******************************************************/
/*                                                    */
/* manysum.h - add many numbers                       */
/*                                                    */
/******************************************************/
#include <map>
#include <cmath>
/* Adds together many numbers (like millions) accurately.
 * Each number is put in a bucket numbered by its exponent returned by frexp().
 * If the sum of the number and what's in the bucket is too big to fit
 * in the bucket, it is put into the next bucket up. If the sum is too small,
 * it is not put into the next bucket down, as it has insignificant bits
 * at the low end. This can happen only when adding numbers of opposite sign,
 * which does not happen when computing volumes, as positive and negative
 * volumes are added separately so that compaction of dirt can be computed.
 */

class manysum
{
private:
  std::map<int,double> bucket;
  static int cnt;
public:
  void clear();
  void prune();
  double total();
  void dump();
  manysum& operator+=(double x);
};
