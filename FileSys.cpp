// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sstream> 

using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// You should implement your server-side FS code here.

// mounts the file system
void FileSys::mount(int sock) {
  bfs.mount();
  curr_dir = 1; //by default current directory is home directory, in disk block #1
  fs_sock = sock; //use this socket to receive file system operations from the client and send back response messages
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
  close(fs_sock);
}

void FileSys::mkdir(const char *name) {  
  cout << "inside mkdir" << endl;
    dirblock_t curr_dir_block;
    bfs.read_block(curr_dir, (void *) &curr_dir_block);

    dirblock_t new_dir;
    short new_block;
    
    // check if the directory already exists
    for (int i = 0; i < curr_dir_block.num_entries; i++) {
        if (strcmp(name, curr_dir_block.dir_entries[i].name) == 0) {
            error_message(ERR_502);
            return;
        }
    }
    if(strlen(name) > MAX_FNAME_SIZE){
        error_message(ERR_504);
        return;
    } else if(curr_dir_block.num_entries == MAX_DIR_ENTRIES){
        error_message(ERR_506);
        return;
    } else if(!(new_block = bfs.get_free_block())){
        error_message(ERR_505);
        return;
    }
    
    new_dir.num_entries = 0;
    new_dir.magic = DIR_MAGIC_NUM;
    bfs.write_block(new_block, &new_dir);
    
    // Add the new directory to the current directory
    int new_entry_index = curr_dir_block.num_entries;
    strcpy(curr_dir_block.dir_entries[new_entry_index].name, name);
    curr_dir_block.dir_entries[new_entry_index].block_num = new_block;
    curr_dir_block.num_entries++;

    bfs.write_block(curr_dir, &curr_dir_block);
    
    success_message("success");
}

// switch to a directory
void FileSys::cd(const char *name)
{
	dirblock_t new_dir_block;
	short i;
	
	if((i = block_index_retrive(name)) == -1){
		error_message(ERR_503);
		return;
	}
	
	bfs.read_block(FileSys::curr_dir_block.dir_entries[i].block_num, (void*) &new_dir_block);
	if(new_dir_block.magic != DIR_MAGIC_NUM){
		error_message(ERR_500);
		return;
	}
	
	curr_dir = curr_dir_block.dir_entries[i].block_num;
	curr_dir_block = new_dir_block;
	success_message( "success");
}


// switch to home directory
void FileSys::home() {
  cout << "inside home" << endl;
  curr_dir = 1;
	bfs.read_block(curr_dir, &curr_dir_block);
	success_message("success");
}

// remove a directory
void FileSys::rmdir(const char *name) {
  cout << "inside rmdir" << endl;
  // read the current directory block
  bfs.read_block(curr_dir, &curr_dir_block);

  // search for the directory entry
  int index;
  for (index = 0; index < curr_dir_block.num_entries; index++) {
    if (strcmp(name, curr_dir_block.dir_entries[index].name) == 0) {
      break;
    }
  }

  if (index == curr_dir_block.num_entries) {
    error_message(ERR_503);
    return;
  } else if (curr_dir_block.magic != DIR_MAGIC_NUM) {
    error_message(ERR_500);
    return;
  }

  // read the directory block to be deleted
  dirblock_t del_curr_dir_block;
  bfs.read_block(curr_dir_block.dir_entries[index].block_num, &del_curr_dir_block);

  // check if the directory is empty
  if (del_curr_dir_block.num_entries > 0) {
    error_message(ERR_507);
    return;
  }

  // free the directory block
  bfs.reclaim_block(curr_dir_block.dir_entries[index].block_num);

  // remove the directory entry from the parent directory
  for (int j = index; j < curr_dir_block.num_entries - 1; j++) {
    curr_dir_block.dir_entries[j] = curr_dir_block.dir_entries[j + 1];
  }
  curr_dir_block.num_entries--;
  bfs.write_block(curr_dir, &curr_dir_block);

  success_message("success");
}

// list the contents of current directory
void FileSys::ls() {
  cout << "This is whats inside: " << endl << endl;

  // Read the current directory block into memory
  bfs.read_block(curr_dir, (void*)&curr_dir_block);
  
  //If there are zero entries, we display the message empty folder
  if(curr_dir_block.num_entries == 0){
    success_message("empty folder");
    return;
  }

  char buffer[1024] = "";

  for (int i = 0; i < curr_dir_block.num_entries; i++) {
    strncat(buffer, curr_dir_block.dir_entries[i].name, sizeof(buffer)-1);
    strncat(buffer, " ", sizeof(buffer)-1);
  }

  // Send a success response to the client
  success_message(buffer);
}

