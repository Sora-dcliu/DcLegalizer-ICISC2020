#include "global.h"

//Variable initialization
int Row_cnt;
int Col_cnt;
int Cell_cnt;
vector<shared_ptr<cell>> CELLS;
vector<vector<Bin>> Grid;

int main(int argc,char *argv[]) {
    try {
        if(argc != 3) throw "Eror - wrong number of command-line arguments.";
        string inputFile  = argv[1];
        string outputFile = argv[2];
        //Reading inoput file
        getInput(inputFile);
        initBins();
        Legalize Leg;
        Leg.doLegalize();
        //dump output file
        Output(outputFile);
    }
    catch(const char* msg) {
        cerr << msg << endl;
        exit(1);
    }
    return 0;
}