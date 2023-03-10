# muduo library
MUDUO_DIRECTORY ?= /usr
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/local/lib

# project
SRC = $(wildcard ../ConnectServer/*.cpp ../Common/*.cpp ../ClientServer/*.cpp ../AppServer/*.cpp)

CXXFLAGS = -g -O0 -Wall -Wextra \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic \
	   -I$(MUDUO_INCLUDE)

LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread -lrt -lmysqlclient

chatServer: $(SRC)
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


all: chatServer
clean:
	rm -f chatServer core ./*.log


.PHONY: all clean
