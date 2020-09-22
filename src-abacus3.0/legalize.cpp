#include "legalize.h"

#include <climits>
#include <cmath>

#define LOG cout << "[LG] "
#define CP LOG << "[PLACE] "

Legalize::Legalize() {
  this->bestCost = LLONG_MAX;

  // general columns
  LOG << "Initialize." << endl;
  for (int i = 0; i < Col_cnt; i++) {
    this->COLS_.push_back(Col(i));
  }
  // split macros and standard cells
  for (auto& inst : CELLS) {
    if (inst->isMacro())
      this->sortedMacro_.push_back(inst);

    else
      this->sortedStd_.push_back(inst);
  }
  // sort macros in ly coordinate
  sort(this->sortedMacro_.begin(), this->sortedMacro_.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         if (inst1->oldly() == inst2->oldly()) {
           return inst1->height() > inst2->height();
         } else
           return inst1->oldly() < inst2->oldly();
       });

  // sort standard cells in ly coordinate
  sort(this->sortedStd_.begin(), this->sortedStd_.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         /* if (inst1->oldly() == inst2->oldly()) {
           return inst1->height() > inst2->height();
         } else */
         return inst1->oldly() < inst2->oldly();
       });
}

void Legalize::doLegalize() {
  LOG << "Legalization begin." << endl;
  LOG << "Macros leagalization." << endl;
  this->ColPlace(this->sortedMacro_);
  this->bestCost = getTotalMacroCost();
  this->reFind(this->sortedMacro_);
  LOG << "Std legalization." << endl;
  this->ColPlace(this->sortedStd_);
  this->bestCost = getTotalCost();
  this->reFind(CELLS);
}

void Legalize::ColPlace(vector<shared_ptr<cell>>& insts) {
  CP << "Col place." << endl;
  for (auto& inst : insts) {
    int idx = inst->idx();
    long long bestCost = LLONG_MAX;  // Minimum cost
    Col bestCol;                     // Save results with minimal cost
    int nearestCol_idx =
        round(1.0 * inst->oldlx() / 8);  // The index of column closest to the starting point
    nearestCol_idx = min(nearestCol_idx, Col_cnt - 1);
    nearestCol_idx = max(0, nearestCol_idx);

    bool left = true, right = true;        // Whether to explore in that direction
    for (int i = 0; left || right; i++) {  // i - Depth of exploration
      // left
      if (nearestCol_idx - i >= 0 && left) {
        auto curCol = COLS_[nearestCol_idx - i];
        long long curCost = curCol.InsertCol(inst);  // Insert the cell and return the cost
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        }
        // When cost increase, stop the search in the direction
        else if (curCost > bestCost)
          left = false;
      } else if (nearestCol_idx - i < 0)
        left = false;
      // right
      if (nearestCol_idx + i < Col_cnt && right && i != 0) {
        auto curCol = COLS_[nearestCol_idx + i];
        long long curCost = curCol.InsertCol(inst);
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        } else if (curCost > bestCost)
          right = false;
      } else if (nearestCol_idx + i >= Col_cnt)
        right = false;
    }
    // Determine the optimal column, and the location of its cells
    int bestCol_idx = bestCol.idx();
    this->COLS_[bestCol_idx] = bestCol;
    this->COLS_[bestCol_idx].determindLoc();
  }
  // initial the set for refind.
  this->last_changedCols_.clear();
  for (int i = 0; i < Col_cnt; i++) {
    this->last_changedCols_.insert(i);
  }
}

