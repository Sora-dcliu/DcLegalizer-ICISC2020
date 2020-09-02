import matplotlib.pyplot as plt
import sys
import numpy as np
import math

class cell:
	def __init__(self, height, oldlx, oldly):
		self.height = height
		self.width = 8
		self.oldlx = oldlx
		self.oldly = oldly
	def setLoc(self, lx, ly):
		self.lx = lx
		self.ly = ly
	def color(self):
		if(self.height > 1): return 'r'
		else: return 'b'
	def cx(self):
		return self.lx + 8/2
	def cy(self):
		return self.ly + self.height/2

inputFile  = '../testcase/' + sys.argv[1] + '.in'
resultFile = '../output/' + sys.argv[1] + '.out'

cells = []
first_line = 0
minx = 100000000
maxx = 0
miny = minx
maxy = 0
for line in open(inputFile):
	if(first_line == 0):
		first_line = 1
		continue
	info = line.split(' ')
	inst = cell(int(info[0]), int(info[1]), int(info[2]))
	cells.append(inst)
	maxx = max(inst.oldlx + 8, maxx)
	minx = min(inst.oldlx, minx)
	maxy = max(inst.oldly + inst.height, maxy)
	miny = min(inst.oldly, miny)

i = 0
cost = 0
for line in open(resultFile):
	info = line.split(' ')
	lx = int(info[0])
	ly = int(info[1])
	cells[i].setLoc(lx,ly)
	cost += cells[i].height * (pow((cells[i].lx - cells[i].oldlx),2) + pow((cells[i].ly - cells[i].oldly),2))
	maxx = max(lx + 8, maxx)
	minx = min(lx, minx)
	maxy = max(ly + cells[i].height, maxy)
	miny = min(ly, miny)
	i+=1

x_tick = []
y_tick = []
for x in range(int(minx/8)-1, int(maxx/8)+1):
	x_tick.append(x * 8)

for y in range(int(miny/8)-1, int(maxy/8)+1):
	y_tick.append(y * 8)

#plot
#grid setting
fig = plt.figure(figsize=(8, 8))
fig.tight_layout()

#ax - result solution
ax = fig.add_subplot(1,1,1)
plt.xlim(minx, maxx)
plt.ylim(miny, maxy)
plt.xticks(x_tick)
plt.yticks(y_tick)
plt.grid(linewidth = 0.05)
plt.tick_params(labelsize = 2.5)
#draw cells
i=0
for cell in cells:
	ax.add_patch(plt.Rectangle((cell.lx, cell.ly), cell.width, cell.height, facecolor = cell.color(),  alpha = 0.4))
	X = [cell.oldlx, cell.lx]
	Y = [cell.oldly, cell.ly]
	ax.plot(X, Y, linewidth = 0.1, color = 'black', linestyle = '--')
	ax.text(cell.cx(), cell.cy(), s = str(i), fontsize = 0.25)
	i+=1

plt.suptitle('Cost:' + str(cost))
plt.savefig(fname = "../output/" + sys.argv[1]+".svg",format="svg")
#plt.show()