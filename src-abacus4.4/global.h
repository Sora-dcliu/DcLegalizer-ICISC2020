#ifndef _GLOBAL_
#define _GLOBAL_

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cell.h"
#include "limits.h"

using namespace chrono;

extern std::chrono::system_clock::time_point start;

// global variable
extern int Row_cnt;   // number of Rows - M
extern int Col_cnt;   // number of columns - N
extern int Cell_cnt;  // number of cells

extern vector<shared_ptr<cell>> CELLS;

// global function
void getInput(const string& inputFile);
void Output(const string& outputFile);
void WriteGds(const string& filename);
long long getTotalCost();
bool checkLegal();
void readOutFile(const string& filename);
#endif