// create an empty data file
void FileSys::create(const char *name)
{
	inode_t new_file;
	short new_block;
	
	if(block_index_retrive(name) >= 0){
		error_message(ERR_502);
		return;
	}
	if(strlen(name) > MAX_FNAME_SIZE){
		error_message(ERR_504);
		return;
	}
	if(curr_dir_block.num_entries == MAX_DIR_ENTRIES){
		error_message(ERR_506);
		return;
	}
	if(!(new_block = bfs.get_free_block())) {
		error_message(ERR_505);
		return;
	}
	
	new_file.size = 0;
	new_file.magic = INODE_MAGIC_NUM;
	bfs.write_block(new_block, &new_file);
	
	//copy name of file into directory
	int i = 0;
	for(i = 0; name[i] != '\0'; i++)
		curr_dir_block.dir_entries[curr_dir_block.num_entries].name[i] = name[i];
	curr_dir_block.dir_entries[curr_dir_block.num_entries].name[i] = name[i];
	
	curr_dir_block.dir_entries[curr_dir_block.num_entries].block_num = new_block;
	curr_dir_block.num_entries++;
	bfs.write_block(curr_dir, (void*) &curr_dir_block);
	
	success_message("success");
}

// append data to a data file
void FileSys::append(const char *name, const char *data) {
	short inode;
	inode_t inode_block;
	int data_index;
	int data_offset;
	datablock_t curr_block;
	
	if ((inode = block_index_retrive(name)) == -1) {
		error_message(ERR_503);
		return;
	}
	
	bfs.read_block(curr_dir_block.dir_entries[inode].block_num, &inode_block);

	if (inode_block.magic != INODE_MAGIC_NUM){
		error_message(ERR_501);
		return;
	} else if (inode_block.size == MAX_FILE_SIZE) {
		error_message(ERR_508);
		return;
	}

	//get the position of the last byte in the file
	data_index = inode_block.size / BLOCK_SIZE;
	data_offset = inode_block.size % BLOCK_SIZE;
	
	//allocate new block if last inode block is full
	if(data_offset == 0) {
		inode_block.blocks[data_index] = bfs.get_free_block();
	}
	
	bfs.read_block(inode_block.blocks[data_index], &curr_block);
	int i = 0;
	while(data[i] != '\0'){
		
		//if at end of current data block check to see if we can get a new one
		if(data_offset == BLOCK_SIZE) {
			data_offset = 0;
			bfs.write_block(inode_block.blocks[data_index++], (void*) &curr_block);
			
			if(data_index == MAX_DATA_BLOCKS) {
				inode_block.size = data_index * BLOCK_SIZE;
				bfs.write_block(curr_dir_block.dir_entries[inode].block_num, (void*) &inode_block);
				error_message(ERR_508);
				return;
			}
			if((inode_block.blocks[data_index] = bfs.get_free_block()) == 0) {
				inode_block.size = data_index * BLOCK_SIZE;
				bfs.write_block(curr_dir_block.dir_entries[inode].block_num, (void*) &inode_block);
				error_message(ERR_505);
				return;
			}
			//create new data block to fill
			curr_block = datablock_t();
		}
		curr_block.data[data_offset++] = data[i++];
	}
	
	bfs.write_block(inode_block.blocks[data_index], (void*) &curr_block);
	inode_block.size = data_index * BLOCK_SIZE + data_offset;
	bfs.write_block(curr_dir_block.dir_entries[inode].block_num, (void*) &inode_block);

	success_message("success");
}



// display the contents of a data file 1
void FileSys::cat(const char *name) {
  head(name, MAX_FILE_SIZE);
}

