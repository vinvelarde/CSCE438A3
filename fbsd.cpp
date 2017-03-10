//-------------------------------------------------------------
// CSCE 438: Distributed objects programming
// HW3
// 
// Chat Server
// Vincent Velarde, Kyle Rowland
// March 2017
//
// NOTES:
// 	-Server program from HW2
//
//-------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <grpc++/grpc++.h>

#include "fb.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using HW2::UserBase;
using HW2::User;
using HW2::Action;
using HW2::SendMsg;
using HW2::Tweeter;
using std::chrono::system_clock;

//this class handles the RPCs
class masterServer final : public Tweeter::Service {
	UserBase userList;
	
	//Welcome RPC
	//checks client username
	//	adds client to user list if they aren't already there
	//	creates client file if one doesn't already exist
	Status Welcome(ServerContext* context, const HW2::User* user,
					SendMsg* welcome) override {
		//first, check if user already exists
		User u;
		welcome->set_sender("SERVER: ");
		for(int i=0; i<userList.user_size(); i++) {
			u = userList.user(i);
			if(u.username() == user->username()) {	//if user exists, welcome back
				welcome->set_message("Welcome Back!");
				return Status::OK;
			}
		}
		
		//if user doesn't exist add them to userList
		User* newUser = userList.add_user();
		newUser->set_username(user->username());
		newUser->add_followers(user->username());
		std::filebuf fb;
		fb.open("UserList.txt", std::ios::out);
		std::ostream os(&fb);
		userList.SerializeToOstream(&os);
		fb.close();
		
		welcome->set_message("Welcome to the Server!");
		
		return Status::OK;
	}
	
	//Chat RPC
	//gets past 20 messages from users log
	//	called when a client switches to chat mode
	Status Chat(ServerContext* context, const HW2::User* user,
				ServerWriter<SendMsg>* writer) override {
		User u;
		SendMsg msg;
		//find current user in userList
		for(int i=0; i<userList.user_size(); i++) {
			u = userList.user(i);
			if(u.username() == user->username()) {
				//send the latest 20 message
				int start = 0;
				if(u.userlog_size() > 20) start = u.userlog_size() - 20;
				for(int j=start; j<u.userlog_size(); j++) {
					msg = u.userlog(j);
					writer->Write(msg);
				}
				break;
			}
		}
		return Status::OK;
	}
	
	//LIST command
	//Sends the client a list of all users indicating which ones
	//	the client is currently following
	Status List(ServerContext* context, const HW2::User* user,
				ServerWriter<User>* writer) override {
		User u;
		//iterate through userList and send each user
		for(int i=0; i<userList.user_size(); i++) {
			u = userList.user(i);
			//iterate through followers and check if the current user is following User u
			for(int j=0; j<u.followers_size(); j++) {
				if(u.followers(j) == user->username()) {
					u.set_following(true);
				}
			}
			writer->Write(u);
		}
		
		return Status::OK;
	}
	
	//JOINT and LEAVE commands
	//Follows or unfollows a user for the client
	Status Following(ServerContext* context, const Action* action,
							SendMsg* msg) override {
		User* u;
		//iterate through userList and find the username to follow/unfollow
		for(int i=0; i<userList.user_size(); i++) {
			u = userList.mutable_user(i);
			if(u->username() == action->username()) {
				if(action->act()) {	//if this is a JOIN command
					u->add_followers(action->currentuser());
					break;
				}
				else {	//if this is a LEAVE command
					User temp;
					temp.set_username(u->username());
					
					//copy over all the followers other than you to temp
					for(int j=0; j<u->followers_size(); j++) {
						if(u->followers(j) != action->currentuser())
							temp.add_followers(u->followers(j));
					}
					
					//clear the followers list and copy back the new list from temp
					u->clear_followers();
					for(int j=0; j<temp.followers_size(); j++) {
						u->add_followers(temp.followers(j));
					}
					break;
				}
			}
		}
		
		std::filebuf fb;
		fb.open("UserList.txt", std::ios::out);
		std::ostream os(&fb);
		userList.SerializeToOstream(&os);
		fb.close();
		
		msg->set_sender("SERVER: ");
		msg->set_message(action->username());
		
		return Status::OK;
	}
	
	int count = 0;
	
	//Used for sending client or server messages
	Status Msg(ServerContext* context, const SendMsg* msg,
				SendMsg* confirm) override {
		system_clock::time_point today = system_clock::now();
		time_t tt;
		tt = system_clock::to_time_t(today);
		
		User* u;
		std::vector<std::string> followers;
		//iterate through userList and find sender
		for(int i=0; i<userList.user_size(); i++) {
			u = userList.mutable_user(i);
			if(u->username() == msg->sender()) {	//find the current user
				for(int j=0; j<u->followers_size(); j++) {	//add current user's followers to a vector
					followers.push_back(u->followers(j));
				}
				break;
			}
		}
		count++;
		//now that we have the names of all the followers we need to update their logs
		SendMsg* m;
		for(int i=0; i<userList.user_size(); i++) {	//iterate through userList
			u = userList.mutable_user(i);
			for(int j=0; j<followers.size(); j++) {	//iterate through followers vector
				if(u->username() == followers[j]) {	//find matching entries
					m = u->add_userlog();	//update userlog with the message
					m->set_sender(msg->sender());
					std::string stamp = ctime(&tt);
					std::cout << count << ": " << stamp << std::endl;
					m->set_timestamp(/*ctime(&tt)*/stamp);
					m->set_message(msg->message());
				}
			}
		}
		
		std::filebuf fb;
		fb.open("UserList.txt", std::ios::out);
		std::ostream os(&fb);
		userList.SerializeToOstream(&os);
		fb.close();
		
		return Status::OK;
	}
	
	public:
		//read current user list from file at server start
		void getUserList() {
			std::filebuf fb;
			fb.open("UserList.txt", std::ios::in);
			std::istream os(&fb);
			userList.ParseFromIstream(&os);
			fb.close();
		}
};

//setup and run the server
void runServer(char* p) {
	//server address and port number from command line argument
	std::string server_address("0.0.0.0:");
	server_address += p;
	
	masterServer service;
	
	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	service.getUserList();
	
	server->Wait();
}

int main(int argc, char** argv) {
	//user must provide port number as command line argument
	if(argc < 2) {
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		exit(1);
	}
	
	runServer(argv[1]);
	
	return 0;
}
