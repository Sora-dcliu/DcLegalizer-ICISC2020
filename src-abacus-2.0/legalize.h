#ifndef _LEGALIZE_H
#define _LEGALIZE_H

#include <map>
#include <vector>

using namespace std;

class Col;

class cell {
 public:
  cell(int idx, int height, int lx, int ly)
      : idx_(idx),
        height_(height),
        old_lx_(lx),
        lx_(lx),
        old_ly_(ly),
        ly_(ly){};

  int oldlx() { return old_lx_; };
  int oldly() { return old_ly_; };

  int lx() { return lx_; };
  int ux() { return lx_ + 8; };
  int ly() { return ly_; };
  int uy() { return ly_ + height_; };
  int height() { return height_; };

  int idx() { return idx_; };

  void setLoc(int lx, int ly) {
    lx_ = lx;
    ly_ = ly;
  };

  // for replace insert model colPlace
  map<long long, Col>& candidateCOLS() { return candidateCOLS_; };
  void addCandiCol(long long cost, Col& col);

 private:
  // for normal model abacus
  int idx_;
  int old_lx_, old_ly_;
  int lx_, ly_;
  int height_;

  // for insert model colPlace
  multimap<long long, Col> candidateCOLS_;
};

class Cluster {
 public:
  Cluster(shared_ptr<cell>& inst, int idx);

  int idx() { return idx_; };
  int ly() { return ly_; };
  int uy() { return ly_ + totalHeight_; };
  void setLy(int y) { ly_ = y; };
  int totalHeight() { return totalHeight_; };
  int Qc() { return Qc_; };

  vector<shared_ptr<cell>>& cells() { return cells_; };

  void addCell(shared_ptr<cell>& inst);
  void addCluster(Cluster& c);

 private:
  int idx_;
  vector<shared_ptr<cell>> cells_;
  int ly_;           // Optimal position(lower left corner)
  int totalHeight_;  // total height as same as Ec
  int Qc_;  // h(1) * y'(1) + sum_{ h(1) * [y'(i) - sum_h(k)]} i: 1 ~ N k: 1 ~
            // i-1
};

class Col {
 public:
  Col(){};
  Col(int idx) : idx_(idx){};

  int idx() { return idx_; };
  vector<Cluster>& Clusters() { return Clusters_; };

  long long PlaceCol(shared_ptr<cell>& inst);
  void collapse(Cluster& c);

  // for insert model colPlace
  shared_ptr<cell>& lastInsertCell() { return lastInsertCell_; };
  void setLastInsetCell(shared_ptr<cell>& inst) { lastInsertCell_ = inst; };

 private:
  int idx_;

  vector<Cluster> Clusters_;

  // for insert model colPlace
  shared_ptr<cell> lastInsertCell_;
};

class Legalize {
 public:
  Legalize();

  void doLegalize();
  void searchBestColPlace(shared_ptr<cell>& inst);
  void normalColPlace(shared_ptr<cell>& inst);
  vector<Col>& replaceColPlace(shared_ptr<cell>& inst);

 private:
  vector<Col> COLS_;
  multimap<int, shared_ptr<cell>> sortedCells;  // sorted in y order
};

#endif