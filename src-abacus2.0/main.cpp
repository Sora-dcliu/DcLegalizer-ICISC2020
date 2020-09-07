#include "global.h"
#include "legalize.h"

#define LOG cout << "[INFO] "
// Variable initialization
int Row_cnt;
int Col_cnt;
int Cell_cnt;
vector<shared_ptr<cell>> CELLS;

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) throw "Eror - wrong number of command-line arguments.";
    string inputFile = argv[1];
    string outputFile = argv[2];
    // Reading inoput file
    getInput(inputFile);
    Legalize leg;
    leg.doLegalize();
    leg.reFind();
    leg.refinement();
    long long cost = leg.getTotalCost();
    LOG << "Total cost - " << cost << endl;

    leg.checkLegal();
    // dump output file
    Output(outputFile);
  } catch (const char* msg) {
    cerr << msg << endl;
    exit(1);
  }
  return 0;
}