void FileSys::head(const char *name, unsigned int n)
{
	short inode_index;
	inode_t inode_block;
	datablock_t data_block;
	int index = 0;
	int offset = 0;
	int block_num = 0;
	
	if((inode_index = block_index_retrive(name)) == -1) {
		error_message(ERR_503);
		return;
	}
	
	bfs.read_block(curr_dir_block.dir_entries[inode_index].block_num, (void*) &inode_block);
	if(inode_block.magic != INODE_MAGIC_NUM) {
		error_message(ERR_501);
		return;
	}
	// determine if to print whole file or n bytes
	n = (n > inode_block.size) ? inode_block.size : n;
	char file_data[n + 1];
	
	for(int i = 0; i < n; i++) {
		//if starting a new block, read that block of data
		if(offset == 0) {
			bfs.read_block(inode_block.blocks[block_num++], (void*) &data_block);
		}
		file_data[index++] = data_block.data[offset++];
		offset = offset % BLOCK_SIZE;
	}

	file_data[index] = '\0';
    cout << "file_data" << endl;
	success_message(file_data);
}


// delete a data file
void FileSys::rm(const char *name)
{
	short inode_index;
	short inode_block_num;
	inode_t inode_block;
	int block_num;
	
	if((inode_index = block_index_retrive(name)) == -1) {
		error_message(ERR_503);
		return;
	}
	
	inode_block_num = curr_dir_block.dir_entries[inode_index].block_num;
	bfs.read_block(inode_block_num, &inode_block);
	if(inode_block.magic != INODE_MAGIC_NUM) {
		error_message(ERR_501);
		return;
	}
	
	//move file to remove to the end of the list then decrease size of list
	swap(curr_dir_block.dir_entries[inode_index], curr_dir_block.dir_entries[curr_dir_block.num_entries - 1]);
	curr_dir_block.num_entries--;
	bfs.write_block(curr_dir,  &curr_dir_block);
	block_num = (inode_block.size == 0) ? 0 : ((inode_block.size - 1) / BLOCK_SIZE + 1);
	
	//reclaim all datablocks used by file
	for(int i = 0; i < block_num; i++) {
		bfs.reclaim_block(inode_block.blocks[i]);
	}
	bfs.reclaim_block(inode_block_num);
	success_message("success");
}

// display stats about file or directory
void FileSys::stat(const char* name) {
	dirblock_t d;
	inode_t inode;
	short index;
	stringstream message;
	
	if((index = block_index_retrive(name)) == -1) {
		success_message(ERR_503);
		return;
	}
	
	bfs.read_block(curr_dir_block.dir_entries[index].block_num, (void*) &d);
	if(d.magic == DIR_MAGIC_NUM) {

		message << "Directory name:     " << name << "\n";

		message << "Directory block:     " << curr_dir_block.dir_entries[index].block_num;

	} else {
		bfs.read_block(curr_dir_block.dir_entries[index].block_num, (void*) &inode);

		message << "Inode block:     " << curr_dir_block.dir_entries[index].block_num << "\n";

		message << "Bytes in file:     " << inode.size << "\n";
		if(inode.size == 0){
            //Number of blocks initally is 1
			message << "Number of blocks:     " << 1 << "\n";
            //First block initally 0
			message << "First block:     " << 0;
		} else {
            int num_blocks = (inode.size - 1) / BLOCK_SIZE + 2;

			message << "Number of blocks:     " << num_blocks << "\n";

            int f_block = inode.blocks[0];
            
			message << "First block:     " << f_block;
		}
	}
	
	success_message(message.str());
}

// HELPER FUNCTIONS (optional)

short FileSys::block_index_retrive(const char* name) {
	for(int i = 0; i < curr_dir_block.num_entries; i++) {
		if(strcmp(curr_dir_block.dir_entries[i].name, name) == 0)
			return i;
	}
	return -1;
}


void FileSys::error_message(const char* code) {
  cout << "error: " << code << endl;
  int code_len = strlen(code) + 1;
  send(fs_sock, code, code_len, 0);
}

void FileSys::success_message(string buffer) {
  string message = "200 OK";
  message += buffer;
  cout << "buffer: " << buffer << endl;
  send(fs_sock, buffer.c_str(), buffer.size(), 0);
}

/*
500 File is not a directory           (Applies to: cd, rmdir)
501 File is a directory               (Applies to: cat, head, append, rm)
502 File exists                       (Applies to: create, mkdir)
503 File does not exist               (Applies to: cd, rmdir, cat, head, append, rm, stat)
504 File name is too long             (Applies to: create, mkdir)
505 Disk is full                      (Applies to: create, mkdir, append)
506 Directory is full                 (Applies to: create, mkdir)
507 Directory is not empty            (Applies to: rmdir)
508 Append exceeds maximum file size  (Applies to: append)
200 OK                                (Applied to: all operations)
*/