void Legalize::reFind(vector<shared_ptr<cell>>& insts) {
  LOG << "Refinding." << endl;
  auto reSortedCell(insts);
  sort(reSortedCell.begin(), reSortedCell.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         if (inst1->uy() == inst2->uy())
           return inst1->height() > inst2->height();
         else
           return inst1->uy() > inst2->uy();
       });
  for (auto& inst : reSortedCell) {
    int nearestCol_idx =
        round(1.0 * inst->oldlx() / 8);  // The index of column closest to the starting point
    nearestCol_idx = min(nearestCol_idx, Col_cnt - 1);
    nearestCol_idx = max(0, nearestCol_idx);
    Col originCol(this->COLS_[inst->lx() / 8]);
    Col bestCol(originCol);
    // The added cost of movement must not be greater than the reduced cost of origin col
    long long bestCost = -originCol.DeleteInst(inst);
    bool left = true, right = true;
    bool changed = false;  // wether the current column has been changed
    if (this->last_changedCols_.find(inst->lx() / 8) != this->last_changedCols_.end())
      changed = true;
    for (int i = 0; left || right; i++) {
      if (inst->getCost((nearestCol_idx - i) * 8, inst->oldly()) - inst->getCost() >= bestCost)
        left = false;
      if (inst->getCost((nearestCol_idx + i) * 8, inst->oldly()) - inst->getCost() >= bestCost)
        right = false;
      // left
      if (nearestCol_idx - i >= 0 && left && nearestCol_idx - i != originCol.idx() &&
          (changed ||
           this->last_changedCols_.find(nearestCol_idx - i) != this->last_changedCols_.end())) {
        auto curCol = COLS_[nearestCol_idx - i];
        long long curCost = curCol.InsertCol(inst) - inst->getCost();
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        }
      } else if (nearestCol_idx - i < 0)
        left = false;

      // right
      if (nearestCol_idx + i < Col_cnt && right && i != 0 &&
          nearestCol_idx + i != originCol.idx() &&
          (changed ||
           this->last_changedCols_.find(nearestCol_idx + i) != this->last_changedCols_.end())) {
        auto curCol = COLS_[nearestCol_idx + i];
        long long curCost = curCol.InsertCol(inst) - inst->getCost();
        if (curCost < bestCost) {
          bestCost = curCost;
          bestCol = curCol;
        }
      } else if (nearestCol_idx + i >= Col_cnt)
        right = false;
    }
    if (bestCol.idx() == inst->lx() / 8) continue;
    COLS_[bestCol.idx()] = bestCol;
    COLS_[originCol.idx()] = originCol;
    this->cur_changedCols.insert(bestCol.idx());
    this->cur_changedCols.insert(originCol.idx());
    this->last_changedCols_.insert(bestCol.idx());
    this->last_changedCols_.insert(originCol.idx());
    // set final position
    bestCol.determindLoc();
    originCol.determindLoc();
  }
  this->last_changedCols_.swap(this->cur_changedCols);
  this->cur_changedCols.clear();
  if (insts.size() == this->sortedMacro_.size()) {
    long long curCost = getTotalMacroCost();
    if (curCost < bestCost) {
      float improve = (bestCost - curCost) * 1.0 / bestCost * 100;
      LOG << "Get better macro solution: " << bestCost << "->" << curCost << " improve: " << improve
          << "%" << endl;
      bestCost = curCost;
      if (improve > 0.1) reFind(insts);
    }
  } else {
    long long curCost = getTotalCost();
    if (curCost < bestCost) {
      float improve = (bestCost - curCost) * 1.0 / bestCost * 100;
      LOG << "Get better solution: " << bestCost << "->" << curCost << " improve: " << improve
          << "%" << endl;
      bestCost = curCost;
      if (improve > 0.1) reFind(insts);
    }
  }
}

long long Col::InsertCol(shared_ptr<cell>& inst) {
  // Check if the new cell overlaps with the existing cluster
  auto iter = this->OverlapCluster(inst);
  if (iter == this->Clusters_.end()) {
    // create a new cluster
    Cluster c(inst, this->idx_ * 8);
    this->Clusters_.insert(make_pair(c.ly(), c));
    iter = this->collapse(this->Clusters_.find(c.ly()));
  } else {
    // Insert the cell into the cluster that overlaps it
    iter->second.addCell(inst);
    iter = this->collapse(iter);
  }
  this->changedCluster_.push_back(iter->first);
  return iter->second.getInsertCost(inst, true);
}

long long Col::DeleteInst(shared_ptr<cell>& inst) {
  int idx = inst->idx();
  long long cost = 0;
  for (auto c : this->Clusters_) {
    auto cells = c.second.cells();
    auto iter = find(cells.begin(), cells.end(), inst);
    if (iter != cells.end()) {
      if (cells.size() == 1) {
        this->Clusters_.erase(c.second.ly());
        return 0;
      }
      cells.erase(iter);
      Col subCol(this->idx_);
      for (auto& subinst : cells) {
        subCol.InsertCol(subinst);
      }
      auto& subclusters = subCol.Clusters();
      for (auto& sc : subclusters) {
        int curLy = sc.second.ly();
        for (auto& subinst : sc.second.cells()) {
          if (subinst->ly() != curLy)
            cost += subinst->getCost(subinst->lx(), curLy) - subinst->getCost();
          curLy += subinst->height();
        }
      }
      if (cost > 0) {
        throw "Erro - positive pop cost.";
      } else {
        this->Clusters_.erase(c.first);
        for (auto& sc : subclusters) {
          this->Clusters_.insert(sc);
          this->changedCluster_.push_back(sc.first);
        }
        return cost;
      }
    }
  }
  throw "Eror - inst not found.";
  return 0;
}

