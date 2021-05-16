CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 
LIBS = /opt/etherlab/lib/libethercat.a -lrt -lpthread -lm
CLEANFILES = core core.* *.core *.o temp.* *.out typescript* \
		*.lc *.lh *.bsdi *.sparc *.uw

PROGS = share_mem_ser  share_mem_cli 
OBJS =  ./buffer/*.cpp ./log/*.cpp

all: $(PROGS)
	
share_mem_ser:share_mem_ser.o 
	$(CXX) $(CFLAGS)  $(OBJS) -o $@ share_mem_ser.o ${LIBS}

share_mem_cli:share_mem_cli.o
	$(CXX) $(CFLAGS) $(OBJS) -o $@ share_mem_cli.o ${LIBS}

clean:
	rm -rf /$(PROGS) $(CLEANFILES)
