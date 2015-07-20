# Copyright 2015 The Ostrich

CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib -lgrpc++_unsecure -lgrpc -lgpr -lprotobuf
LDFLAGS += -lpthread -ldl -lglog -lgflags
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../common/protos

vpath %.proto $(PROTOS_PATH)

all: api_server query_loop

api_server: api_experiment.pb.o api_experiment.grpc.pb.o api.o
	$(CXX) $^ $(LDFLAGS) -o $@

query_loop: api_experiment.pb.o api_experiment.grpc.pb.o query_loop.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. \
		--plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client api_server
