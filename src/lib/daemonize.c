/**
 * @file daemonize.c
 * @brief To daemonize the program
 * @date 2014-08-27
 * @version 1.0.0
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * fork current process into the background.
 *
 * This function forks a process into the background, in order to
 * become a daemon process. It does a few things along the way:
 *
 * - becoming a process/session group leader, and  forking a second time so
 *   that process/session group leader can exit.
 *
 * - changing the working directory to /
 *
 * - closing stdin, stdout and stderr (unless stderr_log is set) and
 *   redirecting them to /dev/null
 *
 * @param enable_stdio indicates if stderr/stdin/stdout is being used for
 *                     logging and shouldn't be closed
 * @return 0:OK, negative: failure
 */
int
daemonize_process(int enable_stdio)
{
	pid_t pid;

	/* ignore signal */
#ifdef SIGHUP
	signal(SIGHUP, SIG_IGN);
#endif
#ifdef SIGTSTP
	signal( SIGTSTP, SIG_IGN );
#endif
#ifdef SIGTTIN
	signal( SIGTTIN, SIG_IGN );
#endif
#ifdef SIGTTOU
	signal( SIGTTOU, SIG_IGN );
#endif

    /*
     * Fork to return control to the invoking process and to
     * guarantee that we aren't a process group leader.
     */
	if ((pid = fork()) > 0) exit(0);
	else if (pid < 0) return -1;

	/* Become a process/session group leader. */
	if(-1 == setsid()) return -1;

	/*
	 * Fork second times to let the process/session group leader exit.
	 */
	if ((pid = fork()) > 0) exit(0);
	else if (pid < 0) return -1;

    /* Avoid keeping any directory in use. */
    chdir("/");

	if (enable_stdio) return 0;

    /*
     * Close inherited file descriptors to avoid
     * keeping unnecessary references.
     */
    close(0);
    close(1);
    close(2);

    /*
     * Redirect std{in,out,err} to /dev/null, just in case.
     */
    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

	return 0;
}

