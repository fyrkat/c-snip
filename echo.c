#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <poll.h>

#define MAX_CONNECTIONS 5

int listen_socket;
int connections[MAX_CONNECTIONS];
int num_connections = 0;


int get_listen_socket()
{
	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket < 0) {
		perror("Error creating listening socket");
		exit(1);
	}
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)))
		perror("setsockopt(SO_REUSEADDR) failed");


	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE
	};

	struct addrinfo * result = NULL;
	int error = getaddrinfo(NULL, "8000", &hints, &result);
	if (error) {
		if (error == EAI_SYSTEM) {
			perror("Error getting address info");
		} else if (error) {
			fprintf(stderr, "Error getting address info: %s\n", gai_strerror(error));
		}
		exit(1);
	}
	
	for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
		if (bind(listen_socket, rp->ai_addr, rp->ai_addrlen)) {
			perror("Error binding an address");
			exit(1);
		} 
	}
	freeaddrinfo(result);
	return listen_socket;
}


void add_connection(int sock)
{
	if (num_connections < MAX_CONNECTIONS) {
		fprintf(stderr, "Add connection\n");
		connections[num_connections++] = sock;
	} else {
		fprintf(stderr, "Maximum number of connections reached (%d)\n", MAX_CONNECTIONS);
		close(sock);
	}
}

void remove_connection(int sock)
{
	fprintf(stderr, "Remove socket %d\n", sock);
	for (int i = 0; i < num_connections; ++i) {
		if (connections[i] == sock) {
			for (int j = i; j < num_connections; j++) {
				connections[j] = connections[j+1];
			}
			num_connections--;
			break;
		}
	}
	close(sock);
}


void accept_connection(int sock)
{
	struct sockaddr_in address;
	socklen_t address_len;

	int conn = accept(sock, (struct sockaddr *) &address, &address_len);
	if (conn < 0) {
		perror("Error accepting connection in accept_connection");
	} else {
		char ipaddr[INET_ADDRSTRLEN] = "";
		inet_ntop(AF_INET, &(address.sin_addr.s_addr), ipaddr, INET_ADDRSTRLEN);
		fprintf(stderr, "Got connection from %s on socket %d\n", ipaddr, conn);
		add_connection(conn);
	}
}


void echo(int sock)
{
	fprintf(stderr, "Echoing socket %d\n", sock);
	char buf[128];
	int bytes_read = read(sock, buf, 128);
	if (bytes_read == 0) {
		remove_connection(sock);
	}
	write(sock, buf, bytes_read);
}


void main_loop()
{
	static int loop_count = 0;
	++loop_count;

	fprintf(stderr, "Loop number: %d\n", loop_count);
	fprintf(stderr, "Num open connections %d\n", num_connections);

	int num_fds = num_connections + 1;
	struct pollfd fds[num_fds];

	fds[0].fd = listen_socket;
	fds[0].events = POLLIN;
	for (int i = 0; i < num_connections; ++i) {
		fds[i+1].fd = connections[i];
		fds[i+1].events = POLLIN | POLLHUP;
	}

	int rv = poll(fds, num_fds, 10000);
	if (rv > 0) {
		fprintf(stderr, "Got %d events\n", rv);

		if (fds[0].revents & POLLIN) {
			accept_connection(listen_socket);
		}
		for (int i = 1; i < num_fds; ++i) {
			if (fds[i].revents & POLLIN) {
				echo(fds[i].fd);
			}
		}
	} else if (rv == 0) {
		fprintf(stderr, "Poll timed out.\n");
	} else {
		perror("Main loop poll error: %s\n");
	}
}

int  main()
{
	listen_socket = get_listen_socket();
	if (listen(listen_socket, 5)) {
		perror("Listen");
		exit(1);
	}
	for (;;) {
		main_loop();
	}
	return 0;
}
