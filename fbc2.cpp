//-------------------------------------------------------------
// CSCE 438: Distributed objects programming
// HW3
// 
// Chat Follower
// Vincent Velarde, Kyle Rowland
// March 2017
//
// NOTES:
// 	-Modified version of client program from HW2
//
//	-Refreshes chats once per second
//
//
//	USAGE INSTRUCTIONS:
//		1. Connect to an active server
//		2. Follow 1 or more clients
//		3. issue CHAT command
//
//-------------------------------------------------------------

#include <iostream>
#include <unistd.h>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>

#include "fb.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using HW2::User;
using HW2::Action;
using HW2::SendMsg;
using HW2::Tweeter;

//this class handles RPCs
class TweeterClient {
	public:
		TweeterClient(std::shared_ptr<Channel> channel)
			: stub_(Tweeter::NewStub(channel)) {}
	
	//called when joining server
	//sends username of client to server and recieves welcome message
	std::string Welcome(const std::string& user) {
		//set up RPC inputs
		User u;
		SendMsg msg;
		u.set_username(user);
		ClientContext context;
		
		//Welcome RPC
		Status status = stub_->Welcome(&context, u, &msg);
		
		//do something with the return value
		if(status.ok()) {
			return msg.message();
		}
		else {
			std::cout << status.error_code() << ": " << status.error_message()
					<< std::endl;
			return "RPC failed";
		}
	}
	
	//called when switching into chat mode
	//	prints the past 20 messages for the clients log
	void Chat(const std::string& u) {
		User user;
		user.set_username(u);
		SendMsg msg;
		ClientContext context;
		
		//loop through returning stream
		std::unique_ptr<ClientReader<SendMsg> > reader(stub_->Chat(&context, user));
		while(reader->Read(&msg)) {
			//print messages
			std::cout << msg.sender() << " : " << msg.timestamp() << msg.message() << std::endl;
		}
		Status status = reader->Finish();
	}
	
	//used for LIST command
	//Sends the client a list of all users indicating which ones
	//	the client is currently following
	void List(const std::string& u) {
		//set up RPC inputs
		User user, reply;
		user.set_username(u);
		ClientContext context;
		
		//loop through returning stream
		std::unique_ptr<ClientReader<User> > reader(stub_->List(&context, user));
		while(reader->Read(&reply)) {
			//print user list and indicate which users have been followed by client
			if(reply.username() == u)
				std::cout << reply.username() << " - You" << std::endl;
			else if(reply.following())
				std::cout << reply.username() << " - Followed" << std::endl;
			else
				std::cout << reply.username() << std::endl;
		}
		Status status = reader->Finish();
	}
	
	//used for JOIN and LEAVE commands
	//bool act represents whether to follow or unfollow
	//	true = follow, false = unfollow
	std::string Following(const std::string& currentUser, std::string& user, bool act) {
		//set up RPC inputs
		SendMsg msg;
		Action action;
		action.set_currentuser(currentUser);
		action.set_username(user);
		action.set_act(act);
		ClientContext context;
		
		//Following RPC
		Status status = stub_->Following(&context, action, &msg);
		
		//do something with the return value
		if(status.ok()) {
			return msg.message();
		}
		else {
			std::cout << status.error_code() << ": " << status.error_message()
					<< std::endl;
			return "RPC failed";
		}
	}
	
	//used for sending client or server messages
	void Msg(const std::string& user, std::string& m) {
		//set up RPC inputs
		SendMsg msg, reply;
		msg.set_sender(user);
		msg.set_message(m);
		ClientContext context;
		
		//Msg RPC
		Status status = stub_->Msg(&context, msg, &reply);
	}
		
	private:
		std::unique_ptr<Tweeter::Stub> stub_;
};

//the client runs this loop while it's in command mode
void cmdLoop(std::string user, TweeterClient* tweeter) {
	std::string input, reply;
	
	//loop infinitely (unless break command is executed)
	while(true) {
		//get user input
		std::cout << "cmd> ";
		getline(std::cin, input);
		
		//handle commands from user input
		if(input == "LIST") {
			tweeter->List(user);
		}
		else if(input.substr(0,4) == "JOIN") {
			std::string follow = input.substr(5, input.length()-5);
			reply = tweeter->Following(user, follow, true);
			std::cout << "Now Following: " << reply << std::endl;
		}
		else if(input.substr(0,5) == "LEAVE") {
			std::string unfollow = input.substr(6, input.length()-6);
			reply = tweeter->Following(user, unfollow, false);
			std::cout << "Unfollowed: " << reply << std::endl;
		}
		else if(input == "CHAT") break;	//break cmdLoop and move to chatLoop
		else std::cout << "Invalid Input" << std::endl;
	}
}

//refresh messages once every second
void chatLoop(std::string user, TweeterClient* tweeter) {
	tweeter->Chat(user);
	
	std::string input, reply;
	//loop infinitely
	while(true) {
		sleep(1);	//we want to refresh the messages once every second.
		system("clear");
		tweeter->Chat(user);
	}
}

int main(int argc, char** argv) {
	//user must provide host name and port number as command line arguments
	if(argc < 4) {
		fprintf(stderr, "USAGE: %s <hostname> <port> <username>\n", argv[0]);
		exit(1);
	}
	
	//server address + port number from command line arguments
	std::string server_address(argv[1]);
	server_address += ":";
	server_address += argv[2];
	
	//create client channel
	TweeterClient tweeter(grpc::CreateChannel(
      server_address, grpc::InsecureChannelCredentials()));
	std::string user(argv[3]);
	
	std::string serverMsg = tweeter.Welcome(user);
	std::cout << serverMsg << std::endl;
	
	cmdLoop(user, &tweeter);
	chatLoop(user, &tweeter);
	
	return 0;
}










