# About

This repository contains an implementation of a client-server network file system designed on top of a virtual disk that uses an indexed block allocation approach. The virtual disk architecture consists of a Superblock, Free block bitmap, Inodes (for files and directories), and Datablocks. The communication between the client and server is persistent and relies on a reliable TCP connection.

## Features
The file system utilizes an indexed block allocation methodology.
The virtual disk architecture incorporates a Superblock, Free block bitmap, Inodes for files and directories, and Datablocks.
Communication between the client and server is persistent and uses a reliable TCP connection.
The system supports file and directory creation, deletion, renaming, and modification.
Multiple clients can access the same file system concurrently, and changes are synchronized across all connected clients.


Team member #1: *Robert Widjaja*
<br />
Contributions:
<br />
Worked on mainly the FileSys , Debugged and tested all files

Team member #2: *Carmel Laurentia*
<br />
Contributions:
<br />
Worked on mainly the Shell, Debugged and Tested all files

Team member #3: *Alexander Chen*
<br />
Contributions:
<br />
Worked on mainly the Server, Debugged and tested all files


## Commands Available
- `ls`: List the contents of the current directory
- `cd <directory>`: Change to a specified directory
- `home`: Switch to the home (root) directory (similar to `cd /` in Unix)
- `rmdir <directory>`: Remove a directory. The directory must be empty
- `create <filename>`: Create an empty file
- `append <filename> <data>`: Append data to an existing file
- `stat <name>`: Display information for a given file or directory
- `cat <filename>`: Display the contents of a file
- `head <filename> <n>`: Display the first `n` bytes of the file
- `rm <filename>`: Remove a file


## Some Test Cases

NFS> ls
  empty folder
NFS> mkdir dir1
  success
NFS> mkdir dir2
  success
NFS> ls
  dir1 dir2 
NFS> cd dir1
  success
NFS> create file1
  success
NFS> append file1 helloworld!
  success
NFS> stat file1
  Inode block:     4
Bytes in file:     11
Number of blocks:     2
First block:     5
NFS> ls
  file1 
NFS> cat file1
  helloworld!
NFS> head file1 5
  hello
NFS> rm file2
  503 File does not exist
NFS> cat file2
  503 File does not exist
NFS> create file1
  502 File exists
NFS> create file2
  success
NFS> rm file1
  success
NFS> ls
  file2 
NFS> home
  success
NFS> ls
  dir1 dir2 
NFS> stat dir1
  Directory name:     dir1
Directory block:     2
NFS> rmdir dir3
  503 File does not exist
NFS> rmdir dir1
  507 Directory is not empty
NFS> rmdir dir2
  success
NFS> ls
  dir1 
NFS> quit
