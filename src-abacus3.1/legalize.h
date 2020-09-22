#ifndef _LEGALIZE_H
#define _LEGALIZE_H

#include <map>
#include <unordered_set>
#include <vector>

#include "global.h"

using namespace std;

class Cluster {
 public:
  Cluster(shared_ptr<cell>& inst, int lx);

  inline int lx() { return lx_; };
  inline int ly() { return ly_; };
  inline int uy() { return ly_ + totalHeight_; };
  inline void setLy(int y) { ly_ = y; };
  inline int totalHeight() { return totalHeight_; };
  inline int Qc() { return Qc_; };

  inline const vector<shared_ptr<cell>>& cells() { return cells_; };

  inline void addCell(shared_ptr<cell>& inst);
  inline void addCluster(Cluster& c);

  inline bool isOverlap(shared_ptr<cell>& inst) {
    if (ly_ < inst->oldly() + inst->height() && uy() > inst->oldly())
      return true;
    else
      return false;
  };

  inline bool isOverlap(Cluster& c) {
    if (ly_ < c.uy() && uy() > c.ly())
      return true;
    else
      return false;
  };

  inline bool isChange() { return changed; };

  void setCellsOrder(vector<shared_ptr<cell>>& newCells) {
    cells_.assign(newCells.begin(), newCells.end());
  };
  long long getInsertCost(shared_ptr<cell>& newInst, bool calcCost);
  long long getTotalCost();
  void setLocations();

 private:
  vector<shared_ptr<cell>> cells_;
  int lx_, ly_;      // Optimal position(lower left corner)
  int totalHeight_;  // total height as same as Ec
  int Qc_;           // h(1) * y'(1) + sum_{ h(1) * [y'(i) - sum_h(k)]} i: 1 ~ N k: 1 ~
                     // i-1
  bool changed;      // mark when changes occur
};

class Col {
 public:
  Col(){};
  Col(int idx) : idx_(idx){};

  inline int idx() { return idx_; };
  inline vector<Cluster>& Clusters() { return Clusters_; };

  long long InsertCol(shared_ptr<cell>& inst);  // Insert the cell and calculate the cost

  // Check if the new cell(old_ly) overlaps with the existing cluster
  int OverlapCluster(shared_ptr<cell>& inst);

  long long DeleteInst(shared_ptr<cell>& inst);  // Delete the cell and calculate the cost

  // Merge clusters and return the iterator of the changed cluster
  int collapse(int c_idx);
  int insert_newCluster(Cluster& c);
  void determindLoc();

 private:
  int idx_;
  vector<Cluster> Clusters_;
};

class Legalize {
 public:
  Legalize();

  void doLegalize();
  void ColPlace();

  void reFind();

 private:
  vector<Col> COLS_;                      // columns
  vector<shared_ptr<cell>> sortedCells_;  // According ly coordinate

  // for iteratibr refind
  unordered_set<int> last_changedCols_;
  unordered_set<int> cur_changedCols;

  long long bestCost;
};

#endif