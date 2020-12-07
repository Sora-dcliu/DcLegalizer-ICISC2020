#include "bipartite.h"
#define LOG cout << "[LG] [BGM] "

const int size_limit_y = 100;
const int size_limit_x = 20;

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
      this->dfs(x, y, insts, INT_MAX, 0, INT_MAX, 0);
      if (insts.size() > 1) {
        int lx = max_x_;
        int ly = max_y_;
        int ux = 0;
        int uy = 0;
        for (auto& inst : insts) {
          lx = min(lx, inst->lx() / 8);
          ux = max(ux, inst->lx() / 8);
          ly = min(ly, inst->ly());
          uy = max(uy, inst->ly());
        }
        this->KM_match(lx, ly, ux, uy, insts);
      }
    }
  }
}

// Depth-first search clustering
void BGM::dfs(int x, int y, set<shared_ptr<cell>>& insts, int lx, int ux, int ly, int uy) {
  auto& bin = Grid[x][y];
  if (bin.isStd() && !bin.visited()) {
    insts.insert(bin.Inst());
    lx = min(lx, bin.Inst()->lx() / 8);
    ux = max(ux, bin.Inst()->lx() / 8);
    ly = min(ly, bin.Inst()->ly());
    uy = max(uy, bin.Inst()->ly());
    bin.mark();
    if (uy - ly >= size_limit_y || ux - lx >= size_limit_x) return;
    if (x - 1 >= 0) dfs(x - 1, y, insts, lx, ux, ly, uy);
    if (x + 1 <= max_x_) dfs(x + 1, y, insts, lx, ux, ly, uy);
    if (y - 1 >= 0) dfs(x, y - 1, insts, lx, ux, ly, uy);
    if (y + 1 <= max_y_) dfs(x, y + 1, insts, lx, ux, ly, uy);
  }
}

// KM algorithm
void BGM::KM_match(int lx, int ly, int ux, int uy, set<shared_ptr<cell>>& insts) {
  // init
  vector<shared_ptr<cell>> vec_insts;
  vec_insts.reserve(insts.size());
  for (auto& inst : insts) {
    vec_insts.push_back(inst);
  }
  int width = ux - lx + 1;
  int height = uy - ly + 1;
  cnt_inst = insts.size();
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

  Map.reserve(cnt_inst);
  Map.resize(cnt_inst);
  for (int i = 0; i < cnt_inst; i++) {
    auto& edge = Map[i];
    edge.reserve(cnt_bin);
    edge.resize(cnt_bin);
    for (int j = 0; j < cnt_bin; j++) {
      int x = lx + j / height;
      int y = ly + j % height;
      auto& bin = this->Grid[x][y];
      if (bin.isMacro() || insts.find(bin.Inst()) == insts.end()) {
        edge[j] = LLONG_MAX;
      } else {
        edge[j] = bin.getCost(vec_insts[i]);
        w_inst[i] = min(w_inst[i], edge[j]);
      }
    }
  }
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
  // update location
  for (int i = 0; i < cnt_bin; i++) {
    int x = lx + i / height;
    int y = ly + i % height;
    auto& bin = this->Grid[x][y];
    if (bin.isMacro()) continue;
    if (c_bin[i] != -1) {
      auto& inst = vec_insts[c_bin[i]];
      bin.setInst(inst);
      inst->setLoc(x * 8, y);
    } else if (insts.find(bin.Inst()) != insts.end())
      bin.deleteInst();
  }
  cleanKM();
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

void BGM::cleanKM() {
  vector<long long>().swap(w_inst);
  vector<long long>().swap(w_bin);
  vector<int>().swap(c_inst);
  vector<int>().swap(c_bin);
  vector<bool>().swap(vis_inst);
  vector<bool>().swap(vis_bin);
  vector<vector<long long>>().swap(Map);
}
