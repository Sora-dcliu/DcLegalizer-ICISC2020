#include "global.h"

#define LOG cout << "[INFO] "

using namespace std;

void getInput(const string& inputFile) {
  ifstream input(inputFile);
  if (!input) {
    cerr << "Error - no input file." << endl;
  } else
    LOG << "Begin reading input file." << endl;
  string line;
  getline(input, line);
  sscanf(line.c_str(), "%d %d %d", &Row_cnt, &Col_cnt, &Cell_cnt);
  LOG << "Grid-size: " << Row_cnt << " X " << Col_cnt << endl;
  LOG << "Cell-num : " << Cell_cnt << endl;
  for (int i = 0; i < Cell_cnt; i++) {
    getline(input, line);
    int height, lx, ly;
    sscanf(line.c_str(), "%d %d %d", &height, &lx, &ly);
    shared_ptr<cell> newcell = make_shared<cell>(i, height, lx, ly);
    CELLS.push_back(newcell);
  }
  input.close();
}

void Output(const string& outputFile) {
  LOG << "Outputing." << endl;
  ofstream output(outputFile);
  for (auto cell : CELLS) {
    output << cell->lx() << " " << cell->ly() << endl;
  }
  output.close();
}