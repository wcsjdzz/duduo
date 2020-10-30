MUDUO_DIR ?= $(HOME)/build/release-install-cpp11
MUDUO_INCLUDE ?= $(MUDUO_DIR)/include
MUDUO_LIB ?= $(MUDUO_DIR)/lib

SRC=../src

CPPFLAGS = -g -Og -Wall -Wextra \
					-march=native -rdynamic

LDFLAGS = -L$(MUDUO_LIB) -lmuduo_net -lmuduo_base -lpthread -lrt


# FIXME: download server is now commented, better find a new solution;
# all: prog_self_defined prog_download_server_myself_version1 prog_download_server_official_version1 prog_download_server_myself_version2 prog_download_server_offcial_version2 prog_download_server_myself_version3
all: prog_asio_server_myself prog_asio_client_myself

clean:
	rm -f prog_self_defined core prog_download_server_myself_version1 prog_download_server_myself_version2 prog_download_server_official_version2 prog_download_server_official_version1 prog_download_server_myself_version3 prog_asio_server_myself prog_asio_client_myself

prog_self_defined: $(SRC)/echo_myself.cc $(SRC)/daytime_myself.cc $(SRC)/server_main.cpp
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_myself_version1: $(SRC)/download_myself.cpp
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_official_version1: $(SRC)/download.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)
	
prog_download_server_myself_version2: $(SRC)/download2_myself.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_official_version2: $(SRC)/download2.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_download_server_myself_version3: $(SRC)/download3_myself.cc
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)


prog_asio_server_myself: $(SRC)/asio/chat_server_myself.cc $(SRC)/asio/LengthHeaderCodec.h
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

prog_asio_client_myself: $(SRC)/asio/chat_client_myself.cc $(SRC)/asio/LengthHeaderCodec.h
	g++ $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: all clean


