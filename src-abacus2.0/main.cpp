#include <chrono>

#include "global.h"
#include "legalize.h"

using namespace chrono;

#define LOG cout << "[INFO] "
// Variable initialization
int Row_cnt;
int Col_cnt;
int Cell_cnt;
vector<shared_ptr<cell>> CELLS;

int main(int argc, char* argv[]) {
  auto start = system_clock::now();
  try {
    if (argc != 3) throw "Eror - wrong number of command-line arguments.";
    string inputFile = argv[1];
    string outputFile = argv[2];
    // Reading inoput file
    getInput(inputFile);
    Legalize leg;
    leg.doLegalize();
    long long cost = getTotalCost();
    LOG << "Displacement cost: " << cost << endl;
    while (leg.reFind()) {
      long long curCost = getTotalCost();
      double impRatio = 1.0 * (cost - curCost) / cost;
      LOG << "Displacement cost: " << curCost << " improve: " << impRatio * 100 << "%" << endl;
      if (impRatio * 100 < 0.1) break;
      cost = curCost;
    }
    leg.refinement();
    // dump output file
    Output(outputFile);
    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    LOG << "Run time: " << double(duration.count()) / microseconds::period::den << "(s)." << endl;
    LOG << "Total cost - " << getTotalCost() << endl;
    checkLegal();
  } catch (const char* msg) {
    cerr << msg << endl;
    exit(1);
  }
  return 0;
}