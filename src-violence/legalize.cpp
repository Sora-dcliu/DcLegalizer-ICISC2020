#include <math.h>

#include "global.h"

#define LOG cout<<"[LG] "

void Bin::addCell(shared_ptr<cell>& inst) {
	this->Insts_.insert(inst);
	for(auto candi : this->candis_) {
		candi->parent()->cancelCandidateCell(candi);
	}
}

Legalize::Legalize() {
	LOG<<"Init legalizer."<<endl;
	//align the grid and classify the cells
	for(auto& inst : CELLS) {
		//Align the cell to the grid
		int mismatch = inst->lx() % 8;
		if(mismatch < 4 && mismatch > 0) inst->setLoc(inst->lx() - mismatch, inst->ly());
		else if(mismatch > 4) inst->setLoc(inst->lx() + 8 - mismatch, inst->ly());
		//Sorting and sorting
		if(inst->isMacro()) this->sortedMacros.insert(make_pair(inst->height(), inst));
		else this->stdCells.push_back(inst);
	}
	LOG<<"Preconsider the search order."<<endl;
	this->preConsiderSearchOrder();
}

void Legalize::preConsiderSearchOrder() {//Take a symmetrical 45 degree Angle, (8,8) as a search unit
	for(int i = 1; i < max(Col_cnt, Row_cnt); i++) {
		for(int j = 0; j <= i; j++) {
			long long cost = pow(i * 8, 2) + pow(j * 8, 2);
			this->SearchOrder.insert(make_pair(cost, j));
			if(i != j) this->SearchOrder.insert(make_pair(cost, i));
		}
	}
}

void Legalize::doLegalize() {
	LOG<<"Do legalization."<<endl;
	//Macros
	LOG<<"[Macro] Searching legal location."<<endl;
	for(auto& inst_pair : this->sortedMacros) {
		auto& inst = inst_pair.second;
		//Just consider the larger cells
		if(this->searchLegalPos_Macro(inst)) {
			inst->addCandidateCellsToGrid();
		}
		else {
			cout<<"Can't find legal location."<<endl;
			exit(0);
		}
	}
	//Just pick the first one for now
	for(auto& inst_pair : this->sortedMacros) {
		auto& inst = inst_pair.second;
		if(inst->CandidateCells().size() > 1) {
			auto& candi = *(inst->CandidateCells().begin());
			inst->setLoc(candi->lx(), candi->ly());
		}
	}
}

bool Legalize::searchLegalPos_Macro(shared_ptr<cell>& inst) {
	int center_col = inst->lx() / 8;
	searchCol centerSC(center_col, inst);
	if(centerSC.isLegal()) {
		inst->addCandidateCellsToGrid();
		return true;
	}
	else {
		map<int, searchCol> Space;//search space
		Space.insert(make_pair(0, centerSC));
		//Use the preprocessed sequence
		for(auto& loc_pair : this->SearchOrder) {//loc = (cost, SC)
			int col = loc_pair.second;
			if(col + center_col >= Col_cnt && center_col - col < 0) break;
			if(Space.find(col) == Space.end()) {//need to create new search columns
				//create a new search column
				if(col + center_col < Col_cnt) {
					searchCol newSC(col + center_col, inst);
					if(newSC.isLegal()) return true;
					Space.insert(make_pair(col, newSC));
				}
				//Columns symmetric about the center
				if(col != 0 && center_col - col >= 0) {
					searchCol newSC(-col + center_col, inst);
					if(newSC.isLegal()) return true;
					Space.insert(make_pair(-col, newSC));
				}
			}
			else {
				//extend the existing search columns
				auto& SC1 = Space.find(col)->second;
				if(SC1.Extend()) return true;
				if(col != 0) {
					auto& SC2 = Space.find(-col)->second;
					if(SC2.Extend()) return true;
				}
			}
		}
	}
	return false;
}

//for search leagal position
searchCol::searchCol(int col, shared_ptr<cell>& inst) {
	//the initial size of search column is the same as cell
	this->inst_ 	= inst;
	this->col_ 		= col;
	this->start_y_ 	= inst->ly();
	this->end_y_ 	= inst->uy()-1;
	this->deadCol = false;
	if(!Grid[col_][start_y_].isOccupied() && !Grid[col_][end_y_].isOccupied()) {
		this->legal = true;
		auto candi = make_shared<candiCell>(inst, col * 8, start_y_);
		inst->addCandidateCell(candi);
		return;
	}
	else this->legal = false;

	this->inst_ 	= inst;
	this->deadPart 	= pair<int, int>(-1,-1);
	//mark the dead part or is legal
	for(int y = start_y_; y < end_y_; y++) {
		if(Grid[col_][y].isOccupied()){
			if(deadPart.first == -1) {
				deadPart.first = y;
			}
			deadPart.second = y;
		}
	}
}

bool searchCol::Extend() {
	if(this->deadCol) return false;
	//down
	for(int i = 1; i <= 8; i++) {
		int y = this->start_y_ - i;
		if(y <= 0) break;
		if(Grid[this->col_][y].isOccupied()) {
			deadPart.first = y;
		}
		else {
			if(deadPart.first - y >= this->inst_->height()) {
				auto candi = make_shared<candiCell>(this->inst_, this->col_ * 8, y);
				this->inst_->addCandidateCell(candi);
				this->legal = true;
				return true;
			}
		}
	}
	start_y_ = max(0, start_y_ - 8);
	//up
	for(int i = 1; i <= 8; i++) {
		int y = this->end_y_ + i;
		if(y > max_y) break;
		if(Grid[this->col_][y].isOccupied()) {
			deadPart.second = y;
		}
		else {
			if(y - deadPart.second >= this->inst_->height()) {
				auto candi = make_shared<candiCell>(this->inst_, this->col_ * 8, deadPart.second + 1);
				this->inst_->addCandidateCell(candi);
				this->legal = true;
				this->legal = true;
				return true;
			}
		}
	}
	end_y_ = min(max_y, end_y_ + 8);
	if(this->deadPart.first < this->inst_->height()) this->deadPart.first = 0;
	if(max_y - this->deadPart.second < this->inst_->height()) this->deadPart.second = max_y;
	if(this->deadPart.first == 0 && this->deadPart.second == max_y) this->deadCol = true;
	return false;
}

void Legalize::printOverlapBins() {
	cout<<"Overlap Bins: ";
	for(auto& col : Grid) {
		for(auto& bin : col) {
			if(bin.isOverlap()) cout<<"("<<(&col - &Grid[0]) * 8<<","<<&bin - &col[0]<<") ";
		}
	}
	cout<<endl;
}
