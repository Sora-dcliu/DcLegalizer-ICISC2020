#ifndef _CELL_H_
#define _CELL_H_

#include <cmath>

using namespace std;

class cell {
 public:
  cell(int idx, int height, int lx, int ly)
      : idx_(idx),
        height_(height),
        old_lx_(lx),
        lx_(lx),
        old_ly_(ly),
        ly_(ly),
        buff_lx_(lx),
        buff_ly_(ly){};

  int oldlx();
  int oldly();

  int lx();
  int ux();
  int ly();
  int uy();
  int height();

  int Key();

  bool isMacro();

  int idx();

  void setLoc(int lx, int ly);

  void clearBuffer(bool saveBuff);

  long long getCost(int lx, int ly);
  long long getCost();

 private:
  int idx_;
  int old_lx_, old_ly_;
  int lx_, ly_;
  int buff_lx_, buff_ly_;
  int height_;
};

#endif