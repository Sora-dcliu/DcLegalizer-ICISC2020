#include "bipartite.h"
#define LOG cout << "[LG] [BGM] "

BGM::BGM() {
  LOG << "Generate grid." << endl;
  this->max_x_ = Col_cnt - 1;
  this->max_y_ = Row_cnt * 8 - 1;
  for (int x = 0; x <= this->max_x_; x++) {
    vector<Bin> Col;
    for (int y = 0; y <= this->max_y_; y++) {
      Col.push_back(Bin(x, y));
    }
    this->Grid.push_back(Col);
  }
  LOG << "Compelete information of grid." << endl;
  for (auto& inst : CELLS) {
    int x = inst->lx() / 8;
    for (int y = inst->ly(); y < inst->uy(); y++) {
      auto& bin = this->Grid[x][y];
      if (bin.Inst())
        throw "Occupied bin.";
      else
        bin.setInst(inst);
    }
  }
}

void BGM::doBipartiteGraphMatch() {
  LOG << "Begin." << endl;
  for (int x = 0; x <= this->max_x_; x++) {
    for (int y = 0; y <= this->max_y_; y++) {
      set<shared_ptr<cell>> insts;
      this->dfs(x, y, insts);
      if (insts.size() > 1) {
        int lx_bbx = max_x_;
        int ly_bbx = max_y_;
        int ux_bbx = 0;
        int uy_bbx = 0;
        for (auto& inst : insts) {
          lx_bbx = min(lx_bbx, inst->lx() / 8);
          ux_bbx = max(ux_bbx, inst->lx() / 8);
          ly_bbx = min(ly_bbx, inst->ly());
          uy_bbx = max(uy_bbx, inst->ly());
        }
        this->lx = lx_bbx;
        this->ly = ly_bbx;
        this->ux = ux_bbx;
        this->uy = uy_bbx;
        cout << x << " " << y << endl;
        this->cleanKM();
        this->Match(insts);
      }
    }
  }
}

// Depth-first search clustering
void BGM::dfs(int x, int y, set<shared_ptr<cell>>& insts) {
  auto& bin = Grid[x][y];
  if ((bin.isStd() || bin.isMacro()) && !bin.visited()) {
    insts.insert(bin.Inst());
    bin.mark();
    if (x - 1 >= 0) dfs(x - 1, y, insts);
    if (x + 1 <= max_x_) dfs(x + 1, y, insts);
    if (y - 1 >= 0) dfs(x, y - 1, insts);
    if (y + 1 <= max_y_) dfs(x, y + 1, insts);
  }
}

