#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "proto/c2nl.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using ::Code2NL;
using ::Code;
using ::Summary;


class SummaryClient {
public:
	static SummaryClient* Instance() {

		static SummaryClient Inst(
			grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())
		);

		return &Inst;
	}

	// Assembles the client's payload, sends it and presents the response back
	// from the server.
	std::string summarize(const std::string& source) {
    	// Data we are sending to the server.
		Code request;
		request.set_code(source);

    	// Container for the data we expect from the server.
    	Summary reply;

		// Context for the client. It could be used to convey extra information to
		// the server and/or tweak certain RPC behaviors.
    	ClientContext context;

    	// The actual RPC.
    	Status status = stub_->summarize(&context, request, &reply);

    	// Act upon its status.
		if (status.ok()) {
		return reply.summary();
		} else {
		std::cout << status.error_code() << ": " << status.error_message()
					<< std::endl;
		return "RPC failed";
		}
	}

private:
	SummaryClient(std::shared_ptr<Channel> channel): stub_(Code2NL::NewStub(channel)) {}
	std::unique_ptr<Code2NL::Stub> stub_;
};