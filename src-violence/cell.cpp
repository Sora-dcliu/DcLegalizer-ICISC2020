#include "global.h"

void cell::addToGrid() {
	for(int y = this->ly_; y < this->ly_ + this->height_; y++) {
		Grid[this->lx_ / 8][y].addCell(CELLS[this->idx_]);
	}
}

void cell::addCandidateCellsToGrid() {
	//set loc if only one candidate
	if(this->candidateCells_.size() == 1) {
		auto& candi = *(candidateCells_.begin());
		this->lx_ = candi->lx();
		this->ly_ = candi->ly();
		this->addToGrid();
		this->candidateCells_.clear();
	}
	else {
		//find the common part of all candidate cells
		int com_lx = -1;
		int com_ly = -1;
		int com_uy = -1;
		for(auto& candi : this->candidateCells_) {
			if(com_lx == -1) com_lx = candi->lx();
			else if(com_lx != candi->lx()) {
				com_lx = -1;
				break;
			}
			if(com_ly == -1 || candi->ly() > com_ly) com_ly = candi->ly();
			if(com_uy == -1 || candi->uy() < com_uy) com_uy = candi->uy();
		}
		if(com_lx != -1 && com_ly <= com_uy) {
			for(int y = com_ly; y < com_uy; y++) {
				Grid[com_lx][y].addCell(CELLS[this->idx_]);
			}
		}
		//add to grid
		for(auto candi : this->candidateCells_) {
			for(int y = candi->ly(); y < candi->ly() + candi->height(); y++) {
				Grid[candi->lx() / 8][y].addCandidateCell(candi);
			}
		}
	}
}

void cell::cancelCandidateCell(shared_ptr<candiCell>& candi) {
	this->candidateCells_.erase(candi);
	for(int y = candi->ly(); y < candi->ly() + candi->height(); y++) {
		Grid[candi->lx() / 8][y].deleteCandidateCell(candi);
	}
	if(this->candidateCells_.size() == 1) {
		auto& candi = *(candidateCells_.begin());
		this->lx_ = candi->lx();
		this->ly_ = candi->ly();
		this->addToGrid();
		this->candidateCells_.clear();
	}
}

candiCell::candiCell(shared_ptr<cell>& inst, int lx, int ly) {
	this->parent_ 	= inst;
	this->lx_		= lx;
	this->ly_		= ly;
	this->height_ 	= inst->height();
	this->width_	= 8;
}