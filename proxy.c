/*
 * Tcp proxy
 *
 * Listens on port 8001 and proxies connections to port 8000 on localhost.
 *
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>

#include <ev.h>

#include "socket_get.h"


static void server_cb(EV_P_ ev_io *w, int revents);
static void proxy_cb(EV_P_ ev_io *w, int revents);


int  main()
{
	struct ev_loop *loop = EV_DEFAULT;

	int server_socket = get_server_socket("8001");

	ev_io server_watcher;
	ev_io_init(&server_watcher, server_cb, server_socket, EV_READ);
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

	socklen_t address_len;
	struct sockaddr_in address;

	int conn = accept(w->fd, (struct sockaddr *) &address, &address_len);
	if (conn < 0) {
		perror("Error accepting connection");
	} else {
		char ipaddr[INET_ADDRSTRLEN] = "None";
		inet_ntop(AF_INET, &(address.sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN);
		fprintf(stderr, "Got connection from %s on socket %d\n", ipaddr, conn);


		int client_socket = get_client_socket("localhost", "8000");

		fprintf(stderr, "Connecting sockets %d and %d\n", conn, client_socket);

		ev_io *watcher1 = malloc(sizeof(ev_io));
		ev_io *watcher2 = malloc(sizeof(ev_io));

		ev_io_init(watcher1, proxy_cb, conn, EV_READ);
		ev_io_init(watcher2, proxy_cb, client_socket, EV_READ);
		watcher1->data = watcher2;
		watcher2->data = watcher1;

		ev_io_start(loop, watcher1);
		ev_io_start(loop, watcher2);
	}

}

static void
proxy_cb(EV_P_ ev_io *w, int revents)
{
 	ev_io *w2 = (ev_io*) w->data;
	char buf[100];
	int bytes_read = read(w->fd, buf, 100);
	if (bytes_read == 0) {
		fprintf(stderr, "Closing connection between %d and %d\n", w->fd, w2->fd);
		ev_io_stop(loop, w);
		ev_io_stop(loop, w2);
		close(w->fd);
		close(w2->fd);
		free(w);
		free(w2);
		return;
	}
	fprintf(stderr, "Writing from %d to %d\n", w->fd, w2->fd);
	write(w2->fd, buf, bytes_read);
}
