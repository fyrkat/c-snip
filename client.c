/*
 * Simple program to test the proxy
 *
 * It reads from stin and writes to stdout.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <ev.h>

#include "socket_get.h"

int sock;

static void stdin_cb(EV_P_ ev_io *w, int revents);
static void sock_cb(EV_P_ ev_io *w, int revents);

int main()
{
	struct ev_loop *loop = EV_DEFAULT;

	sock = get_client_socket("localhost", "8001");

	ev_io sock_watcher;
	ev_io_init(&sock_watcher, sock_cb, sock, EV_READ);
	ev_io_start(loop, &sock_watcher);

	ev_io stdin_watcher;
	ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
	ev_io_start(loop, &stdin_watcher);

	ev_run(loop, 0);
	return 0;
}


static void
stdin_cb(EV_P_ ev_io *w, int revents) 
{
	char buf[100];
	int bytes = read(w->fd, buf, 100);
	if (bytes == 0) {
		ev_break(loop, EVBREAK_ONE);
		return;
	}
	fprintf(stderr, "Read %d bytes from stdin and writing back over socket\n", bytes);
	write(sock, buf, bytes);
}

static void
sock_cb(EV_P_ ev_io *w, int revents)
{
	char buf[100];
	int bytes = read(w->fd, buf, 100);
	if (bytes == 0) {
		ev_break(loop, EVBREAK_ONE);
		return;
	}
	fprintf(stderr, "Read %d bytes from socket and writing to stdout\n", bytes);
	write(STDOUT_FILENO, buf, bytes);
}

