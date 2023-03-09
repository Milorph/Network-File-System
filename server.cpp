#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "FileSys.h"
#include <unistd.h>
using namespace std;


int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    //networking part: create the socket and accept the client connection
    //change this line when necessary!
    int sock = socket(AF_INET, SOCK_STREAM, 0);  // IPV4
    if (sock < 0) {
        cerr << "Error: Failed to create socket." << endl;
        exit(1);
    }

    
    
    struct sockaddr_in server_addr, client_addr;
    
    // Initialize the serv_addr variable by setting all bytes to 0
    // This ensures that any unused bytes in the variable are set to a known value
    std::memset(&server_addr, 0, sizeof(server_addr)); // 
    server_addr.sin_family = AF_INET; // sets the address family of serv_addr to IPV4.
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind a socket to all available network interfaces on a system.
    server_addr.sin_port = htons(port); // Set the port number for the socket in network byte order
    
    
    // checks if bind successful
     if (::bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error: Failed to bind socket to port." << endl;
        exit(1);
    }

    cout << "Bind complete" << endl;
    cout << "Awaiting for connection..." << endl;

    // listen for incoming connections
    if(listen(sock, 5) != 0) {
        cerr << "Error: listening on socket failed" << endl;
        exit(1);
    }

    // accept client connections
    socklen_t clientsize = sizeof(client_addr);
    int newsock = accept(sock, (struct sockaddr*) &client_addr, &clientsize);
    if (newsock < 0) {
        cerr << "Error: Failed to accept client connection." << endl;
        exit(1);
    }

    cout << "Connection success" << endl;

    //mount the file system
    FileSys fs;
    fs.mount(newsock);  //assume that sock is the new socket created 
                        //for a TCP connection between the client and the server.   
 
    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the client
    //until the client closes the TCP connection.

    string input_command = "";

    while(true) {
        
        
        char buffer[1024]; // create a global buffer

        // receives from client
        string message = "";
        
        // variables for the received strings 
        string recieved_string = "";
        int actual_recv = 0;
       
        // receiving 
        while(true) {
            actual_recv = read(newsock, (void*) buffer, 1024);
            if (actual_recv == -1) {
                cerr << "error recieving data" << endl;
                break;
            }
            buffer[actual_recv] = '\0';
            recieved_string = string(buffer);
            if(message.find("\r\n") >= 0){
                message += recieved_string;
                break;
            }
            message += recieved_string;
        }

        // takes the received into input_command
        istringstream input_command(message);

        // load command args into strings
        string desired_command, filename, num_str;
        std::getline(input_command, desired_command, ' ');
        std::getline(input_command, filename, ' ');
        std::getline(input_command, num_str);

        // trigger commands based by the request
        if(desired_command == "mkdir") {
			fs.mkdir(filename.data());
		} else if (desired_command == "cd") {
			fs.cd(filename.data());
		} else if (desired_command.substr(0,4) == "home") {
			fs.home();
		} else if (desired_command == "rmdir") {
			fs.rmdir(filename.data());
		} else if (desired_command == "ls") {
			fs.ls();
		} else if (desired_command == "create") {
			fs.create(filename.data());
		} else if (desired_command == "append") {
			fs.append(filename.data(), num_str.data());
		} else if (desired_command == "cat") {
			fs.cat(filename.data());
		} else if(desired_command == "head") {
            int num = stoi(num_str);
			fs.head(filename.data(), num);
		} else if(desired_command == "rm") {
			fs.rm(filename.data());
		} else if(desired_command == "stat") {
			fs.stat(filename.data());
		} else {
			// sends error msg
			string error = "Invalid Command";
            const char* buf = (error + "\r\n").c_str();
            int error_msg_size = error.length() + 2;
            int data_released = 0; // total amt data sent
            int data_sent = 0;     // data that is currently being sent
            while(data_released < error_msg_size){
                data_sent = send(newsock, (void*) (buf + data_released), error_msg_size - data_released, 0);
                if(data_sent == -1) {
                    cerr << "error sending data" << endl;
                    break;
                }
            data_released += data_sent;
            }
		}

    };
    
    
    //close the listening socket
    close(sock);

    //unmout the file system
    fs.unmount();

    return 0;
}