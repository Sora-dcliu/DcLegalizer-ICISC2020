#include "legalize.h"

#include <climits>
#include <cmath>

#include "bipartite.h"

#define LOG cout << "[LG] "
#define CP LOG << "[PLACE] "

Legalize::Legalize() {
  this->bestCost = LLONG_MAX;

  // general columns
  LOG << "Initialize." << endl;
  for (int i = 0; i < Col_cnt; i++) {
    this->COLS_.push_back(Col(i));
  }
  sortedCells_.assign(CELLS.begin(), CELLS.end());
  // sort standard cells in ly coordinate
  sort(this->sortedCells_.begin(), this->sortedCells_.end(),
       [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
         if (inst1->oldly() == inst2->oldly()) {
           return inst1->height() > inst2->height();
         } else
           return inst1->oldly() < inst2->oldly();
       });
}

void Legalize::doLegalize() {
  LOG << "Legalization begin." << endl;
  LOG << "Columns place." << endl;
  this->ColPlace();
  this->bestCost = getTotalCost();
  LOG << "Total cost: " << this->bestCost << endl;
  WriteGds("orig.gds");
  this->reFind();
  this->BipartiteGraphMatch();
}

void Legalize::ColPlace() {
  CP << "Col place." << endl;
  for (auto& inst : sortedCells_) {
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

void Legalize::reFind() {
  LOG << "Refinding." << endl;
  auto reSortedCell(sortedCells_);
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

  long long curCost = getTotalCost();
  if (curCost < bestCost) {
    float improve = (bestCost - curCost) * 1.0 / bestCost * 100;
    LOG << "Get better solution: " << bestCost << "->" << curCost << " improve: " << improve << "%"
        << endl;
    bestCost = curCost;
    if (improve > 0.1) reFind();
  }
}

void Legalize::BipartiteGraphMatch() {
  LOG << "Bipartite graph match." << endl;
  BGM bgm;
  bgm.doBipartiteGraphMatch();
  LOG << "[BGM] Compelete." << endl;
}

long long Col::InsertCol(shared_ptr<cell>& inst) {
  // Check if the new cell overlaps with the existing cluster
  // idx of overlaped cluster, and also present the changed cluster.
  int c_idx = this->OverlapCluster(inst);
  if (c_idx == -1) {  // no overlap
    // create a new cluster
    Cluster c(inst, this->idx_ * 8);
    c_idx = this->insert_newCluster(c);
    c_idx = this->collapse(c_idx);
  } else {
    // Insert the cell into the cluster that overlaps it
    this->Clusters_[c_idx].addCell(inst);
    c_idx = this->collapse(c_idx);
  }
  return this->Clusters_[c_idx].getInsertCost(inst, true);
}

long long Col::DeleteInst(shared_ptr<cell>& inst) {
  long long cost = 0;
  int c_idx = this->OverlapCluster(inst);
  auto cells = this->Clusters_[c_idx].cells();
  cells.erase(find(cells.begin(), cells.end(), inst));  // Delete target cell
  Col subCol(this->idx_);  // create a new col to re-place the cells left
  this->Clusters_.erase(this->Clusters_.begin() + c_idx);
  for (auto& subinst : cells) subCol.InsertCol(subinst);
  for (auto& sc : subCol.Clusters()) {
    int curLy = sc.ly();
    for (auto& subinst : sc.cells()) {
      if (subinst->ly() != curLy)
        cost += subinst->getCost(subinst->lx(), curLy) - subinst->getCost();
      curLy += subinst->height();
    }
    this->insert_newCluster(sc);
  }
  if (cost > 0) {
    WriteGds("error.gds");
    throw "Erro - positive pop cost.";
  } else
    return cost;

  throw "Eror - inst not found.";
  return 0;
}

// Check if the new cell(old_ly) overlaps with the existing cluster
int Col::OverlapCluster(shared_ptr<cell>& inst) {
  int num = this->Clusters_.size();
  if (num == 0) return -1;
  // case 1 - ColPlace stage, only overlaps with the last one.
  if (this->Clusters_[num - 1].isOverlap(inst))
    return num - 1;
  else {  // case 2 - reFind stage, could overlap with any cluster.
    // Binary search
    int ll = 0, hh = num - 2;
    while (hh >= ll) {
      int mid = (ll + hh) / 2;
      if (this->Clusters_[mid].isOverlap(inst))
        return mid;
      else if (this->Clusters_[mid].ly() < inst->oldly())
        ll = mid + 1;
      else
        hh = mid - 1;
    }
    return -1;
  }
}

int Col::collapse(int c_idx) {
  auto& c = this->Clusters_[c_idx];
  c.setLy(round(1.0 * c.Qc() / c.totalHeight()));
  if (c.ly() < 0) c.setLy(0);
  if (c.uy() > Row_cnt * 8) c.setLy(Row_cnt * 8 - c.totalHeight());
  // Whether it overlaps with the cluster below
  if (c_idx != 0) {
    auto& c_below = this->Clusters_[c_idx - 1];
    if (c_below.uy() > c.ly()) {  // overlap
      c_below.addCluster(c);
      this->Clusters_.erase(this->Clusters_.begin() + c_idx);
      return collapse(c_idx - 1);
    }
  }
  // above
  if (c_idx != this->Clusters_.size() - 1) {
    auto& c_above = this->Clusters_[c_idx + 1];
    if (c_above.ly() < c.uy()) {
      c.addCluster(c_above);
      this->Clusters_.erase(this->Clusters_.begin() + c_idx + 1);
      return collapse(c_idx);
    }
  }
  return c_idx;
}

int Col::insert_newCluster(Cluster& c) {
  int num = this->Clusters_.size();
  if (num == 0) {
    this->Clusters_.push_back(c);
    return 0;
  }
  // case 1 - ColPlace stage,place to the end.
  if (this->Clusters_[num - 1].ly() < c.ly()) {
    this->Clusters_.push_back(c);
    return num;
  }
  // case 2 - reFind stage.
  else {
    for (int i = num - 2; i >= 0; i--) {
      if (this->Clusters_[i].ly() < c.ly()) {
        this->Clusters_.insert(this->Clusters_.begin() + i + 1, c);
        return i + 1;
      }
    }
    this->Clusters_.insert(this->Clusters_.begin(), c);
    return 0;
  }
}

void Col::determindLoc() {
  for (auto& c : this->Clusters_) c.setLocations();
}

Cluster::Cluster(shared_ptr<cell>& inst, int lx) {
  this->cells_.push_back(inst);
  this->lx_ = lx;
  this->ly_ = inst->oldly();
  this->Qc_ = inst->height() * (inst->oldly());
  this->totalHeight_ = inst->height();
  this->changed = true;
}

void Cluster::addCell(shared_ptr<cell>& inst) {
  // Only the parameters are changed, but the vector remains unchanged
  this->Qc_ += inst->height() * (inst->oldly() - this->totalHeight_);
  this->totalHeight_ += inst->height();
  this->changed = true;
}

void Cluster::addCluster(Cluster& c) {
  this->Qc_ += c.Qc() - c.totalHeight() * this->totalHeight_;
  this->totalHeight_ += c.totalHeight();
  for (auto inst : c.cells()) {
    this->getInsertCost(inst, false);
  }
  this->changed = true;
}

long long Cluster::getInsertCost(shared_ptr<cell>& newInst, bool calcCost) {
  int num = this->cells_.size();
  if (num == 0) throw "Error - cluster cell cnt.";
  if (num == 1 && this->cells_[0] == newInst) return newInst->getCost(this->lx_, this->ly_);
  for (int i = num - 1; i >= 0; i--) {
    if (this->cells_[i]->Key() < newInst->Key()) {
      this->cells_.insert(this->cells_.begin() + i + 1, newInst);
      break;
    }
  }
  if (this->cells_.size() == num) this->cells_.insert(this->cells_.begin(), newInst);

  long long cost = 0;
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
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
  if (!this->changed) return;
  int curLy = this->ly_;
  for (auto& inst : this->cells_) {
    inst->setLoc(this->lx_, curLy);
    curLy += inst->height();
  }
  this->changed = false;
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