/*
 * Echo server
 *
 * Accepts tcp/ipv4 connections on port 8000
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <ev.h>


#include "socket_get.h"

static void echo_cb(EV_P_ ev_io *w, int revents);
static void server_cb(EV_P_ ev_io *w, int revents);


int  main()
{
	struct ev_loop *loop = EV_DEFAULT;

	int listen_socket = get_server_socket("8000");

	ev_io server_watcher;
	ev_io_init(&server_watcher, server_cb, listen_socket, EV_READ);
	ev_io_start(loop, &server_watcher);

	ev_run(loop, 0);
	return 0;
}


static void
server_cb(EV_P_ ev_io *w, int revents)
{
	if ( !(revents & EV_READ) ) {
		return;
	}

	struct sockaddr_in address;
	socklen_t address_len;

	int conn = accept(w->fd, (struct sockaddr *) &address, &address_len);
	if (conn < 0) {
		perror("Error accepting connection");
	} else {
		char ipaddr[INET_ADDRSTRLEN] = "";
		inet_ntop(AF_INET, &(address.sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN);
		fprintf(stderr, "Got connection from %s on socket %d\n", ipaddr, conn);

		ev_io *watcher = malloc(sizeof(ev_io));
		ev_io_init(watcher, echo_cb, conn, EV_READ);
		ev_io_start(loop, watcher);
	}
}


static void
echo_cb(EV_P_ ev_io *w, int revents)
{
	char buf[128];
	int bytes_read = read(w->fd, buf, 128);
	if (bytes_read == 0) {
		fprintf(stderr, "Closing socket %d\n", w->fd);
		ev_io_stop(loop, w);
		close(w->fd);
		free(w);
	} else {
		fprintf(stderr, "Echoing socket %d\n", w->fd);
		write(w->fd, buf, bytes_read);
	}
}

