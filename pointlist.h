#include <map>
#include <string>
#include <vector>

typedef std::map<int,point> ptlist;
typedef std::map<point*,int> revptlist;
class pointlist
{
public:
  ptlist points;
  revptlist revpoints;
  void addpoint(int numb,point pnt,bool overwrite=false);
  void clear();
};
struct criterion
{
  string str;
  bool istopo;
};

typedef std::vector<criterion> criteria;

extern pointlist surveypoints,topopoints;

void copytopopoints(criteria crit);
