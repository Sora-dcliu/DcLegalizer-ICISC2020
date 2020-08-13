#include <climits>
#include <cmath>

#include "legalize.h"

#define LOG cout<<"[LG] "

Cluster::Cluster(shared_ptr<cell>& inst, int idx) {
	this->idx_ 			= idx;
	this->cells_.push_back(inst);
	this->ly_		 	= inst->oldly();
	this->Qc_ 			= inst->height() * (inst->oldly());
	this->totalHeight_	= inst->height();
}

void Cluster::addCell(shared_ptr<cell>& inst) {
	this->cells_.push_back(inst);
	this->Qc_		   += inst->height() * (inst->oldly() - this->totalHeight_);
	this->totalHeight_ += inst->height();
}

void Cluster::addCluster(Cluster& c) {
	auto& c_cells = c.cells();
	this->cells_.insert(this->cells_.end(), c_cells.begin(), c_cells.end());
	this->Qc_		   += c.Qc() - c.totalHeight() * this->totalHeight_;
	this->totalHeight_ += c.totalHeight();
}

void Col::collapse(Cluster& c) {
	c.setLy(round(1.0 * c.Qc() / c.totalHeight()));
	if(c.ly() < 0) c.setLy(0);
	if(c.uy() > Row_cnt * 8 -1) c.setLy(Row_cnt * 8 -1 - c.totalHeight());
	//if overlap with predecessor c'
	if(c.idx() > 0) {
		auto& pre_c = this->Clusters_[c.idx()-1];
		if(pre_c.uy() > c.ly()) {//cluster overlap
			pre_c.addCluster(c);
			this->Clusters_.erase(this->Clusters_.begin()+c.idx());
			collapse(pre_c);
		}
	}

}

long long Col::PlaceCol(shared_ptr<cell>& inst) {
	if(this->Clusters_.size() == 0 || this->Clusters_.rbegin()->uy() <= inst->oldly()) {
		//create a new cluster
		Cluster c(inst, this->Clusters_.size());
		this->Clusters_.push_back(c);
	}
	else {
		auto& c =this->Clusters_[this->Clusters_.size()-1];
		c.addCell(inst);
		this->collapse(c);
	}
	//caculate the cost
	long long cost = 0;
	for(auto& c : this->Clusters_) {
		int curLy = c.ly();
		for(auto& inst_c : c.cells()) {
			int curLx = this->idx_ * 8;
			if(inst_c == inst) cost += inst->height() * (pow(inst->oldlx() - curLx, 2) + pow(inst->oldly() - curLy, 2));
			else {
				if(abs(curLy - inst_c->oldly()) > abs(inst_c->ly() - inst_c->oldly()))//further from original position
					cost += inst_c->height() * pow(inst_c->ly() - curLy, 2);
				else //closer from original position
					cost -= inst_c->height() * pow(inst_c->ly() - curLy, 2);
			}
			curLy += inst->height();
		}
	}
	return cost;
}

Legalize::Legalize() {
	//general rows
	for(int i = 0; i < Col_cnt; i++) {
		COLS.push_back(Col(i));
	}
	//sort cells in y order
	for(auto& inst : CELLS) {
		this->sortedCells.insert(make_pair(inst->ly(), inst));
	}
}

void Legalize::doLegalize() {
	for(auto& cell_pair : this->sortedCells) {
		auto& inst = cell_pair.second;
		long long bestCost = LLONG_MAX;
		Col bestCol;
		int nearestCol = round(1.0 * inst->lx()/8);
		bool left = true, right = true;
		for(int i = 0; left || right; i++) {
			//left
			if(nearestCol - i >=0 && left) {
				auto curCol = COLS[nearestCol - i];
				long long curCost = curCol.PlaceCol(inst);
				if(curCost < bestCost) {
					bestCost = curCost;
					bestCol  = curCol;
				}
				else if(curCost > bestCost) left = false;
			}
			else if(nearestCol - i < 0) left = false;

			//right
			if(nearestCol + i < Col_cnt && right && i != 0) {
				auto curCol = COLS[nearestCol + i];
				long long curCost = curCol.PlaceCol(inst);
				if(curCost < bestCost) {
					bestCost = curCost;
					bestCol  = curCol;
				}
				else if(curCost > bestCost) right = false;
			}
			else if(nearestCol + i >= Col_cnt) right = false;

		}
		COLS[bestCol.idx()] = bestCol;
		//set final position
		for(auto& c : bestCol.Clusters()) {
			int curLy = c.ly();
			for(auto& inst : c.cells()) {
				inst->setLoc(bestCol.idx() * 8, curLy);
				curLy += inst->height();
			}
		}
	}
}