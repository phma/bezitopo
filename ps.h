/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/

extern FILE *psfile;
extern int orientation;
void psprolog();
void startpage();
void endpage();
void dot(xy pnt);
void line(edge lin,int num);
void pstrailer();
void psopen(const char * psfname);
void psclose();
void line2p(xy pnt1,xy pnt2);
void widen(double factor);
void setcolor(double r,double g,double b);
void setscale(double minx,double miny,double maxx,double maxy);
