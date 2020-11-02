#include "global.h"
#include "legalize.h"

#define LOG cout << "[INFO] "
// Variable initialization
int Row_cnt;
int Col_cnt;
int Cell_cnt;
vector<shared_ptr<cell>> CELLS;
// start time
std::chrono::system_clock::time_point start;

int main(int argc, char *argv[]) {
  start = system_clock::now();
  try {
    if (argc != 3)
      throw "Eror - wrong number of command-line arguments.";
    string inputFile = argv[1];
    string outputFile = argv[2];
    // Reading inoput file
    getInput(inputFile);

    // do legalizaion
    Legalize leg;
    leg.doLegalize();

    // dump output file
    Output(outputFile);
    // WriteGds(outputFile + ".gds");
    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    LOG << "Run time: " << double(duration.count()) / microseconds::period::den
        << "(s)." << endl;
    // LOG << "Total cost - " << getTotalCost() << endl;
    // checkLegal();
  } catch (const char *msg) {
    cerr << msg << endl;
    exit(1);
  }
  return 0;
}