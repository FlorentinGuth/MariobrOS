#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "malloc.h"
#include "string.h"
#include "fs_types.h"

/**
 *  @name std_buf - The standard buffer, one block wide, used by most functions
 *  of the filesystem
 */
u_int8 *std_buf;

/**
 * @name std_inode - The standard inode container, similar to std_buf for inodes
 */
inode_t *std_inode;

/**
 *  @name set_inode  - Sets an inode buffer to the corresponding inode number
 *  @param inode     - The inode number
 *  @param buffer    - The output buffer of the inode, must be at least 128B wide
 */
void set_inode(u_int32 inode, inode_t *buffer);

/**
 *  @name allocate_inode - Allocates a new inode
 *  @return ret          - The inode number of the new used inode. 
 *                         0 if there is no free inode available
 */
u_int32 allocate_inode();

/**
 *  @name unallocate_inode - Unallocates an inode
 *  @param inode - The inode to free
 *  @return      - 0: No error
 *                 1: The inode was not allocated before. No change was made
 */
u_int8 unallocate_inode(u_int32 inode);

/**
 *  @name allocate_block - Allocates a new block
 *  @return ret          - The block number of the new used block. 
 *                         0 if there is no free block available
 */
u_int32 allocate_block(u_int32 prec);

/**
 *  @name unallocate_block - Unallocates a block
 *  @param inode - The block to free
 *  @return      - 0: No error
 *                 1: The block was not allocated before. No change was made
 */
u_int8 unallocate_block(u_int32 block);

/**
 *  @name read_inode_data - Copies a certain amount of data from the disk to
 *  a given buffer. Only reads up to the end of the current block.
 *
 *  @param inode  - The inode number of the file to read
 *  @param buffer - The output buffer
 *  @param offset - Offset within the file, in words (NOT in bytes)
 *  @param length - The length of the data to copy, in words (NOT in bytes)
 *
 *  @return       - The actually read length of data
 */
u_int32 read_inode_data(u_int32 inode, u_int8* buffer, u_int32 offset, \
                        u_int32 length);

/**
 *  @name write_inode_data - Writes a certain amount of data to the disk from
 *  a given buffer. Only writes up to the end of the current block.
 *  Also sets up std_inode to the content of the inode
 *
 *  @param inode  - The inode number of the file to write
 *  @param buffer - The input buffer
 *  @param offset - Offset within the file, in words (NOT in bytes)
 *  @param length - The length of the data to copy, in words (NOT in bytes)
 *
 *  @return       - The actually written length of data
 */
u_int32 write_inode_data(u_int32 inode, u_int8* buffer, u_int32 offset, \
                         u_int32 length);


/**
 *  @name find_inode - Opens a file
 *  Also sets up std_inode to the content of the parent inode, and std_buf to
 *  the content of the parent directory data block
 *
 *  @param str_path - The path to the file, relative to root
 *  @param root     - If path begins with '/', this argument is ignored
 *  @return inode   - The inode number of the file
 */
u_int32 find_inode(string str_path, u_int32 root);

/**
 *  @name add_file - Adds a file to a directory
 *
 *  @param dir       - The inode number of the directory
 *  @param inode     - The inode number of the file to add
 *  @param file_type - The file type for the directory entry (FILE_...)
 *  @param name      - A string containing the name of the new file
 *
 *  @return          - 0: No error
 *                     1: Already a file with the same name in the directory
 *                     2: No room for another file in this directory
 *                     3: Name is too long (must be < 256 characters)
 *                     4: No directory
 *                     5: No inode
 */
u_int8 add_file(u_int32 dir, u_int32 inode, u_int8 file_type, string name);

/**
 *  @name remove_file - Removes the file from the directory
 *  WARNING: Does not erase the file itself, only removes it from search
 *
 *  @param dir        - The parent directory
 *  @param inode      - The inode number of the file to remove
 *
 *  @param return     - 0: No error
 *                      1: No such file found in the directory
 *                      4: No directory
 *                      5: No inode
 */
u_int8 remove_file(u_int32 dir, u_int32 inode);

/**
 *  @name erase_file_data - Unallocates all the blocks used by a file
 *  @param inode          - The inode number of the file to erase
 *  NOTE : the inode will not be unallocated
 */
void erase_file_data(u_int32 inode);

/**
 *  @name delete_file - Deletes a file from the filesystem
 *  @param dir        - The parent directory
 *  @param inode      - The inode number of the file to remove
 *  @return           - Same as remove_file
 */
u_int8 delete_file(u_int32 dir, u_int32 inode);

/**
 *  @name create_file - Creates a file
 *
 *  @param father    - The inode number of the father directory of this one
 *  @param name      - The name of the directory
 *  @param type      - The file type, for the inode (PERM_... | TYPE_...)
 *  @param ftype     - The file type, for the directory (FILE_...)
 *
 *  @return num      - The inode number of the created file
 */
u_int32 create_file(u_int32 father, string name, u_int16 type, u_int8 ftype);

/**
 *  @name create_dir - Creates a directory
 *  @param father    - The inode number of the father directory of this one
 *  @param name      - The name of the directory
 *  @return num      - The inode number of the created directory
 */
u_int32 create_dir(u_int32 father, string name);


/**
 *  @name  ls_dir - Prints to the screen the contents of a directory
 *  @param inode  - The inode number of the directory
 */
void ls_dir(u_int32 inode);

/**
 *  @name filesystem_install - initializes all the necessary constants of the fs
 */
void filesystem_install();

#endif
