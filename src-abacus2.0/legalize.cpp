#include "legalize.h"

#include <climits>
#include <cmath>

#define LOG cout << "[LG] "

Cluster::Cluster(shared_ptr<cell>& inst, int idx, int lx) {
  this->idx_ = idx;
  this->cells_.push_back(inst);
  this->lx_ = lx;
  this->ly_ = inst->oldly();
  this->Qc_ = inst->height() * (inst->oldly());
  this->totalHeight_ = inst->height();
}

void Cluster::addCell(shared_ptr<cell>& inst) {
  this->cells_.push_back(inst);
  this->Qc_ += inst->height() * (inst->oldly() - this->totalHeight_);
  this->totalHeight_ += inst->height();
}

void Cluster::addCluster(Cluster& c) {
  auto& c_cells = c.cells();
  this->cells_.insert(this->cells_.end(), c_cells.begin(), c_cells.end());
  this->Qc_ += c.Qc() - c.totalHeight() * this->totalHeight_;
  this->totalHeight_ += c.totalHeight();
}

long long Cluster::getInsertCost() {
  int num = this->cells_.size();
  if (num == 1) {
    auto& inst = this->cells_[0];
    return inst->height() * (pow(inst->oldlx() - this->lx_, 2) +
                             pow(inst->oldly() - this->ly_, 2));
  }
  long long cost = 0;
  int curLy = this->ly_;
  for (int i = 0; i < num - 2; i++) {
    auto& inst = this->cells_[i];
    cost += inst->height() * (pow(inst->oldly() - curLy, 2) -
                              pow(inst->oldly() - inst->ly(), 2));
    curLy += inst->height();
  }
  // change the order of the last two, choose the best order
  auto inst1 = this->cells_[num - 2];  // The last one of original cells
  auto inst2 = this->cells_[num - 1];  // The inserted cell
  //[1,2]
  long long cost1 =
      inst2->height() * ((pow(inst2->oldlx() - this->lx_, 2) +
                          pow(inst2->oldly() - curLy - inst1->height(), 2))) +
      inst1->height() * (pow(inst1->oldly() - curLy, 2) -
                         pow(inst1->oldly() - inst1->ly(), 2));
  //[2,1]
  long long cost2 =
      inst2->height() * ((pow(inst2->oldlx() - this->lx_, 2) +
                          pow(inst2->oldly() - curLy, 2))) +
      inst1->height() * (pow(inst1->oldly() - curLy - inst2->height(), 2) -
                         pow(inst1->oldly() - inst1->ly(), 2));
  if (cost1 > cost2) {
    this->cells_[num - 2] = inst2;
    this->cells_[num - 1] = inst1;
  }
  return cost + min(cost1, cost2);
}

long long Cluster::getTotalCost() {
  long long cost = 0;
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
    cost += inst->height() *
            (pow(inst->oldlx() - this->lx_, 2) + pow(inst->oldly() - curLy, 2));
    curLy += inst->height();
  }
  return cost;
}

void Cluster::setLocations() {
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
    inst->setLoc(this->lx_, curLy);
    curLy += inst->height();
  }
}

void Col::collapse(Cluster& c) {
  c.setLy(round(1.0 * c.Qc() / c.totalHeight()));
  if (c.ly() < 0) c.setLy(0);
  if (c.uy() > Row_cnt * 8 - 1) c.setLy(Row_cnt * 8 - 1 - c.totalHeight());
  // if overlap with predecessor c'
  if (c.idx() > 0) {
    auto& pre_c = this->Clusters_[c.idx() - 1];
    if (pre_c.uy() > c.ly()) {  // cluster overlap
      pre_c.addCluster(c);
      this->Clusters_.erase(this->Clusters_.begin() + c.idx());
      collapse(pre_c);
    }
  }
}

long long Col::PlaceCol(shared_ptr<cell>& inst) {
  int id = inst->idx();
  if (this->Clusters_.size() == 0 ||
      this->Clusters_.rbegin()->uy() <= inst->oldly()) {
    // create a new cluster
    Cluster c(inst, this->Clusters_.size(), this->idx_ * 8);
    this->Clusters_.push_back(c);
  } else {
    auto& c = this->Clusters_[this->Clusters_.size() - 1];
    c.addCell(inst);
    this->collapse(c);
  }
  long long cost = this->Clusters_[this->Clusters_.size() - 1].getInsertCost();
  return cost;
}

