# CSCE 438: Distributed objects programming
# HW3
# 
# Makefile
# Vincent Velarde, Kyle Rowland
# March 2017
#
# NOTES:
#	"make" or "make all" to compile all parts
#	"clean" to remove everything but your source files
#

CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++ grpc`       \
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed \
           -lprotobuf -lpthread -ldl
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ./

vpath %.proto $(PROTOS_PATH)

all: client follower server

client: fb.pb.o fb.grpc.pb.o fbc.o
	$(CXX) $^ $(LDFLAGS) -o $@
	
follower: fb.pb.o fb.grpc.pb.o fbc2.o
	$(CXX) $^ $(LDFLAGS) -o $@

server: fb.pb.o fb.grpc.pb.o fbsd.o
	$(CXX) $^ $(LDFLAGS) -o $@

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client follower server UserList.txt
