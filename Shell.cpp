// Implements a basic shell (command line interface) for the file system

// You should implement your client-side code here. !!! //

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <climits>

using namespace std;

#include "Shell.h"

static const string PROMPT_STRING = "NFS> ";	// shell prompt

bool is_mounted = false;

// Mount the network file system with server name and port number in the format of server:port
void Shell::mountNFS(string fs_loc) {
  if (is_mounted) {
    cout << "mount failed: NFS is already mounted" << endl;
    return;
  }

	// create the socket cs_sock and connect it to the server and port specified in fs_loc

  // parse string to obtain the server address and port number
  size_t pos = fs_loc.find(':');
  if (pos == string::npos) {
    cout << "mount failed: invalid file system location" << endl;
    return;
  }
  string address = fs_loc.substr(0, pos);
  string fs_port = fs_loc.substr(pos+1);
  int port = stoi(fs_port);

  // create a TCP socket
  cs_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (cs_sock < 0) {
    perror("socket creation failed");
    return;
  }

  // Convert the server name to an IP address
  struct sockaddr_in server;
  memset(&server, 0, sizeof (server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (connect(cs_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    // mount fail
    perror("connection failed");
    close(cs_sock);
    exit(0);
  }

  //if all the above operations are completed successfully, set is_mounted to true
  is_mounted = true;
}

// Unmount the network file system if it was mounted
void Shell::unmountNFS() {
	// close the socket if it was mounted
  if (is_mounted) {
    close(cs_sock);
    is_mounted = false;
  } else {
    cout << "unmount failed: NFS is not mounted" << endl;
  }
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname) {
  string command_line = "mkdir " + dname;
  execute("mkdir", command_line);
}

// Remote procedure call on cd
void Shell::cd_rpc(string dname) {
  string command_line = "cd " + dname;
  execute("cd", command_line);
}

// Remote procedure call on home
void Shell::home_rpc() {
  string command_line = "home\r\n";
  execute("home", command_line);
}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname) {
  string command_line = "rmdir " + dname ;
  execute("rmdir", command_line);
}

// Remote procedure call on ls
void Shell::ls_rpc() {
  string command_line = "ls";
  execute("ls", command_line);
}

// Remote procedure call on create
void Shell::create_rpc(string fname) {
  string command_line = "create " + fname;
  execute("create", command_line);
}

// Remote procedure call on append
void Shell::append_rpc(string fname, string data) {
  string command_line = "append " + fname + " " + data ;
  execute("append", command_line);  
}

// Remote procesure call on cat
void Shell::cat_rpc(string fname) {
  string command_line = "cat " + fname;
  execute("cat", command_line);
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n) {
  string command_line = "head " + fname + " " + to_string(n);
  execute("head", command_line);
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname) {
  string command_line = "rm " + fname;
  execute("rm", command_line);
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname) {
  string command_line = "stat " + fname;
  execute("stat", command_line);
}

// Helper function to display response
void Shell::execute(string command, string arg) {
  if (!is_mounted) {
    cout << "Error: NFS is not mounted" << endl;
    return;
  }

  char command_line[1024];
  strcpy(command_line, arg.c_str());
  send(cs_sock, command_line, sizeof(command_line), 0);

  // create a buffer to store the received message
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));

  // receive the message from the server
  int bytes_received = recv(cs_sock, buffer, sizeof(buffer), 0);

  if (bytes_received == -1) {
    cout << "error receiving message" << endl;
  } else {
    cout << buffer << endl;
  }

}

// Executes the shell until the user quits.
void Shell::run()
{
  // make sure that the file system is mounted
  if (!is_mounted)
 	return; 
  
  // continue until the user quits
  bool user_quit = false;
  while (!user_quit) {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }

  // unmount the file system
  unmountNFS();
}

// Execute a script.
void Shell::run_script(char *file_name)
{
  // make sure that the file system is mounted
  if (!is_mounted)
  	return;

  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail()) {
    cerr << "Could not open script file" << endl;
    return;
  }

  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit) {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  unmountNFS();
  infile.close();
}


// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "") {
    return false;
  }
  else if (command.name == "mkdir") {
    mkdir_rpc(command.file_name);
  }
  else if (command.name == "cd") {
    cd_rpc(command.file_name);
  }
  else if (command.name == "home") {
    home_rpc();
  }
  else if (command.name == "rmdir") {
    rmdir_rpc(command.file_name);
  }
  else if (command.name == "ls") {
    ls_rpc();
  }
  else if (command.name == "create") {
    create_rpc(command.file_name);
  }
  else if (command.name == "append") {
    append_rpc(command.file_name, command.append_data);
  }
  else if (command.name == "cat") {
    cat_rpc(command.file_name);
  }
  else if (command.name == "head") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      head_rpc(command.file_name, n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "rm") {
    rm_rpc(command.file_name);
  }
  else if (command.name == "stat") {
    stat_rpc(command.file_name);
  }
  else if (command.name == "quit") {
    return true;
  }

  return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str)
{
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  if (ss >> command.name) {
    num_tokens++;
    if (ss >> command.file_name) {
      num_tokens++;
      if (ss >> command.append_data) {
        num_tokens++;
        string junk;
        if (ss >> junk) {
          num_tokens++;
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0) {
    return empty;
  }
    
  // Check for invalid command lines
  if (command.name == "ls" ||
      command.name == "home" ||
      command.name == "quit")
  {
    if (num_tokens != 1) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "mkdir" ||
      command.name == "cd"    ||
      command.name == "rmdir" ||
      command.name == "create"||
      command.name == "cat"   ||
      command.name == "rm"    ||
      command.name == "stat")
  {
    if (num_tokens != 2) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "append" || command.name == "head")
  {
    if (num_tokens != 3) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl; 
    return empty;
  } 

  return command;
}