Legalize::Legalize() {
  // general rows
  for (int i = 0; i < Col_cnt; i++) {
    COLS_.push_back(Col(i));
  }
  // sort cells in y order
  this->sortedCells_.assign(CELLS.begin(), CELLS.end());
  sort(this->sortedCells_.begin(), this->sortedCells_.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         if (inst1->ly() == inst2->ly()) {
           return inst1->height() > inst2->height();
         } else
           return inst1->ly() < inst2->ly();
       });
}

void Legalize::doLegalize() {
  LOG << "Do legalization." << endl;
  for (auto& inst : this->sortedCells_) {
    int id = inst->idx();
    long long bestCost = LLONG_MAX;
    Col bestCol;
    int nearestCol = round(1.0 * inst->lx() / 8);
    bool left = true, right = true;
    for (int i = 0; left || right; i++) {
      // left
      if (nearestCol - i >= 0 && left) {
        auto curCol = COLS_[nearestCol - i];
        long long curCost = curCol.PlaceCol(inst);
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        } else if (curCost > bestCost)
          left = false;
      } else if (nearestCol - i < 0)
        left = false;

      // right
      if (nearestCol + i < Col_cnt && right && i != 0) {
        auto curCol = COLS_[nearestCol + i];
        long long curCost = curCol.PlaceCol(inst);
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        } else if (curCost > bestCost)
          right = false;
      } else if (nearestCol + i >= Col_cnt)
        right = false;
    }
    COLS_[bestCol.idx()] = bestCol;
    // set final position
    for (auto& c : bestCol.Clusters()) {
      int curLy = c.ly();
      for (auto& inst : c.cells()) {
        inst->setLoc(bestCol.idx() * 8, curLy);
        curLy += inst->height();
      }
    }
  }
}

void Legalize::refinement() {
  LOG << "Refinement." << endl;
  long long totalCost = 0;
  for (auto& co : this->COLS_) {
    for (auto& c : co.Clusters()) {
      // move cell order
      int stride = 5;
      int num = c.cells().size();
      if (num == 1) {
        totalCost += c.getTotalCost();
        continue;
      }
      if (num < stride + 1) stride = num - 1;
      auto cells = c.cells();
      vector<shared_ptr<cell>> bestList = cells;
      long long bestCost = c.getTotalCost();
      for (int n1 = 0, n2 = n1 + stride; n2 < num; n1++, n2++) {
        vector<shared_ptr<cell>> slice(cells.begin() + n1,
                                       cells.begin() + n2 + 1);
        while (next_permutation(slice.begin(), slice.end())) {
          vector<shared_ptr<cell>> curList;
          curList.assign(cells.begin(), cells.begin() + n1);
          curList.insert(curList.end(), slice.begin(), slice.end());
          curList.insert(curList.end(), cells.begin() + n2 + 1, cells.end());
          c.setCellsOrder(curList);
          long long curCost = c.getTotalCost();
          if (curCost < bestCost) {
            bestCost = curCost;
            bestList = curList;
          }
        }
        slice.assign(cells.begin() + n1, cells.begin() + n2 + 1);
        while (prev_permutation(slice.begin(), slice.end())) {
          vector<shared_ptr<cell>> curList;
          curList.assign(cells.begin(), cells.begin() + n1);
          curList.insert(curList.end(), slice.begin(), slice.end());
          curList.insert(curList.end(), cells.begin() + n2 + 1, cells.end());
          c.setCellsOrder(curList);
          long long curCost = c.getTotalCost();
          if (curCost < bestCost) {
            bestCost = curCost;
            bestList = curList;
          }
        }
      }
      c.setCellsOrder(bestList);
      c.setLocations();
    }
  }
}

long long Legalize::getTotalCost() {
  long long cost = 0;
  for (auto& col : this->COLS_) {
    for (auto& c : col.Clusters()) {
      cost += c.getTotalCost();
    }
  }
  return cost;
}