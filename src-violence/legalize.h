#ifndef _LEGALIZE_H
#define _LEGALIZE_H

#include <unordered_set>
#include <map>

#include "cell.h"

using namespace std;

class Bin {
public:
	Bin(){};

	void addCell(shared_ptr<cell> &inst);
	void deleteCell(shared_ptr<cell> &inst) {Insts_.erase(inst);};

	bool isCandidate() {return (candis_.size() > 0);};
	void addCandidateCell(shared_ptr<candiCell>& candi) {if(Insts_.find(candi->parent()) == Insts_.end()) candis_.insert(candi);};
	void deleteCandidateCell(shared_ptr<candiCell>& candi) {candis_.erase(candi);};

	bool isOverlap() 	{return (Insts_.size() > 1);};
	bool isOccupied() 	{return (Insts_.size() > 0);};

	unordered_set<shared_ptr<cell>>& Insts() {return Insts_;};
private:
	unordered_set<shared_ptr<cell>> Insts_;
	unordered_set<shared_ptr<candiCell>> candis_;
};

//for search
class searchCol {// a column for search
public:
	searchCol(int col, shared_ptr<cell>& inst);
	bool isLegal() {return legal;};

	bool Extend();
private:
	int col_;
	int start_y_, end_y_;
	shared_ptr<cell> inst_;
	//judge
	bool legal;
	bool deadCol;//Has been unable to extend
	pair<int,int> deadPart;
};

class Legalize {
public:
	Legalize();

	void doLegalize();

	void preConsiderSearchOrder();
	bool searchLegalPos_Macro(shared_ptr<cell>& inst);

	void printOverlapBins();
private:
	vector<shared_ptr<cell>> stdCells;//standard cell - height = 1
	multimap<int, shared_ptr<cell>, greater<int>> sortedMacros; //cells sorted in descending order of height
	multimap<long long, int> SearchOrder; // cost, search column
};
#endif