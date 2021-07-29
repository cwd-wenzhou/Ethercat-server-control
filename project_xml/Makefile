CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

PROGS = prasexml
OBJS = ./tinyxml2.cpp

all: $(PROGS)
	
prasexml:prasexml.o tinyxml2.o
	$(CXX) $(CFLAGS)  $(OBJS) -o $@ prasexml.o 

clean:
	rm -rf /$(PROGS) $(CLEANFILES)