#ifndef _GLOBAL_
#define _GLOBAL_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "cell.h"
#include "legalize.h"

//global variable
extern int Row_cnt; //number of Rows - M
extern int Col_cnt; //number of columns - N
extern int Cell_cnt; //number of cells

extern vector<shared_ptr<cell>> CELLS;
extern vector<vector<Bin>> Grid;

//global function
void addInstToGrid(shared_ptr<cell>& inst);

void getInput(const string& inputFile);
void Output(const string& outputFile);
void initBins();
#endif