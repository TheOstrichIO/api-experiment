// Copyright 2015, The Ostrich

#include <chrono>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/status.h>

#include "api_experiment.grpc.pb.h"

DEFINE_string(server_address,
              "0.0.0.0:51051",
              "Server address `ip:port` to bind to");

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using api_experiment::BaseRequest;
using api_experiment::SleepRequest;
using api_experiment::TimestampReply;
using api_experiment::ApiExperiment;

class ApiExperimentImpl final : public ApiExperiment::Service {
  Status EchoTimestamp(ServerContext* context, const BaseRequest* request,
                       TimestampReply* reply) override {
    reply->set_req_id(request->req_id());
    microseconds ms = duration_cast<microseconds>(
        system_clock::now().time_since_epoch());
    reply->set_timestamp(ms.count());
    return Status::OK;
  }

  Status SleepAndEcho(ServerContext* context, const SleepRequest* request,
                      TimestampReply* reply) override {
    reply->set_req_id(request->req_id());
    microseconds ms = duration_cast<microseconds>(
        system_clock::now().time_since_epoch());
    reply->set_timestamp(ms.count());
    return Status::OK;
  }
};

void RunServer() {
  ApiExperimentImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(FLAGS_server_address,
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << FLAGS_server_address;
  server->Wait();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);
  RunServer();
  return 0;
}
