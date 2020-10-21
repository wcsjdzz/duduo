MUDUO_DIR ?= $(HOME)/build/release-install-cpp11
MUDUO_INCLUDE ?= $(MUDUO_DIR)/include
MUDUO_LIB ?= $(MUDUO_DIR)/lib

SRC=../src

CPPFLAGS = -g -Og -Wall -Wextra \
					-march=native -rdynamic

LDFLAGS = -L$(MUDUO_LIB) -lmuduo_net -lmuduo_base -lpthread -lrt

all: prog_self_defined prog_download_server_myself_version1 prog_download_server_official_version1

clean:
	rm -f prog_self_defined core prog_download_server_myself_version1 prog_download_server_official_version1

prog_self_defined: $(SRC)/echo_myself.cc $(SRC)/daytime_myself.cc $(SRC)/server_main.cpp
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_myself_version1: $(SRC)/download_myself.cpp
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_official_version1: $(SRC)/download.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: all clean


