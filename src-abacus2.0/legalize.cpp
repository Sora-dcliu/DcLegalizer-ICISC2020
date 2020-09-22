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
  this->Qc_ += inst->height() * (inst->oldly() - this->totalHeight_);
  this->totalHeight_ += inst->height();
}

void Cluster::addCluster(Cluster& c) {
  auto& c_cells = c.cells();
  this->cells_.insert(this->cells_.end(), c_cells.begin(), c_cells.end());
  this->Qc_ += c.Qc() - c.totalHeight() * this->totalHeight_;
  this->totalHeight_ += c.totalHeight();
}

long long Cluster::getInsertCost(shared_ptr<cell>& newInst) {
  int id = newInst->idx();
  int num = this->cells_.size();
  if (num == 1 && this->cells_[0] == newInst) {
    return newInst->getCost(this->lx_, this->ly_);
  }
  int newLoc = num;
  int curUy = this->uy();
  for (int i = num - 1; i >= 0; i--) {
    auto inst = this->cells_[i];
    if (inst->oldly() > newInst->oldly() + newInst->height()) {
      curUy -= inst->height();
      newLoc = i;
      continue;
    }
    if (inst->oldly() + inst->height() > newInst->oldly()) {
      //[...,old,new] cost = old + new
      long long cost1 =
          inst->height() * pow(inst->oldly() - (curUy - newInst->height() - inst->height()), 2) +
          newInst->height() * pow(newInst->oldly() - (curUy - newInst->height()), 2);
      //[...,new,old]
      long long cost2 = inst->height() * pow(inst->oldly() - (curUy - inst->height()), 2) +
                        newInst->height() *
                            pow(newInst->oldly() - (curUy - inst->height() - newInst->height()), 2);
      if (cost1 > cost2) {
        newLoc = i;
        curUy -= inst->height();
      } else
        break;
    } else
      break;
  }
  this->cells_.insert(this->cells_.begin() + newLoc, newInst);
  long long cost = 0;
  int curLy = this->ly_;
  for (int i = 0; i <= num; i++) {
    auto& inst = this->cells_[i];
    if (inst == newInst) {
      cost += inst->getCost(this->lx_, curLy);
    } else {
      cost += inst->getCost(inst->lx(), curLy) - inst->getCost();
    }
    curLy += inst->height();
  }
  return cost;
}

