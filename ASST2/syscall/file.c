
#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <file.h>
#include <vfs.h>
#include <vnode.h>
#include <lib.h>
#include <kern/limits.h> /* For PATH_MAX */
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/stat.h>
#include <uio.h>
#include <limits.h>
#include <proc.h>
#include <current.h>
#include <copyinout.h>
#include <kern/fcntl.h>
#include <kern/seek.h>

/* Add your file-system related functions here ... */

static int opened_files_count=0;
static int open_file(char *kernel_file_name, int filehandler, int openflags){
	struct file_t *file = kmalloc(sizeof(struct file_t*));
	if(file == NULL) {
		return ENFILE;
	}	

	int error;
	error = vfs_open(kernel_file_name, openflags, 0, &file->v_ptr);
	if (error) {
		kfree(file);
		return error;
	}

	file->file_lock = lock_create("create file lock");
	if(file->file_lock) {
		curproc->file_descriptor_table[filehandler] = file;
		file->offset = 0;
		file->openflags = openflags;
		file->references = 1;
		return 0;
	}
	else {
		vfs_close(file->v_ptr);
		kfree(file);
		return ENFILE;
	}	
}
static int check_filehanlder(int filehandler){
	if(filehandler < 0){
		return EBADF;			
	}
	if(filehandler >= MAX_PROCESS_OPEN_FILES){
		return EBADF;			
	}
	 
	if(curproc->file_descriptor_table[filehandler]==NULL) {	
		return EBADF;
	}
	return 0;
}
static void uio_initialize( struct uio *uio_obj,
	  userptr_t buffer, size_t len, off_t position, enum uio_rw rw)
{
	struct iovec* iov = kmalloc(sizeof(struct iovec));
	iov->iov_kbase = buffer;
	iov->iov_len = len;
	uio_obj->uio_iov = iov;
	uio_obj->uio_segflg = UIO_USERSPACE;
	uio_obj->uio_rw = rw;
	uio_obj->uio_iovcnt = 1;
	uio_obj->uio_offset = position;
	uio_obj->uio_resid = len;
	uio_obj->uio_space = proc_getas();
}
int sys_write(int filehandler, userptr_t buffer, size_t size, int *retval) {	
	if(filehandler<=2) {
		kprintf("%s",(char *)buffer);
		return 0;
	}
	
	int error = check_filehanlder(filehandler);
	if (error){
		return error;
	}

	struct file_t *file = curproc->file_descriptor_table[filehandler];
	int how = file->openflags & O_ACCMODE;
	if (how == O_RDONLY) {
		return EBADF;
	}

	lock_acquire(file->file_lock);
	off_t old_offset = file->offset;
	
	struct uio uio_obj;
	uio_initialize( &uio_obj, buffer, size, file->offset, UIO_WRITE);

	error = VOP_WRITE(file->v_ptr, &uio_obj);
	if (error) {
		lock_release(file->file_lock);
		return error;
	}
	else {
		*retval = uio_obj.uio_offset - old_offset;
		file->offset = uio_obj.uio_offset;
		lock_release(file->file_lock);	
		return 0;
	}
}


int sys_open(userptr_t file_name, int openflags, int *retval) {
	if (file_name == NULL) {
		return EFAULT;
	}

	if (opened_files_count>=MAX_SYSTEM_OPEN_FILES)
	{
		return ENFILE;
	}

	char *kernel_file_name = kmalloc((PATH_MAX)*sizeof(char));
	if (kernel_file_name == NULL) {
		return ENFILE;
	}
	
	size_t got_len;
	int error;
	error = copyinstr(file_name, kernel_file_name, PATH_MAX + 1, &got_len);
	if (error) {
		kfree(kernel_file_name);
		return error;
	}

	int i, filehandler=0;
	for (i=0; i <= MAX_PROCESS_OPEN_FILES; i++) {
		if(i<=2) continue;	
		if (i == MAX_PROCESS_OPEN_FILES) {
			kfree(kernel_file_name);
			return EMFILE;
		}	
		if (curproc->file_descriptor_table[i] == NULL) {
			filehandler = i;			
			break;
		}
	}
	
	error = open_file(kernel_file_name, filehandler, openflags);
	if(error){
		kfree(kernel_file_name);
		return error;
	}
	else{
		opened_files_count = opened_files_count+1;
		*retval = filehandler;
		return 0;
	}
}
int sys_close(int filehandler){
	int error = check_filehanlder(filehandler);
	if (error){
		return error;
	}

	struct file_t *file = curproc->file_descriptor_table[filehandler];

	lock_acquire(file->file_lock);
	curproc->file_descriptor_table[filehandler] = NULL;
	file->references --;

	if(file->references==0) {
		lock_release(file->file_lock); 
		vfs_close(file->v_ptr);
		lock_destroy(file->file_lock);
		kfree(file);
	}
	else{
		lock_release(file->file_lock);
	}
	opened_files_count--;
	return 0;
}

int sys_read(int filehandler, userptr_t buffer, size_t size, int *retval) {
	int error = check_filehanlder(filehandler);
	if (error){
		return error;
	}

	struct file_t *file = curproc->file_descriptor_table[filehandler];

	int how = file->openflags & O_ACCMODE;
	if (how == O_WRONLY) {
		return EBADF;
	}
	lock_acquire(file->file_lock);
	off_t old_offset = file->offset;

	struct uio uio_obj;
	uio_initialize( &uio_obj, buffer, size, file->offset, UIO_READ);

	error = VOP_READ(file->v_ptr, &uio_obj);
	if (error) {
		lock_release(file->file_lock);
		return error;
	}
	else {
		*retval = uio_obj.uio_offset - old_offset;
		file->offset = uio_obj.uio_offset;
		lock_release(file->file_lock);
		return 0;
	}
	
}
int sys_lseek(int filehandler, off_t position, userptr_t mode_ptr, off_t *retval){
	int error = check_filehanlder(filehandler);
	if (error){
		return error;
	}

	struct file_t *file = curproc->file_descriptor_table[filehandler];
	
	
	struct stat status;
	error = VOP_STAT(file->v_ptr, &status);
	if(error){
		return error;
	}	

	if(!VOP_ISSEEKABLE(file->v_ptr)){
		return ESPIPE;
	}

	int mode;
	error = copyin(mode_ptr, &mode, sizeof(int));
	if(error) {
		return error;
	}

	if(mode==SEEK_SET){
		if(position < 0){
			return EINVAL;
		}
		lock_acquire(file->file_lock);
		file->offset = position;
		*retval = position;
		lock_release(file->file_lock);
		return 0;
	}
	else if(mode==SEEK_CUR){
		lock_acquire(file->file_lock);
		off_t target = file->offset + position;
		if(target < 0){
			lock_release(file->file_lock);
			return EINVAL;
		}
		file->offset = target;
		*retval = target;
		lock_release(file->file_lock);
		return 0;
	}
	else if(mode==SEEK_END){
		off_t target = status.st_size + position;
		if(target < 0){
			return EINVAL;
		}
		lock_acquire(file->file_lock);
		file->offset = target;
		*retval = target;
		lock_release(file->file_lock);
		return 0;
	}
	else{
		return EINVAL;
	}

}

