#ifndef _CELL_H_
#define _CELL_H_

#include <cmath>

using namespace std;

class cell {
 public:
  cell(int idx, int height, int lx, int ly)
      : idx_(idx), height_(height), old_lx_(lx), lx_(lx), old_ly_(ly), ly_(ly){};

  inline int oldlx() { return old_lx_; };
  inline int oldly() { return old_ly_; };

  inline int lx() { return lx_; };
  inline int ux() { return lx_ + 8; };
  inline int ly() { return ly_; };
  inline int uy() { return ly_ + height_; };
  inline int height() { return height_; };
  inline bool isMacro() {return height_ > 1;};

  inline int idx() { return idx_; };

  inline void setLoc(int lx, int ly) {
    lx_ = lx;
    ly_ = ly;
  };
  inline long long getCost(int lx, int ly) {
    return height_ * (pow(old_lx_ - lx, 2) + pow(old_ly_ - ly, 2));
  };
  inline long long getCost() {
    return height_ * (pow(old_lx_ - lx_, 2) + pow(old_ly_ - ly_, 2));
  };

 private:
  int idx_;
  int old_lx_, old_ly_;
  int lx_, ly_;
  int height_;
};

#endif