// Check if the new cell(old_ly) overlaps with the existing cluster
map<int, Cluster>::iterator Col::OverlapCluster(shared_ptr<cell>& inst) {
  // above the inst, the nearest cluster
  auto iter = this->Clusters_.lower_bound(inst->oldly());
  if (iter != this->Clusters_.end()) {
    auto& above_c = iter->second;
    if (above_c.ly() < inst->oldly() + inst->height()) return iter;
  }
  // below
  if (iter != this->Clusters_.begin()) {
    iter--;
    auto& below_c = iter->second;
    if (below_c.uy() > inst->oldly()) return iter;
  }
  return this->Clusters_.end();
}

map<int, Cluster>::iterator Col::collapse(map<int, Cluster>::iterator iter) {
  auto c = iter->second;
  c.setLy(round(1.0 * c.Qc() / c.totalHeight()));
  if (c.ly() < 0) c.setLy(0);
  if (c.uy() > Row_cnt * 8) c.setLy(Row_cnt * 8 - c.totalHeight());
  if (c.ly() != iter->first) {
    int oldly = iter->first;
    this->Clusters_.erase(iter);
    iter = this->Clusters_.find(c.ly());
    if (iter != this->Clusters_.end()) {
      iter->second.addCluster(c);
      return this->collapse(iter);
    } else {
      this->Clusters_.insert(make_pair(c.ly(), c));
      iter = this->Clusters_.find(c.ly());
    }
  }
  // Check for overlap with the cluster below
  if (iter != this->Clusters_.begin()) {
    auto below_iter = iter;
    below_iter--;
    auto& below_c = below_iter->second;
    if (below_c.uy() > c.ly()) {
      below_c.addCluster(c);
      this->Clusters_.erase(iter);
      return this->collapse(below_iter);
    }
  }
  // above
  auto above_iter = iter;
  above_iter++;
  if (above_iter != this->Clusters_.end()) {
    auto& above_c = above_iter->second;
    if (above_c.ly() < c.uy()) {
      iter->second.addCluster(above_c);
      this->Clusters_.erase(above_iter);
      return this->collapse(iter);
    }
  }
  return iter;
}

void Col::determindLoc() {
  for (int ly : this->changedCluster_) {
    auto iter = this->Clusters_.find(ly);
    if (iter != this->Clusters_.end()) {
      auto& c = iter->second;
      c.setLocations();
    }
  }
  this->changedCluster_.clear();
}

Cluster::Cluster(shared_ptr<cell>& inst, int lx) {
  this->cells_.push_back(inst);
  this->lx_ = lx;
  this->ly_ = inst->oldly();
  this->Qc_ = inst->height() * (inst->oldly());
  this->totalHeight_ = inst->height();
}

void Cluster::addCell(shared_ptr<cell>& inst) {
  // Only the parameters are changed, but the vector remains unchanged
  this->Qc_ += inst->height() * (inst->oldly() - this->totalHeight_);
  this->totalHeight_ += inst->height();
}

void Cluster::addCluster(Cluster& c) {
  this->Qc_ += c.Qc() - c.totalHeight() * this->totalHeight_;
  this->totalHeight_ += c.totalHeight();
  // Default the lower one shelter upper one
  for (auto inst : c.cells()) {
    this->getInsertCost(inst, false);
  }
}

long long Cluster::getInsertCost(shared_ptr<cell>& newInst, bool calcCost) {
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
  if (!calcCost) return 0;
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

void Cluster::setLocations() {
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
    inst->setLoc(this->lx_, curLy);
    curLy += inst->height();
  }
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

long long getTotalCost() {
  long long cost = 0;
  for (auto& inst : CELLS) {
    cost += inst->getCost();
  }
  return cost;
}

long long getTotalMacroCost() {
  long long cost = 0;
  for (auto& inst : CELLS) {
    if (inst->isMacro()) cost += inst->getCost();
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