long long Cluster::getTotalCost() {
  long long cost = 0;
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
    cost += inst->getCost(this->lx_, curLy);
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

int Col::collapse(Cluster& c) {
  int idx = c.idx();
  c.setLy(round(1.0 * c.Qc() / c.totalHeight()));
  if (c.ly() < 0) c.setLy(0);
  if (c.uy() > Row_cnt * 8) c.setLy(Row_cnt * 8 - c.totalHeight());
  // if overlap with predecessor c-pre or next
  if (c.idx() > 0) {
    auto& pre_c = this->Clusters_[c.idx() - 1];
    if (pre_c.uy() > c.ly()) {  // cluster overlap
      pre_c.addCluster(c);
      this->deletCluster(c.idx());
      idx = collapse(pre_c);
    }
  }
  if (c.idx() < this->Clusters_.size() - 1) {
    auto& next_c = this->Clusters_[c.idx() + 1];
    if (c.uy() > next_c.ly()) {
      c.addCluster(next_c);
      this->deletCluster(next_c.idx());
      idx = collapse(c);
    }
  }
  return idx;
}

long long Col::PlaceCol(shared_ptr<cell>& inst) {
  int id = inst->idx();
  if (this->Clusters_.size() == 0 || this->Clusters_.rbegin()->uy() <= inst->oldly()) {
    // create a new cluster
    Cluster c(inst, this->Clusters_.size(), this->idx_ * 8);
    this->Clusters_.push_back(c);
    int idx = this->collapse(this->Clusters_[this->Clusters_.size() - 1]);
    return this->Clusters_[idx].getInsertCost(inst);
  } else {
    auto& c = this->Clusters_[this->Clusters_.size() - 1];
    c.addCell(inst);
    this->collapse(c);
  }
  long long cost = this->Clusters_[this->Clusters_.size() - 1].getInsertCost(inst);
  return cost;
}

long long Col::popReduce(shared_ptr<cell>& inst) {
  long long cost = 0;
  int c_cnt = this->Clusters_.size();
  for (int i = 0; i < c_cnt; i++) {
    auto cells = Clusters_[i].cells();
    auto iter = find(cells.begin(), cells.end(), inst);
    if (iter != cells.end()) {
      if (cells.size() == 1) {
        this->deletCluster(i);
        return 0;
      }
      cells.erase(iter);
      Col subCol(this->idx_);
      for (auto& subinst : cells) {
        subCol.PlaceCol(subinst);
      }
      auto& subclusters = subCol.Clusters();
      for (auto c : subclusters) {
        int curLy = c.ly();
        for (auto& subinst : c.cells()) {
          if (subinst == inst) throw "Error - popReduce.";
          if (subinst->ly() != curLy) {
            cost += subinst->getCost(subinst->lx(), curLy) - subinst->getCost();
          }
          curLy += subinst->height();
        }
      }

      if (cost > 0)
        throw "Erro - positive pop cost.";
      else {
        this->deletCluster(i);
        this->Clusters_.insert(this->Clusters_.begin() + i, subclusters.begin(), subclusters.end());
        for (int j = i; j < this->Clusters_.size(); j++) {
          this->Clusters_[j].setIdx(j);
        }
        return cost;
      }
    }
  }
  throw "Eror - inst not found.";
  return 0;
}

long long Col::ReInsertCol(shared_ptr<cell>& inst) {
  int id = inst->idx();
  int num = this->Clusters_.size();
  long long cost = LLONG_MAX;
  if (num == 0) {
    Cluster newc(inst, num, this->idx_ * 8);
    this->Clusters_.push_back(newc);
    this->collapse(this->Clusters_[0]);
    cost = inst->getCost(this->idx_ * 8, this->Clusters_[0].ly()) - inst->getCost();
    return cost;
  }
  if (this->Clusters_[0].ly() >= inst->oldly() + inst->height()) {
    // create a new cluster
    Cluster newc(inst, 0, this->idx_ * 8);
    for (int j = 0; j < num; j++) this->Clusters_[j].setIdx(j + 1);
    this->Clusters_.insert(this->Clusters_.begin(), newc);
    this->collapse(this->Clusters_[0]);
    cost = this->Clusters_[0].getInsertCost(inst) - inst->getCost();
    return cost;
  }
  if (this->Clusters_[num - 1].uy() <= inst->oldly()) {
    // create a new cluster
    Cluster newc(inst, num, this->idx_ * 8);
    this->Clusters_.push_back(newc);
    this->collapse(this->Clusters_[num]);
    cost = this->Clusters_[this->Clusters_.size() - 1].getInsertCost(inst) - inst->getCost();
    return cost;
  }
  for (int i = num - 1; i >= 0; i--) {
    auto& c = this->Clusters_[i];
    if (c.ly() < inst->oldly() + inst->height() && c.uy() > inst->oldly()) {
      // insert into existent cluster
      c.addCell(inst);
      int idx = this->collapse(c);
      cost = this->Clusters_[idx].getInsertCost(inst) - inst->getCost();
      break;
    } else if (c.uy() <= inst->oldly()) {
      // create a new cluster
      Cluster newc(inst, i + 1, this->idx_ * 8);
      this->Clusters_.insert(this->Clusters_.begin() + i + 1, newc);
      for (int j = i + 2; j <= num; j++) this->Clusters_[j].setIdx(j);
      this->collapse(this->Clusters_[i + 1]);
      cost = inst->getCost(this->idx_ * 8, inst->oldly()) - inst->getCost();
      break;
    }
  }
  if (cost == LLONG_MAX) throw "Error - Reinsert cost.";
  return cost;
}

Legalize::Legalize() {
  // general columns
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
    bestCol.Clusters().rbegin()->setLocations();
  }
  for (int i = 0; i < Col_cnt; i++) {
    this->last_changedCols_.insert(i);
  }
}

