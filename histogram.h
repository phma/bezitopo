/******************************************************/
/*                                                    */
/* histogram.h - adaptive histogram                   */
/*                                                    */
/******************************************************/
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
  unsigned total;
  void split(int n);
public:
  histogram();
  histogram(double least,double most);
  void clear(); // leaves least and most intact, makes a single empty bin
  void clear(double least,double most);
  void clearcount(); // leaves bin widths intact, just clears all their counts
  int find(double val);
  histogram& operator<<(double val);
  unsigned nbars();
  histobar getbar(unsigned n);
};
