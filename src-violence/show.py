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

row_cnt = 0
col_cnt = 0
cell_cnt = 0

cells = []

for line in open(inputFile):
	info = line.split(' ')
	if(row_cnt == 0):
		row_cnt = int(info[0])
		col_cnt = int(info[1])
		cell_cnt = int(info[2])
	else:
		cells.append(cell(int(info[0]), int(info[1]), int(info[2])))

i = 0
cost = 0
for line in open(resultFile):
	info = line.split(' ')
	lx = int(info[0])
	ly = int(info[1])
	cells[i].setLoc(lx,ly)
	cost += cells[i].height * (pow((cells[i].lx - cells[i].oldlx),2) + pow((cells[i].ly - cells[i].oldly),2))
	i+=1

x_tick = []
y_tick = []
for x in range(0, int(col_cnt/8)+1):
	x_tick.append(x * 8)
if(col_cnt % 8 > 0): x_tick.append(col_cnt)

for y in range(0, int(row_cnt / 8)+1):
	y_tick.append(y * 8)
if(row_cnt % 8 > 0): y_tick.append(row_cnt)


#plot
#grid setting
fig = plt.figure(figsize = (8 * col_cnt/max(col_cnt, row_cnt)*2+1, 8* row_cnt/max(col_cnt, row_cnt)))
fig.tight_layout()

#ax1 - original position
ax1 = fig.add_subplot(1,2,1)
ax1.set_title('INPUT')

plt.xticks(x_tick)
plt.yticks(y_tick)
plt.tick_params(width=0)
plt.axis('equal')
ax1.xaxis.set_ticks_position('bottom')
ax1.spines['bottom'].set_position(('data',0))
ax1.yaxis.set_ticks_position('left')
ax1.spines['left'].set_position(('data',0))
plt.grid()

#ax2 - result solution
ax2 = fig.add_subplot(1,2,2)
ax2.set_title('SOLUTION')

plt.xticks(x_tick)
plt.yticks(y_tick)
plt.tick_params(width=0)
plt.axis('equal')
ax2.xaxis.set_ticks_position('bottom')
ax2.spines['bottom'].set_position(('data',0))
ax2.yaxis.set_ticks_position('left')
ax2.spines['left'].set_position(('data',0))
plt.grid()

#draw cells
for cell in cells:
	ax1.add_patch(plt.Rectangle((cell.oldlx, cell.oldly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))
	ax2.add_patch(plt.Rectangle((cell.lx, cell.ly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))

plt.suptitle('Cost:' + str(cost))
plt.show()