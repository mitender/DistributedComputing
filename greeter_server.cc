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
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::FileName;
using helloworld::FileContent;
using helloworld::AddFileRequest;
using helloworld::ServerReply;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {

   Status GetFileContent(ServerContext* context, const FileName* request, FileContent* response) override {
	   
        std::string filename = request->filename();
		std::cout << "File Request received by the server for the file name: " << filename << std::endl; 
        
        std::ifstream file(filename);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            response->set_content(buffer.str());
			std::cout << "Server 1 FileContent: " << buffer.str() << std::endl;
        } else {
            std::cout << "Server FileNotFoundError: " << filename << std::endl;
            // File not found locally, return empty content
                return forwardRequestToNextServer(filename, response);
        }
        return Status::OK;
    }
	
	Status AddFileContent(ServerContext* context, const AddFileRequest* request, ServerReply* response) override {
	   
        std::string filename = request->filename();
		std::string content= request->content();
		
		std::cout << "Adding file: " << filename << std::endl;
		std::cout << "Content: " << content << std::endl;
		

		// Combine the upload directory with the filename
		std::string file_path = filename;

		// Open the file in binary mode and write the content
		std::ofstream file(file_path, std::ios::out | std::ios::binary);
		if (file.is_open()) {
			file.write(content.c_str(), content.size());
			file.close();
			
			// Respond to the client
			response->set_message("File added successfully");
		} else {
			// Failed to open the file
			std::cerr << "Error: Unable to open file for writing." << std::endl;
			response->set_message("Failed to add file");
		}


        return Status::OK;
    }
	
	private:
	 Status forwardRequestToNextServer(const std::string& filename, FileContent* response) {
        std::cout << "Forwarding request to next server: " << filename << std::endl;
        std::string next_server_address = next_server_address_ + ":" + std::to_string(next_server_port_);
		
		if (visited_servers_.count(next_server_address) > 0) {
			visited_servers_.erase(next_server_address);
            std::cerr << "Error: Cyclic condition detected. Cannot forward request." << std::endl;
            return Status(grpc::StatusCode::CANCELLED, "Cyclic condition detected");
        }
		
		std::cerr<<"no of visited servers:"<<visited_servers_.count(next_server_address)<<std::endl;
        
        // Mark the next server as visited
        visited_servers_.insert(next_server_address);
		
        std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(grpc::CreateChannel(next_server_address, grpc::InsecureChannelCredentials()));
        FileName request;
        request.set_filename(filename);
        FileContent next_response;
        grpc::ClientContext context;
        Status status = stub->GetFileContent(&context, request, &next_response);
		visited_servers_.erase(next_server_address);
        if (status.ok()) {
			
			 response->set_content(next_response.content());
			std::ofstream copy("copy_" + filename);
			
			std::string file_path =filename;

			// Open the file in binary mode and write the content
			std::ofstream file(file_path, std::ios::out | std::ios::binary);	
			if (file.is_open()) {
				file.write(next_response.content().c_str(), next_response.content().size());
				file.close();
				
				// Respond to the client
				//response->set_message("File added successfully");
				std::cout<<"file added successfully"<<std::endl;
			} else {
				// Failed to open the file
				std::cerr << "Error: Unable to open file for writing." << std::endl;
			}
        } else {
            std::cerr << "Error forwarding request: " << status.error_code() << ": " << status.error_message() << std::endl;
        }
        return status;
    }

    std::string next_server_address_;
    int next_server_port_;
	std::unordered_set<std::string> visited_servers_;
	
	public:
    explicit GreeterServiceImpl(const std::string& next_server_address, int next_server_port)
        : next_server_address_(next_server_address), next_server_port_(next_server_port) {}
  
  
};

void RunServer(std::string server_address) {
	
  //std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
  GreeterServiceImpl service("127.0.0.1",50052);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  //absl::ParseCommandLine(argc, argv);
  //RunServer(absl::GetFlag(FLAGS_port));
  std::string server1_ip = "127.0.0.1"; // Replace with actual server IP address
    int server1_port = 50051; // Replace with actual server port

    // Create a channel to connect to the server
    std::string server1_address = server1_ip + ":" + std::to_string(server1_port);
	RunServer(server1_address);

	
  return 0;
}
