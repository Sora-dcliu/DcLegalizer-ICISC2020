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
fig = plt.figure()
fig.tight_layout()

#ax1 - original position
ax1 = fig.add_subplot(1,2,1)
ax1.set_title('INPUT')
plt.xlim(minx, maxx)
plt.ylim(miny, maxy)
plt.xticks(x_tick)
plt.yticks(y_tick)
plt.grid()

#ax2 - result solution
ax2 = fig.add_subplot(1,2,2)
ax2.set_title('SOLUTION')
plt.xlim(minx, maxx)
plt.ylim(miny, maxy)
plt.xticks(x_tick)
plt.yticks(y_tick)
plt.grid()

#draw cells
for cell in cells:
	ax1.add_patch(plt.Rectangle((cell.oldlx, cell.oldly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))
	ax2.add_patch(plt.Rectangle((cell.lx, cell.ly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))

plt.suptitle('Cost:' + str(cost))
plt.show()