/**
* This file is added for assignment 2
**/
#ifndef __FILE_H
#define __FILE_H

#include <types.h>
#include <synch.h>
#include <vnode.h>

/* Some limits  */

/* This is the maximum number of files that can be opened per
 * process */
#define MAX_PROCESS_OPEN_FILES  16

/* This is the maximum number of files that can be open in the system
 * at any one time */
#define MAX_SYSTEM_OPEN_FILES   64 

/* Place your data-structures here ... */
struct vnode;
struct lock;
struct file_t {
    off_t offset;
    int openflags;
    int32_t references;
    struct lock *file_lock;
    struct vnode *v_ptr;
};


int sys_write(int filehandler, userptr_t buffer, size_t size, int *retval);
int sys_open(userptr_t file_name, int openflags, int *retval);
int sys_close(int filehandler);
int sys_read(int filehandler, userptr_t buf, size_t size, int *retval);
int sys_lseek(int filehandler, off_t position, userptr_t mode_ptr, off_t *retval);
#endif
