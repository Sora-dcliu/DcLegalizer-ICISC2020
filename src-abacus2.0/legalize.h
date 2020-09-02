#ifndef _LEGALIZE_H
#define _LEGALIZE_H

#include <map>
#include <vector>

#include "global.h"

using namespace std;

class Cluster {
 public:
  Cluster(shared_ptr<cell>& inst, int idx, int lx);

  inline int idx() { return idx_; };
  inline int lx() {return lx_;};
  inline int ly() { return ly_; };
  inline int uy() { return ly_ + totalHeight_; };
  inline void setLy(int y) { ly_ = y; };
  inline int totalHeight() { return totalHeight_; };
  inline int Qc() { return Qc_; };

  inline const vector<shared_ptr<cell>>& cells() { return cells_; };

  void addCell(shared_ptr<cell>& inst);
  void addCluster(Cluster& c);

  void setCellsOrder(vector<shared_ptr<cell>>& newCells) {
    cells_.assign(newCells.begin(), newCells.end());
  };
  long long getInsertCost();
  long long getTotalCost();
  void setLocations();

 private:
  int idx_;
  vector<shared_ptr<cell>> cells_;
  int lx_, ly_;      // Optimal position(lower left corner)
  int totalHeight_;  // total height as same as Ec
  int Qc_;  // h(1) * y'(1) + sum_{ h(1) * [y'(i) - sum_h(k)]} i: 1 ~ N k: 1 ~
            // i-1
};

class Col {
 public:
  Col(){};
  Col(int idx) : idx_(idx){};

  inline int idx() { return idx_; };
  inline vector<Cluster>& Clusters() { return Clusters_; };

  long long PlaceCol(shared_ptr<cell>& inst);
  void collapse(Cluster& c);

 private:
  int idx_;

  vector<Cluster> Clusters_;
};

class Legalize {
 public:
  Legalize();

  void doLegalize();
  void refinement();

  long long getTotalCost();

 private:
  vector<Col> COLS_;
  multimap<int, shared_ptr<cell>> sortedCells;  // sorted in y order
};

#endif