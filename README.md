# CSCE 438 - Homework 3
###Vincent Velarde, Kyle Rowland
March 2017

###Notes:
  - This project involves stress testing the server from [HW2](https://github.com/vinvelarde/438A2)
  
###Compiling and Running the Program:
- Use "make all" to compile and "make clean" to clear everything but source files
- Run server with "./server \<port>"
- Run client with "./client \<hostname> \<port> \<username> \<# chats> \<frequency>"
- Run follower with "./follower \<hostname> \<port> \<username>

##Functionality Overview:
###Client Code - fbc.cpp: 
Connects to server and sends a set number of messages at a set frequency:

###Follower Code - fbc2.cpp
Connects to server and recieves incoming messages
- Connect to the server, JOIN one or more other users, and issue CHAT command
- Refreshes once every second

###Server Code - fbsd.cpp:
Starts server and waits for client input. (Server from HW2)
- When new user joins the server, their username will be checked against a list of all users. If the name doesn't already exist, it will be added.
- When a client switches to chat mode they will be sent the past 20 messages from all users they are following.
