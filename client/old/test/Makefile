# Makefile for unit-tests

SRC=$(wildcard *.cc)
TARGET=runtest
CXXFLAGS+=-Wall -g -O2 -I. -I../src/common -I /usr/local/include -I /home/y/include `cppunit-config --cflags` -ftemplate-depth-40
LIBS=-L../src/common -L/home/y/lib -lylem_Client -lysys `cppunit-config --libs`
SHFLAGS=-Wl,-rpath ../src/common

all: $(TARGET)

test: $(TARGET)
	-@./$(TARGET)

$(TARGET):$(SRC) ../src/common/libylem_Client.so
	$(CXX) $(CXXFLAGS) $(SHFLAGS) $(SRC) -o $@ $(LIBS)

clean:
	-@rm $(TARGET)