// KM algorithm
void BGM::Match(set<shared_ptr<cell>>& insts) {
  // init
  int width = ux - lx + 1;
  int height = uy - ly + 1;
  vector<shared_ptr<cell>>().swap(vec_insts);
  vector<shared_ptr<cell>>().swap(macros);
  for (auto& inst : insts) {
    if (inst->isMacro())
      macros.push_back(inst);
    else
      vec_insts.push_back(inst);
  }
  buildMap();
  long long bestCost = KM();
  this->setLocation();
  if (macros.size() == 0) {
    // update locations
    cleanKM();
    return;
  }

  sort(macros.begin(), macros.end(), [](shared_ptr<cell> inst1, shared_ptr<cell> inst2) {
    return inst1->height() > inst2->height();
  });
  for (int idx = 0; idx < macros.size(); idx++) {
    auto& macro = macros[idx];
    // left->right
    {
      int x = macro->lx() / 8 - 1;
      bool move = false;
      if (x >= lx)
        for (int y = macro->ly(); y < macro->uy(); y++) {
          auto& inst = this->Grid[x][y].Inst();
          if (find(vec_insts.begin(), vec_insts.end(), inst) != vec_insts.end() &&
              inst->oldlx() > macro->lx()) {
            move = true;
            break;
          }
        }
      if (move) {
        for (x = macro->lx() / 8 + 1; x <= ux; x++) {
          long long cost =
              this->MoveMacro(idx, 3) + macro->getCost(x * 8, macro->ly()) - macro->getCost();
          if (cost < bestCost) {
            bestCost = cost;
            this->setLocation();
            macro->setLoc(x * 8, macro->ly());
            this->cleanKM();
          } else {
            for (int y = macro->ly(); y < macro->uy(); y++) {
              this->Grid[x - 1][y].setInst(macro);
              this->Grid[x][y].deleteInst();
            }
            for (auto& inst : vec_insts) this->Grid[inst->lx() / 8][inst->ly()].setInst(inst);
            break;
          }
        }
      }
      // right->left
      {
        int x = macro->ux() / 8;
        bool move = false;
        if (x <= ux)
          for (int y = macro->ly(); y < macro->uy(); y++) {
            auto& inst = this->Grid[x][y].Inst();
            if (find(vec_insts.begin(), vec_insts.end(), inst) != vec_insts.end() &&
                inst->oldlx() < macro->ux() - 4) {
              move = true;
              break;
            }
          }
        if (move) {
          for (x = macro->lx() / 8 - 1; x >= lx; x--) {
            long long cost =
                this->MoveMacro(idx, 4) + macro->getCost(x * 8, macro->ly()) - macro->getCost();
            if (cost < bestCost) {
              bestCost = cost;
              this->setLocation();
              macro->setLoc(x * 8, macro->ly());
              this->cleanKM();
            } else {
              for (int y = macro->ly(); y < macro->uy(); y++) {
                this->Grid[x + 1][y].setInst(macro);
                this->Grid[x][y].deleteInst();
              }
              for (auto& inst : vec_insts) this->Grid[inst->lx() / 8][inst->ly()].setInst(inst);
              break;
            }
          }
        }
        // top->down
        {
          auto& inst = this->Grid[macro->lx() / 8][macro->uy()].Inst();
          if (find(vec_insts.begin(), vec_insts.end(), inst) != vec_insts.end() &&
              inst->oldly() < macro->uy()) {
            if (!this->Grid[macro->lx() / 8][macro->ly() - 1].isMacro())
              for (int y = macro->ly() - 1; y >= ly; y--) {
                long long cost =
                    this->MoveMacro(idx, 1) + macro->getCost(macro->lx(), y) - macro->getCost();
                if (cost < bestCost) {
                  bestCost = cost;
                  this->setLocation();
                  macro->setLoc(macro->lx(), y);
                  this->cleanKM();
                } else {
                  this->Grid[macro->lx() / 8][macro->ly() - 1].deleteInst();
                  this->Grid[macro->lx() / 8][macro->uy() - 1].setInst(macro);
                  for (auto& inst : vec_insts) this->Grid[inst->lx() / 8][inst->ly()].setInst(inst);
                  break;
                }
              }
          }
        }
        // down->top
        {
          auto& inst = this->Grid[macro->lx() / 8][macro->ly() - 1].Inst();
          if (find(vec_insts.begin(), vec_insts.end(), inst) != vec_insts.end() &&
              inst->oldly() > macro->ly()) {
            if (!this->Grid[macro->lx() / 8][macro->uy()].isMacro())
              for (int y = macro->ly() + 1; y <= uy - macro->height(); y++) {
                long long cost =
                    this->MoveMacro(idx, 2) + macro->getCost(macro->lx(), y) - macro->getCost();
                if (cost < bestCost) {
                  bestCost = cost;
                  this->setLocation();
                  macro->setLoc(macro->lx(), y);
                  this->cleanKM();
                } else {
                  this->Grid[macro->lx() / 8][macro->uy()].deleteInst();
                  this->Grid[macro->lx() / 8][macro->ly()].setInst(macro);
                  for (auto& inst : vec_insts) this->Grid[inst->lx() / 8][inst->ly()].setInst(inst);
                  break;
                }
              }
          }
        }
      }
    }
  }
}

long long BGM::MoveMacro(int idx, int type) {
  auto& macro = macros[idx];
  // 1-top->down 2-down->top 3-left->right 4-right->left
  if (type == 1) {
    auto& b1 = this->Grid[macro->lx() / 8][macro->ly() - 1];
    auto& b2 = this->Grid[macro->lx() / 8][macro->uy() - 1];
    b1.setInst(macro);
    b2.deleteInst();

  } else if (type == 2) {
    auto& b1 = this->Grid[macro->lx() / 8][macro->ly()];
    auto& b2 = this->Grid[macro->lx() / 8][macro->uy()];

    b2.setInst(macro);
    b1.deleteInst();
  } else if (type == 3 || type == 4) {
    int x = macro->lx() / 8;
    int newx = (type == 3) ? (x + 1) : (x - 1);
    for (int y = macro->ly(); y < macro->uy(); y++) {
      this->Grid[x][y].deleteInst();
      this->Grid[newx][y].setInst(macro);
    }
  }

  this->buildMap();
  return this->KM();
}

