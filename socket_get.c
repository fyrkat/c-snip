#define _POSIX_C_SOURCE 201112L
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int get_server_socket(char *port)
{
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE
	};

	struct addrinfo *result = NULL;
	int error = getaddrinfo(NULL, port, &hints, &result);
	if (error) {
		if (error == EAI_SYSTEM) {
			perror("Error getting address info");
		} else if (error) {
			fprintf(stderr, "Error getting address info: %s\n", gai_strerror(error));
		}
		exit(1);
	}
	int sock = -1;
	for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, 0);
		if (sock < 0) {
			perror("Error creating listening socket");
			continue;
		}
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)))
			perror("setsockopt(SO_REUSEADDR) failed");
		if (bind(sock, rp->ai_addr, rp->ai_addrlen)) {
			perror("Error binding an address");
			close(sock);
			sock = -1;
			continue;
		} 
	}
	if (sock < 0) {
		exit(1);
	}

	freeaddrinfo(result);

	if (listen(sock, 5)) {
		perror("Listen");
		exit(1);
	}
	fprintf(stderr, "Listening on port %s\n", port);
	return sock;
}


int get_client_socket(char *host, char *port)
{
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	struct addrinfo * result = NULL;
	int error = getaddrinfo(host, port, &hints, &result);
	if (error) {
		if (error == EAI_SYSTEM) {
			perror("Error getting address info");
		} else if (error) {
			fprintf(stderr, "Error getting address info: %s\n", gai_strerror(error));
		}
		exit(1);
	}
	
	int sock = -1;
	for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, 0);
		if (sock < 0) {
			perror("Error creating socket");
			continue;
		}
		if (connect(sock, rp->ai_addr, rp->ai_addrlen)) {
			perror("Error connecting");
			close(sock);
			sock = -1;
			continue;
		} 
	}
	if (sock < 0) {
		fprintf(stderr, "No connection\n");
		exit(1);
	}
	freeaddrinfo(result);
	fprintf(stderr, "Connected to %s:%s\n", host, port);
	return sock;
}
