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

max_y = 0
max_x = 0

for line in open(resultFile):
	info = line.split(' ')
	lx = int(info[0])
	ly = int(info[1])
	cells[i].setLoc(lx,ly)
	cost += cells[i].height * (pow((cells[i].lx - cells[i].oldlx),2) + pow((cells[i].ly - cells[i].oldly),2))
	if(lx + 8 > max_x): max_x = lx + 8
	if(ly + cells[i].height > max_y): max_y = ly + cells[i].height
	i+=1

x_tick = []
y_tick = []
for x in range(0, int(max_x/8+0.5)+1):
	x_tick.append(x)

for y in range(0, int(max_y/8+0.5)):
	y_tick.append(y)


#plot
#grid setting
fig = plt.figure(figsize = (8 * max_x/max(max_x, max_y)*2+1, 8* max_y/max(max_x, max_y)))
fig.tight_layout()

""" #ax1 - original position
ax1 = fig.add_subplot(1,2,1)
ax1.set_title('INPUT')
plt.xlim(0, col_cnt)
plt.ylim(0, row_cnt)
plt.xticks(x_tick)
plt.yticks(y_tick)
plt.grid()
 """
#ax2 - result solution
ax2 = fig.add_subplot(1,1,1)
ax2.set_title('SOLUTION')
plt.xlim(0, int(max_x/8+0.5))
plt.ylim(0, int(max_y/8+0.5))
plt.xticks(x_tick)
plt.yticks(y_tick)
plt.axis('equal')
plt.grid()
plt.subplots_adjust(left=0.04, bottom=0.04, right=1, top=1, wspace=0, hspace=0)
#draw cells
for cell in cells:
	cell.oldlx 	/= 8.0
	cell.oldly	/= 8.0
	cell.lx    	/= 8.0
	cell.ly		/= 8.0
	cell.width	/= 8.0
	cell.height	/= 8.0
	#ax1.add_patch(plt.Rectangle((cell.oldlx, cell.oldly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))
	ax2.add_patch(plt.Rectangle((cell.lx, cell.ly), cell.width, cell.height, facecolor = cell.color(), edgecolor = 'black', linewidth = 2, alpha = 0.4))

plt.suptitle('Cost:' + str(cost))
plt.show()