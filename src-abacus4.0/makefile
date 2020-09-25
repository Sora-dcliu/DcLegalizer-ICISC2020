CC=g++
TARGET=main
SRC=$(shell echo *.cpp)
OBJ=$(SRC:$.cpp=$.o)
CFLAGS= -std=c++11
.PHONY:clean

debug:
	$(CC) -g -c $(SRC) $(CFLAGS)
	$(CC) -o $(TARGET) $(OBJ)
release:
	$(CC) -c -O3 $(SRC) $(CFLAGS)
	$(CC) -o $(TARGET) $(OBJ)
	rm -fr *.o
clean:
	rm -fr *.o
	rm main
