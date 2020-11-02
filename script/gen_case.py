import random

num = 20000
macro_num =  int(num/18)
filename = '../testcase/' + str(num) + '.in'

f = open(filename,'w')

f.write('160 80 '+str(num)+'\n')

for i in range(0,macro_num):
	height = random.randint(2,128)
	x = random.randint(0,80*8-1)
	y = random.randint(0,160*8-1)
	f.write(str(height)+' '+str(x)+' '+str(y)+'\n')
for i in range(macro_num,num):
	x = random.randint(0,80*8-1)
	y = random.randint(0,160*8-1)
	f.write('1 '+str(x)+' '+str(y)+'\n')