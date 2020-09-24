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

void drawBox(int lx, int ux, int ly, int uy, int layer, ofstream& gds) {
  gds << "BOUNDARY" << endl
      << "LAYER " << layer << endl
      << "DATATYPE 0" << endl
      << "XY " << lx << ": " << ly << endl
      << ux << ": " << ly << endl
      << ux << ": " << uy << endl
      << lx << ": " << uy << endl
      << lx << ": " << ly << endl
      << "ENDEL" << endl;
}

void writeText(int x, int y, const string& str, float size, ofstream& gds) {
  gds << "TEXT" << endl
      << "LAYER 4" << endl
      << "TEXTTYPE 0" << endl
      << "PRESENTATION 11" << endl
      << "MAG " << to_string(size) << endl
      << "XY " << x << ": " << y << endl
      << "STRING " << str << endl
      << "ENDEL" << endl;
}

void WriteGds(const string& filename) {
  LOG << "Writting gds file." << endl;
  ofstream gds(filename);
  gds << "HEADER 600" << endl
      << "BGNLIB" << endl
      << "LIBNAME LIB" << endl
      << "UNITS 1 1e-09" << endl
      << "BGNSTR" << endl
      << "STRNAME TOP" << endl;
  // Boundary
  drawBox(0, Col_cnt * 8, 0, Row_cnt * 8, 0, gds);
  long long cost = getTotalCost();
  writeText(0, Row_cnt * 8, "COST: " + to_string(cost), 10, gds);
  // cells
  for (auto& inst : CELLS) {
    if (inst->isMacro())
      drawBox(inst->lx(), inst->ux(), inst->ly(), inst->uy(), 2, gds);
    else
      drawBox(inst->lx(), inst->ux(), inst->ly(), inst->uy(), 1, gds);
    writeText(inst->lx(), inst->ly(), to_string(inst->idx()), 0.01, gds);
    // path
    gds << "PATH" << endl
        << "LAYER 3" << endl
        << "DATATYPE 0" << endl
        << "PATHTYPE 2" << endl
        << "WIDTH 0.1" << endl
        << "XY " << inst->oldlx() << ": " << inst->oldly() << endl
        << inst->lx() << ": " << inst->ly() << endl
        << "PROPATTR 1" << endl
        << "PROPVALUE TOP" << endl
        << "ENDEL" << endl;
  }
  gds << "ENDSTR" << endl << "ENDLIB" << endl;
}