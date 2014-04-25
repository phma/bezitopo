/******************************************************/
/*                                                    */
/* random.h - random numbers                          */
/*                                                    */
/******************************************************/

class randm
{
public:
  randm();
  unsigned short usrandom();
  unsigned char ucrandom();
  ~randm();
private:
  FILE *randfil;
};

extern randm rng;