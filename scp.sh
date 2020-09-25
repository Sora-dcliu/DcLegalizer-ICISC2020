#!/bin/bash
fs=""
for f in ./$1/*.cpp ./$1/*.h ./$1/makefile
do
	fs="$fs $f"
done 
	expect -c "
	spawn scp $fs eda20819@10.100.2.15:~/dcleg
	expect {
	    \"*assword\" 
	                {
	                    send \"ss8713657\r\";
	                }
	    \"yes/no\" 
	                {
	                    send \"yes\r\"; exp_continue;}
	                }
	expect eof"