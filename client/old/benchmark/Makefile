DEFINES=-DMAIN -DDEBUG
#DEFINES=-DMAIN
CXXFLAGS=-g -I/home/y/include $(DEFINES)
LDFLAGS=-L/home/y/lib -lylem_Client -lcurl

all: ylem_benchmark

test: all
	./ylem_benchmark

ylem_benchmark: ylem_benchmark.cc
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f ylem_benchmark

