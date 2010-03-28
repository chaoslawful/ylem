CC := g++
AppName := ylemClient

FLAGS := -g -Wall
CXXFLAGS := $(CFLAGS)

SRC = $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp)

LINC := -I/home/y/include/ -I../src/common
LLDLIBS := -Wl,-L/usr/local/lib -L../src/common -lylem_Client

OBJS := $(patsubst %.cpp,%.o, $(patsubst %.c, %.o, $(patsubst %.cc, %.o, $(SRC))))

all: depend $(OBJS)
	$(CC) $(FLAGS) $(OBJS) $(LLDLIBS) -o $(AppName)

depend: $(SRC)
	@$(CC) $(LINC) $(FLAGS) -w -MM $^ > $@


ylemClient.o: ylemClient.cc
	$(CC) $(LINC) $(FLAGS) -c ylemClient.cc

include depend

.PHONY:clean rebuild check

clean:
	-rm $(AppName) $(OBJS) depend

rebuild: clean all

check:
	-make -t
