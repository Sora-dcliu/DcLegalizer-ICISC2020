#!/bin/bash
set -e
for i in {1..100}
do
	cd script
	python3 gen_case.py
	cd ..
    ./src-abacus4.4/eda20819.exe testcase/15000.in output/15000.out
done