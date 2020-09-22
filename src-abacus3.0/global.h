#ifndef _GLOBAL_
#define _GLOBAL_

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cell.h"

// global variable
extern int Row_cnt;   // number of Rows - M
extern int Col_cnt;   // number of columns - N
extern int Cell_cnt;  // number of cells

extern vector<shared_ptr<cell>> CELLS;

// global function
void getInput(const string& inputFile);
void Output(const string& outputFile);
long long getTotalCost();
long long getTotalMacroCost();
void checkLegal();
#endif