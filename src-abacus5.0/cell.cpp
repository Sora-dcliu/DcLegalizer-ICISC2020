#include "cell.h"

#include <cmath>

#include "global.h"

bool bufferMod = false;

int cell::oldlx() { return old_lx_; };
int cell::oldly() { return old_ly_; };

int cell::lx() {
  if (!bufferMod)
    return lx_;
  else
    return buff_lx_;
}
int cell::ux() {
  if (!bufferMod)
    return lx_ + 8;
  else
    return buff_lx_ + 8;
}
int cell::ly() {
  if (!bufferMod)
    return ly_;
  else
    return buff_ly_;
}
int cell::uy() {
  if (!bufferMod)
    return ly_ + height_;
  else
    return buff_ly_ + height_;
}
int cell::height() { return height_; }

int cell::Key() { return 2 * old_ly_ + height_; }

bool cell::isMacro() { return height_ > 1; }

int cell::idx() { return idx_; }

void cell::setLoc(int lx, int ly) {
  if (!bufferMod) {
    lx_ = lx;
    ly_ = ly;
  }
  buff_lx_ = lx;
  buff_ly_ = ly;
}

void cell::clearBuffer(bool saveBuff) {
  if (saveBuff) {
    lx_ = buff_lx_;
    ly_ = buff_ly_;
  } else {
    buff_lx_ = lx_;
    buff_ly_ = ly_;
  }
}

long long cell::getCost(int lx, int ly) {
  return height_ * (pow(old_lx_ - lx, 2) + pow(old_ly_ - ly, 2));
}

long long cell::getCost() {
  int lx, ly;
  if (bufferMod) {
    lx = buff_lx_;
    ly = buff_ly_;
  } else {
    lx = lx_;
    ly = ly_;
  }
  return height_ * (pow(old_lx_ - lx, 2) + pow(old_ly_ - ly, 2));
}