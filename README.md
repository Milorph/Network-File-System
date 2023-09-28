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
<br />
  empty folder
  <br />
NFS> mkdir dir1
<br />
  success
  <br />
NFS> mkdir dir2
<br />
  success
  <br />
NFS> ls
<br />
  dir1 dir2 
  <br />
NFS> cd dir1
<br />
  success
  <br />
NFS> create file1
<br />
  success
  <br />
NFS> append file1 helloworld!
<br />
  success
  <br />
NFS> stat file1
<br />
  Inode block:     4
  <br />
Bytes in file:     11
<br />
Number of blocks:     2
<br />
First block:     5
<br />
NFS> ls
<br />
  file1 
  <br />
NFS> cat file1
<br />
  helloworld!
  <br />
NFS> head file1 5
<br />
  hello
  <br />
NFS> rm file2
<br />
  503 File does not exist
  <br />
NFS> cat file2
<br />
  503 File does not exist
  <br />
NFS> create file1
<br />
  502 File exists
  <br />
NFS> create file2
<br />
  success
  <br />
NFS> rm file1
<br />
  success
  <br />
NFS> ls
<br />
  file2 
  <br />
NFS> home
<br />
  success
  <br />
NFS> ls
<br />
  dir1 dir2 
  <br />
NFS> stat dir1
<br />
  Directory name:     dir1
  <br />
Directory block:     2
<br />
NFS> rmdir dir3
<br />
  503 File does not exist
  <br />
NFS> rmdir dir1
<br />
  507 Directory is not empty
  <br />
NFS> rmdir dir2
<br />
  success
  <br />
NFS> ls
<br />
  dir1 
  <br />
NFS> quit
