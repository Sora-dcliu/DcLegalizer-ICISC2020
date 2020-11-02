CC=g++
CURDIR=$(shell pwd)
TARGET=eda20819.exe
TARGETDIR=$(CURDIR)/../bin/$(TARGET)
SRC=$(shell echo *.cpp)
OBJ=$(SRC:$.cpp=$.o)
CFLAGS= -std=c++11
.PHONY:clean

all:
	$(CC) -c -O3 $(SRC) $(CFLAGS)
	$(CC) -o $(TARGETDIR) $(OBJ)
	rm -fr *.o
clean:
	rm -fr *.o
	rm main
