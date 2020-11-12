#ifndef _BIPARTITE_H
#define _BIPARTITE_H

#include <set>

#include "global.h"

using namespace std;

class Bin {
 public:
  Bin(int x, int y) : x_(x), y_(y), visited_(false){};
  inline shared_ptr<cell>& Inst() { return inst_; };
  inline void setInst(shared_ptr<cell>& inst) { inst_ = inst; };
  inline void deleteInst() {
    inst_.reset();
    visited_ = false;
  };
  inline long long getCost(shared_ptr<cell>& inst) { return inst->getCost(x_ * 8, y_); };

  inline bool isStd() {
    if (inst_)
      return !inst_->isMacro();
    else
      return false;
  };
  inline bool isMacro() {
    if (inst_)
      return inst_->isMacro();
    else
      return false;
  }

  inline void mark() { visited_ = true; };
  inline void unmark() { visited_ = false; };
  inline bool visited() { return visited_; };

 private:
  int x_, y_;
  shared_ptr<cell> inst_;
  bool visited_;
};

// Bipartite Graph Match
class BGM {
 public:
  BGM();
  void doBipartiteGraphMatch();

  // For KM algorithm
  void dfs(int x, int y, set<shared_ptr<cell>>& insts);
  void Match(set<shared_ptr<cell>>& insts);
  long long MoveMacro(int idx, int type);  // 1-top->down 2-down->top 3-left->right 4-right->left
  long long KM();
  bool findPath(int u);

  void initKM();
  void buildMap();
  void setLocation();
  void cleanKM();

 private:
  int max_x_, max_y_;
  vector<vector<Bin>> Grid;
  // parameters for KM algorithm
  int lx, ly, ux, uy;                  // The coordinate of bbx
  int cnt_inst, cnt_bin;               // The number of points
  vector<shared_ptr<cell>> macros;     // The list of macros
  vector<shared_ptr<cell>> vec_insts;  // The list of std_cells
  vector<long long> w_inst, w_bin;     // The top value of each point
  vector<int> c_inst, c_bin;           // The points that each point matches
  vector<bool> vis_inst, vis_bin;      // Whether each point enters the augmented path
  vector<vector<long long>> Map;       // Edge weights
  long long minz;                      // The smallest difference
};

#endif