/**
 * @file lock.c
 * @brief Define file lock utility
 * @date 2012-10-05
 * @version 1.0.0
 */

#include <fcntl.h>
#include <unistd.h>

int
lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;

	lock.l_type = type;		/* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;	/* byte offset relative to l_whence */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;		/* #bytes (0 means to EOF) */
	lock.l_pid = getpid();	/* out pid */

	return (fcntl(fd, cmd, &lock));	/* -1 on error */
}

int
lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;

	lock.l_type = type;		/* F_RDLCK or F_WRLCK */
	lock.l_start = offset;	/* byte offset relative to l_whence */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;		/* #bytes (0 means to EOF) */

	if (fcntl(fd, F_GETLK, &lock) == -1) return -1;
	
	if (lock.l_type == F_UNLCK) return 0;

	return lock.l_pid;	/* PID of lock owner */
}