long long BGM::KM() {
  // do KM
  for (int i = 0; i < cnt_inst; i++) {
    while (1) {
      minz = LLONG_MAX;
      fill(vis_inst.begin(), vis_inst.end(), false);
      fill(vis_bin.begin(), vis_bin.end(), false);

      if (findPath(i)) break;  // It's matched correctly
      // Not matched, update top value
      for (int j = 0; j < cnt_inst; j++)
        if (vis_inst[j]) w_inst[j] += minz;

      for (int j = 0; j < cnt_bin; j++)
        if (vis_bin[j]) w_bin[j] -= minz;
    }
  }
  long long ans = 0;
  for (int i = 0; i < cnt_inst; i++) {
    ans += w_inst[i] + w_bin[c_inst[i]];
  }
  return ans;
}

bool BGM::findPath(int u) {
  vis_inst[u] = true;  // Join the augmented path
  for (int v = 0; v < cnt_bin; v++) {
    if (!vis_bin[v] && Map[u][v] != LLONG_MAX) {
      // The bin point has not added into an augmented path, and there is a path
      long long t = w_inst[u] + w_bin[v] - Map[u][v];
      if (t == 0) {
        vis_bin[v] = true;
        if (c_bin[v] == -1 || findPath(c_bin[v])) {
          c_inst[u] = v;
          c_bin[v] = u;
          return true;
        }
      } else if (t < 0) {
        minz = min(minz, -t);
      }
    }
  }
  return false;
}

void BGM::initKM() {
  int width = ux - lx + 1;
  int height = uy - ly + 1;
  cnt_inst = vec_insts.size();
  cnt_bin = width * height;

  w_inst.reserve(cnt_inst);
  w_inst.resize(cnt_inst);
  fill(w_inst.begin(), w_inst.end(), LLONG_MAX);

  w_bin.reserve(cnt_bin);
  w_bin.resize(cnt_bin);
  fill(w_bin.begin(), w_bin.end(), 0);

  c_inst.reserve(cnt_inst);
  c_inst.resize(cnt_inst);
  fill(c_inst.begin(), c_inst.end(), -1);

  c_bin.reserve(cnt_bin);
  c_bin.resize(cnt_bin);
  fill(c_bin.begin(), c_bin.end(), -1);

  vis_inst.reserve(cnt_inst);
  vis_bin.reserve(cnt_bin);
  vis_inst.resize(cnt_inst);
  vis_bin.resize(cnt_bin);
}

void BGM::buildMap() {
  this->initKM();
  int height = uy - ly + 1;
  for (int i = 0; i < cnt_inst; i++) {
    vector<long long> edge(cnt_bin);
    for (int j = 0; j < cnt_bin; j++) {
      int x = lx + j / height;
      int y = ly + j % height;
      auto& bin = this->Grid[x][y];
      if (find(vec_insts.begin(), vec_insts.end(), bin.Inst()) == vec_insts.end()) {
        edge[j] = LLONG_MAX;
      } else {
        edge[j] = bin.getCost(vec_insts[i]);
        w_inst[i] = min(w_inst[i], edge[j]);
      }
    }
    Map.push_back(edge);
  }
}

void BGM::setLocation() {
  int height = uy - ly + 1;
  for (int i = 0; i < cnt_bin; i++) {
    int x = lx + i / height;
    int y = ly + i % height;
    auto& bin = this->Grid[x][y];
    if (bin.isMacro()) continue;
    if (c_bin[i] != -1) {
      auto& inst = vec_insts[c_bin[i]];
      bin.setInst(inst);
      inst->setLoc(x * 8, y);
    } else if (find(vec_insts.begin(), vec_insts.end(), bin.Inst()) != vec_insts.end()) {
      bin.deleteInst();
    }
  }
}

void BGM::cleanKM() {
  vector<long long>().swap(w_inst);
  vector<long long>().swap(w_bin);
  vector<int>().swap(c_inst);
  vector<int>().swap(c_bin);
  vector<bool>().swap(vis_inst);
  vector<bool>().swap(vis_bin);
  vector<vector<long long>>().swap(Map);
}
