//// Each team member's name and contributions ////////////
Team member #1: Robert Widjaja
Contributions:
Worked on mainly the FileSys , Debugged and tested all files

Team member #2: Carmel Laurentia
Contributions:
Worked on mainly the Shell, Debugged and Tested all files

Team member #3: Alexander Chen
Contributions:
Worked on mainly the Server, Debugged and tested all files



//// My own rating on the functionality ///////////////////
Rating: A, B, C, D, or F?
    A-
Explanation: 

Test cases provided in the pdf worked successfully. Messages that are meant to appear in the client side aren't showed in the server.
cd do not work on some ports but were able to successfully work on most ports. Functionality of each function works as intended
and displayed nicely to the client.



//// Test cases ///////////////////////////////////////////
[claurentia@cs1 hw]$ ./nfsclient localhost:10480
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