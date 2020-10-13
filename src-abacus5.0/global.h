#ifndef _GLOBAL_
#define _GLOBAL_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cell.h"
#include "limits.h"

using namespace std;

// global variable
extern int Row_cnt;     // number of Rows - M
extern int Col_cnt;     // number of columns - N
extern int Cell_cnt;    // number of cells
extern bool bufferMod;  // control buffer mode switch

extern vector<shared_ptr<cell>> CELLS;

// global function
void getInput(const string& inputFile);
void Output(const string& outputFile);
void WriteGds(const string& filename);
long long getTotalCost();
void checkLegal();

#endif