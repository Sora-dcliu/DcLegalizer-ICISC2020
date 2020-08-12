#ifndef _CELL_H
#define _CELL_H

#include <unordered_set>

using namespace std;

class candiCell;

class cell {
public:
	cell(){};
	cell(int idx, int height, int lx , int ly):
		height_(height), width_(8),
		lx_(lx), ly_(ly),
		orig_lx_(lx), orig_ly_(ly)
		{};

	int lx() {return lx_;};
	int ly() {return ly_;};
	int ux() {return lx_ + 8;};
	int uy() {return ly_ + height_;};
	int height() {return height_;};

	void addToGrid();
	void addCandidateCell(shared_ptr<candiCell>& candi){candidateCells_.insert(candi);};
	void addCandidateCellsToGrid();
	unordered_set<shared_ptr<candiCell>>& CandidateCells() {return candidateCells_;};
	void cancelCandidateCell(shared_ptr<candiCell>& candi);

	void setLoc(int lx, int ly) {lx_ = lx; ly_ = ly;};

	bool isMacro() {return (height_ > 1);};

protected:
	int height_, width_;
	int lx_, ly_;

private:
	int idx_;
	int orig_lx_, orig_ly_;

	unordered_set<shared_ptr<candiCell>> candidateCells_; //positive positions
};

class candiCell: public cell {
public:
	candiCell(shared_ptr<cell>& inst, int lx, int ly);
	shared_ptr<cell>& parent() {return parent_;};
private:
	shared_ptr<cell> parent_;
};

#endif