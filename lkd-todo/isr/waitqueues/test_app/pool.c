/*
poll() is a userspace function which provides application to monitor I/O events on a set of file descriptors.

#include <poll.h>
int poll(struct pollfd fds[], nfds_t nfds, int timeout);// polls over an array of structures

The <poll.h> header shall define the pollfd structure, which shall include at least the following members:
int    fd       The following descriptor being polled. 
short  events   The input event flags 
short  revents  The output event flags  

nfds means the number of file descriptors on which polling is being done.
Timeout means the poll function will wait for this many milliseconds for the vent to occur.
If the value of timeout is 0 then the function return immediately.
if the vale of timeout is -1 then poll shall bloack untill a requested event has occured or untill the call is interrupted.

The poll function supports for regular files, terminal and pseudo-terminal devices, FIFOs, pipes, sockets and  STREAMS-based files.

fds[0].fd = open("/dev/dev0", ...);
fds[1].fd = open("/dev/dev1", ...);
fds[0].events = POLLOUT | POLLWRBAND;
fds[1].events = POLLOUT | POLLWRBAND;
poll(fds, 2, timeout_msecs);

The system uses a poll_wait call to indicate that the poll system is interested in events. 
The poll_wait call includes a reference to a wait queue that must be triggered by a driver event.
Another argument to poll_wait function is a poll_table structure.
*/

#define _XOPEN_SOURCE 700
#include <fcntl.h> /* creat, O_CREAT */
#include <poll.h> /* poll */
#include <stdio.h> /* printf, puts, snprintf */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */
#include <unistd.h> /* read */

int main(int argc, char **argv) {

	char buf[1024];
	int fd, n, i;
	short revents;
	struct pollfd pfd;

	fd = open(argv[1], O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	pfd.fd = fd;
	pfd.events = POLLIN;
	while (1) {
		puts("loop");
		i = poll(&pfd, 1, -1);
		if (i == -1) {
			perror("poll");
			exit(EXIT_FAILURE);
		}

		revents = pfd.revents;
		if (revents & POLLIN) {
			n = read(pfd.fd, buf, sizeof(buf));
			printf("POLLIN n=%d buf=%.*s\n", n, n, buf);
		}
		if(revents & POLLHUP) {
			printf("POLLHUP\n");
			close(pfd.fd);
			break;
		}
		if (revents & POLLNVAL) {
			printf("POLLNVAL\n");
		}
		if (revents & POLLERR) {
			printf("POLLERR\n");
		}
	}
}
