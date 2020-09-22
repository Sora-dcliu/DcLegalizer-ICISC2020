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
};

class Col {
 public:
  Col(){};
  Col(int idx) : idx_(idx){};

  inline int idx() { return idx_; };
  inline map<int, Cluster>& Clusters() { return Clusters_; };

  long long InsertCol(shared_ptr<cell>& inst);  // Insert the cell and calculate the cost

  // Check if the new cell(old_ly) overlaps with the existing cluster
  map<int, Cluster>::iterator OverlapCluster(shared_ptr<cell>& inst);

  long long DeleteInst(shared_ptr<cell>& inst);  // Delete the cell and calculate the cost

  // Merge clusters and return the iterator of the changed cluster
  map<int, Cluster>::iterator collapse(map<int, Cluster>::iterator iter);
  void determindLoc();

 private:
  int idx_;
  map<int, Cluster> Clusters_;  // Sort by ly coordinate
  vector<int> changedCluster_;  // mark the changede cluster
};

class Legalize {
 public:
  Legalize();

  void doLegalize();
  void ColPlace(vector<shared_ptr<cell>>& insts);

  void reFind(vector<shared_ptr<cell>>& insts);

 private:
  vector<Col> COLS_;                      // columns
  vector<shared_ptr<cell>> sortedMacro_;  // According ly coordinate
  vector<shared_ptr<cell>> sortedStd_;    // According to ly coordinate

  // for iteratibr refind
  unordered_set<int> last_changedCols_;
  unordered_set<int> cur_changedCols;

  long long bestCost;
};

#endif