#ifndef _CELL_H_
#define _CELL_H_

using namespace std;

class cell {
public:
	cell(int idx, int height, int lx, int ly):
		idx_(idx), height_(height), old_lx_(lx), lx_(lx), old_ly_(ly), ly_(ly) {};

	int oldlx()		{return old_lx_;};
	int oldly()		{return old_ly_;};

	int lx() 		{return lx_;};
	int ux() 		{return lx_ + 8;};
	int ly() 		{return ly_;};
	int uy() 		{return ly_ + height_;};
	int height() 	{return height_;};

	int idx() {return idx_;};

	void setLoc(int lx, int ly) {lx_ = lx; ly_ = ly;};
private:
	int idx_;
	int old_lx_, old_ly_;
	int lx_, ly_;
	int height_;
};

#endif