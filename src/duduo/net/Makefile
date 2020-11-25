# ********************First of All****************************************
MUDUO_DIR ?= $(HOME)/build/release-install-cpp11
MUDUO_INCLUDE ?= $(MUDUO_DIR)/include
MUDUO_LIB ?= $(MUDUO_DIR)/lib

SRC=./Reactor

CPPFLAGS = -g -Og -Wall -Wextra \
					-march=native -rdynamic

LDFLAGS = -L$(MUDUO_LIB) -lmuduo_net -lmuduo_base -lpthread -lrt

# ********************Dependency List****************************************

ReactorDependency = $(SRC)/EventLoop.cc $(SRC)/Channel.cc $(SRC)/Poller.cc $(SRC)/main_timer.cc $(SRC)/Timer.cc $(SRC)/TimerQueue.cc

all : reactor
	# g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

reactor : $(ReactorDependency)
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean all
	
clean :
	rm -rf reactor
