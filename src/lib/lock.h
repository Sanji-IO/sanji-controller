/**
 * @file lock.h
 * @brief Declaration of file lock utility
 * @date 2012-10-05
 * @version 1.0.0
 */

#ifndef _LOCK_H
#define _LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ########################
 * MACRO
 * ########################
 */

#define write_unlock(fd, offset, whence, len) \
			lock_reg(fd, F_SETLK, F_UNLCK, offset, whence, len)
#define write_lock(fd, offset, whence, len) \
			lock_reg(fd, F_SETLK, F_WRLCK, offset, whence, len)
#define writew_lock(fd, offset, whence, len) \
			lock_reg(fd, F_SETLKW, F_WRLCK, offset, whence, len)
#define is_write_lockable(fd, offset, whence, len) \
			lock_test(fd, F_WRLCK, offset, whence, len)

/*
 * ########################
 * LIBRARY FUNCTION
 * ########################
 */

extern int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);
extern int lock_test(int fd, int type, off_t offset, int whence, off_t len);

#ifdef __cplusplus
}
#endif

#endif /* _LOCK_H */
