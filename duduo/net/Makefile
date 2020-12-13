# ********************First of All****************************************
MUDUO_DIR ?= $(HOME)/build/release-install-cpp11
MUDUO_INCLUDE ?= $(MUDUO_DIR)/include
MUDUO_LIB ?= $(MUDUO_DIR)/lib


CPPFLAGS = -g -Og -Wall -Wextra \
					-march=native -rdynamic

LDFLAGS = -L$(MUDUO_LIB) -lmuduo_net -lmuduo_base -lpthread -lrt

# ********************Dependency List****************************************

SRC=./Reactor
ReactorDependency = $(SRC)/EventLoop.cc $(SRC)/Channel.cc $(SRC)/Poller.cc $(SRC)/Timer.cc $(SRC)/TimerQueue.cc $(SRC)/EventLoopThread.cc $(SRC)/EventLoopThreadPool.cc

timerDependency = $(ReactorDependency) $(SRC)/main_timer.cc

acptSRC=./Server
acptDependency = $(ReactorDependency) $(acptSRC)/Acceptor.cc $(acptSRC)/Socket.cc $(acptSRC)/SockOptions.cc 

serverSRC=./Server
serverDependency = $(acptDependency) $(serverSRC)/TcpServer.cc $(serverSRC)/TcpConnection.cc

all : timer acceptor server1 server_muduo
	# g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

timer : $(ReactorDependency) $(SRC)/main_timer.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

acceptor : $(acptDependency) $(acptSRC)/main_Acceptor.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

server1 : $(serverDependency) $(serverSRC)/main_server_version1.cc 
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

server_muduo : $(serverSRC)/main_server_version1.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean all
	
clean :
	rm -rf reactor acceptor server1 timer server_muduo