bool Legalize::reFind() {
  LOG << "Refinding." << endl;
  bool improve = false;
  // Arrange cells in descending order of uy
  auto reSortedCell(CELLS);
  sort(reSortedCell.begin(), reSortedCell.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         if (inst1->uy() == inst2->uy())
           return inst1->height() > inst2->height();
         else
           return inst1->uy() > inst2->uy();
       });

  for (auto& inst : reSortedCell) {
    int idx = inst->idx();
    int nearestCol = round(inst->oldlx() / 8);
    Col originCol(this->COLS_[inst->lx() / 8]);
    Col bestCol(originCol);
    // The added cost of movement must not be greater than the reduced cost of origin col
    long long bestCost = -originCol.popReduce(inst);
    bool left = true, right = true;
    bool changed = false;  // wether the current column has been changed
    if (this->last_changedCols_.find(inst->lx() / 8) != this->last_changedCols_.end())
      changed = true;
    for (int i = 0; left || right; i++) {
      if (inst->getCost((nearestCol - i) * 8, inst->oldly()) - inst->getCost() >= bestCost)
        left = false;
      if (inst->getCost((nearestCol + i) * 8, inst->oldly()) - inst->getCost() >= bestCost)
        right = false;
      // left
      if (nearestCol - i >= 0 && left && nearestCol - i != originCol.idx() &&
          (changed ||
           this->last_changedCols_.find(nearestCol - i) != this->last_changedCols_.end())) {
        auto curCol = COLS_[nearestCol - i];
        long long curCost = curCol.ReInsertCol(inst);
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        }
      } else if (nearestCol - i < 0)
        left = false;

      // right
      if (nearestCol + i < Col_cnt && right && i != 0 && nearestCol + i != originCol.idx() &&
          (changed ||
           this->last_changedCols_.find(nearestCol + i) != this->last_changedCols_.end())) {
        auto curCol = COLS_[nearestCol + i];
        long long curCost = curCol.ReInsertCol(inst);
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        }
      } else if (nearestCol + i >= Col_cnt)
        right = false;
    }
    if (bestCol.idx() == inst->lx() / 8)
      continue;
    else
      improve = true;
    COLS_[bestCol.idx()] = bestCol;
    COLS_[originCol.idx()] = originCol;
    this->cur_changedCols.insert(bestCol.idx());
    this->cur_changedCols.insert(originCol.idx());
    this->last_changedCols_.insert(bestCol.idx());
    this->last_changedCols_.insert(originCol.idx());
    // set final position
    for (auto& c : bestCol.Clusters()) {
      c.setLocations();
    }
    for (auto& c : originCol.Clusters()) {
      c.setLocations();
    }
  }
  this->last_changedCols_.swap(this->cur_changedCols);
  this->cur_changedCols.clear();
  return improve;
}

void Legalize::VerMove() {
  LOG << "Vertical move." << endl;
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
        vector<shared_ptr<cell>> slice(cells.begin() + n1, cells.begin() + n2 + 1);
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

long long getTotalCost() {
  long long cost = 0;
  for (auto& inst : CELLS) {
    cost += inst->getCost();
  }
  return cost;
}

void checkLegal() {
  cout << "[EVL] Check legality." << endl;
  vector<vector<shared_ptr<cell>>> grid(Col_cnt, vector<shared_ptr<cell>>(Row_cnt * 8));
  for (auto& inst : CELLS) {
    if (inst->lx() % 8)
      throw "Error - Illegal lx.";
    else if (inst->lx() < 0 || inst->ux() > Col_cnt * 8 || inst->ly() < 0 ||
             inst->uy() > Row_cnt * 8) {
      throw "Error - beyond the border.";
    } else {
      for (int i = inst->ly(); i < inst->uy(); i++) {
        if (grid[inst->lx() / 8][i]) {
          throw "Error - overlap.";
        } else
          grid[inst->lx() / 8][i] = inst;
      }
    }
  }
  cout << "[EVL] Legal!" << endl;
}