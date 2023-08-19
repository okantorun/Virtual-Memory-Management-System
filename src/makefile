CC = g++
CFLAGS = -std=c++11

TARGET = operateArrays

SRCS = main.cpp VirtualMemory.hpp PhysicalMemory.hpp PageTable.hpp InvertedPageTable.hpp SecondChanceFifo.hpp HardwareLRU.hpp WSClock.hpp

OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm operateArrays
	rm main.o
	rm diskFileName.dat
