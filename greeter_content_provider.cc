/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");



#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "helloworld.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::ServerReply;
using helloworld::HelloRequest;
using helloworld::FileName;
using helloworld::AddFileRequest;

class GreeterContentProvider {
public:
    GreeterContentProvider(std::shared_ptr<Channel> channel)
            : stub_(Greeter::NewStub(channel)) {}

    std::string AddFileContent(const std::string& filename, const std::string& content) {
        AddFileRequest request;
        request.set_filename(filename);
        request.set_content(content);
        ServerReply reply;
        ClientContext context;
        Status status = stub_->AddFileContent(&context, request, &reply);
        if (status.ok()) {
            return "File added successfully";
        } else {
            return "Failed to add file";
        }
    }

private:
    std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
    // Create a channel to connect to the server
	
	absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  
  std::string server_ip = "127.0.0.1"; // Replace with actual server IP address
	int server_port = 50051; // Replace with actual server port

	// Create a channel to connect to the server
	std::string server_address = server_ip + ":" + std::to_string(server_port);
	// We indicate that the channel isn't authenticated (use of
	// InsecureChannelCredentials()).

    GreeterContentProvider greeter(
            grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    // Your client code goes here...
	
	std::string filename = "example1.txt";
	std::string reply = greeter.AddFileContent(filename,"I am server 1");
	std::cout << "Reply from server" << reply << std::endl;
    
    return 0;
}

