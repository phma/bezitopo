#include <map>

typedef std::map<int,point> ptlist;
extern ptlist points;
extern std::map<point*,int> revpoints;
void addpoint(int numb,point pnt,bool overwrite=false);
