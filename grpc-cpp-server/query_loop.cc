// Copyright 2015, The Ostrich

#include <chrono>
#include <cstdint>
#include <cstdio>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpc/grpc.h>
#include <grpc/support/log.h>
#include <grpc++/async_unary_call.h>
#include <grpc++/channel_arguments.h>
#include <grpc++/channel_interface.h>
#include <grpc++/client_context.h>
#include <grpc++/completion_queue.h>
#include <grpc++/create_channel.h>
#include <grpc++/credentials.h>
#include <grpc++/status.h>

#include "api_experiment.grpc.pb.h"

DEFINE_int32(base_id,
             1,
             "Base value for request ID's");
DEFINE_int32(num_queries,
             5000,
             "Number of queries to make against backend");
DEFINE_string(backend,
              "localhost:51051",
              "Connection string `ip:port` for backend server");

using api_experiment::BaseRequest;
using api_experiment::TimestampReply;
using api_experiment::ApiExperiment;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using grpc::ChannelArguments;
using grpc::ChannelInterface;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

class ApiClient {
 public:
  explicit ApiClient(std::shared_ptr<ChannelInterface> channel)
      : stub_(ApiExperiment::NewStub(channel)) {}

  uint64_t EchoTimestamp(unsigned req_id) {
    BaseRequest request;
    request.set_req_id(req_id);
    TimestampReply reply;
    ClientContext context;
    CompletionQueue cq;

    Status status = stub_->EchoTimestamp(&context, request, &reply);
    VLOG(5) << "Got ID " << reply.req_id() << " in response to request ID "
            << req_id;
    if (status.ok() && reply.req_id() == req_id) {
      return reply.timestamp();
    } else {
      // TODO(itamar): Change this to exception?
      return 0;
    }
  }

 private:
  std::unique_ptr<ApiExperiment::Stub> stub_;
};


uint64_t elapsed_nanoseconds(
    const high_resolution_clock::time_point& ref_time_point) {
  return duration_cast<nanoseconds>(
        high_resolution_clock::now() - ref_time_point).count();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);
  printf("query_id,success,query_time,backend_ts\n");
  ApiClient client(grpc::CreateChannel(
      FLAGS_backend, grpc::InsecureCredentials(), ChannelArguments()));
  auto begin_tp = high_resolution_clock::now();
  auto steady_tp = steady_clock::now();
  const unsigned num_requests = 5;
  unsigned errors = 0;
  for (unsigned i=0; i < FLAGS_num_queries; ++i) {
    unsigned req_id = i + FLAGS_base_id;
    uint64_t ns_before = elapsed_nanoseconds(begin_tp);
    uint64_t backend_ts = client.EchoTimestamp(req_id);
    uint64_t ns_after = elapsed_nanoseconds(begin_tp);
    printf("%u,%d,%llu,", req_id, (backend_ts == 0 ? 0 : 1),
           ns_after - ns_before);
    if (backend_ts == 0) {
      errors++;
      printf("-1\n");
    } else {
      printf("%llu.%06llu\n", backend_ts / 1000000, backend_ts % 1000000);
    }
  }
  double total_time = duration_cast<duration<double>>(
      steady_clock::now() - steady_tp).count();
  LOG(INFO) << "Finished " << FLAGS_num_queries << " requests with " << errors
      << " errors, took " << total_time << " seconds";

  return 0;
}
