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

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::FileName;
using helloworld::FileContent;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  
  std::string GetFileContent(const std::string& filename) {
	  
	std::cout << "File Request sent for the file name: " << filename << std::endl;  
	FileName request;
	request.set_filename(filename);

	FileContent fileContent;
	ClientContext context;

	Status status = stub_->GetFileContent(&context, request, &fileContent);

	if (status.ok()) {
		return fileContent.content();
	} else {
		std::cout << "Error:  " << status.error_code() << ": " << status.error_message() << std::endl;
		return "File not found";
	}
    }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
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
	GreeterClient greeter(
	  grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

	std::string filename = "example3.txt";
	std::string file_content = greeter.GetFileContent(filename);
	std::cout << "File content: " << file_content << std::endl;
	// Combine the upload directory with the filename
	std::string file_path =filename;

	// Open the file in binary mode and write the content
	std::ofstream file(file_path, std::ios::out | std::ios::binary);	
	if (file.is_open()) {
		file.write(file_content.c_str(), file_content.size());
		file.close();
		
		// Respond to the client
		//response->set_message("File added successfully");
		std::cout<<"file added successfully"<<std::endl;
	} else {
		// Failed to open the file
		std::cerr << "Error: Unable to open file for writing." << std::endl;
	}


  return 0;
}
