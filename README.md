# CSCE 438 - Homework 3
###Vincent Velarde, Kyle Rowland
March 2017

###Notes:
  - This project involves stress testing the server from [HW2](https://github.com/vinvelarde/438A2)
  
###Compiling and Running the Program:
- Use "make all" to compile and "make clean" to clear everything but source files
- Run server with "./server /<port/>"
- Run client with "./client <hostname> <port> <username>"

##Functionality Overview:
###Client Code - [fbc.cpp](https://github.com/vinvelarde/CSCE438-HW2/blob/master/fbc.cpp): 
Connects to server and waits for user to issue commands, then takes some action based on the command:
- If user issues LIST command, the "List (User) returns (stream user)" RPC is called.
- If the user issues JOIN or LEAVE command, the "Following (Action) returns (SendMsg)" RPC is called.
- If the user issues CHAT command, the client leaves command mode and enters chat mode. In chat mode the "Msg (SendMSG) returns (SendMSG)"    RPC is called for any input.
- IMPORTANT: When running in CHAT mode on the client, type ":r" to refresh. This will reload the 20 most current posts.

###Server Code - [fbsd.cpp](https://github.com/vinvelarde/CSCE438-HW2/blob/master/fbsd.cpp):
Starts server and waits for client input.
- When new user joins the server, their username will be checked against a list of all users. If the name doesn't already exist, it will be added.
- When a client switches to chat mode they will be sent the past 20 messages from all users they are following.
