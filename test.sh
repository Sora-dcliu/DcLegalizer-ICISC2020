#!/bin/bash

for i in {1..100}
do
	cd script
	python3 gen_case.py
	cd ..
    ./bin/eda20819.exe testcase/20000.in output/20000.out
done