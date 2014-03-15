
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <stdlib.h>
#include <vector>
#include <cstdlib>

#include "socket.h"

void
socket_init(struct socket *s,
		char *hostname,
		char *port)
{
	s->socket = -1;
	s->hostname[0] = '\0';
	s->port[0] = '\0';
	if (hostname)
		strcpy(s->hostname, hostname);

	if (port)
		strcpy(s->port, port);
}

void
socket_destroy(struct socket *s)
{
	if (s->socket != -1) {
		close(s->socket);
	}
	s->hostname[0] = '\0';
	s->port[0] = '\0';
	s->socket = -1;
}

bool
socket_isvalid(struct socket *s)
{
	return s->socket != -1;
}

static bool
fill_server_info(struct socket *s)
{
	int status;
#define HOSTNAME_LEN 512
	char hostname[HOSTNAME_LEN];
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	status = getsockname(s->socket, (struct sockaddr*)&addr, &len);
	if (status != 0)
	{
		printf("Error(getsockname)\n");
		return false;
	}

	status = gethostname(hostname, HOSTNAME_LEN);
	if (status != 0)
	{
		printf("Error(gethostname)\n");
		return false;
	}
	strcpy(s->hostname, hostname);
	sprintf(s->port, "%d", ntohs(addr.sin_port));

	return true;
}

bool
bind_server_socket(struct socket *s)
{
	int conn_socket;
	int status;
	struct addrinfo hints;
	struct addrinfo *info;
	struct addrinfo *info_itr;

	socket_destroy(s);
	socket_init(s);


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(0, "0", &hints, &info);
	if (status != 0)
	{
		printf("Error(getaddrinfo):\n");
		return false;
	}

	for (info_itr = info; info_itr != NULL; info_itr = info_itr->ai_next)
	{
		conn_socket = socket(info_itr->ai_family,
				info_itr->ai_socktype,
				info_itr->ai_protocol);
		if (conn_socket == -1)
		{
			continue;
		}

		int yes = 1;
		if (setsockopt(conn_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			printf("ERROR!");
			exit(1);
		}

		status = bind(conn_socket,
				info_itr->ai_addr,
				info_itr->ai_addrlen);
		if (status == -1)
		{
			close(conn_socket);
			continue;
		}
		break;
	}

	if (info_itr == NULL)
	{
		printf("Error(bind)\n");
		return false;
	}

	freeaddrinfo(info);

	s->socket = conn_socket;

	if (!fill_server_info(s))
	{
		socket_destroy(s);
		return false;
	}

	return true;
}


bool connect_socket(struct socket *s)
{
	addrinfo hints;
	addrinfo *sa;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(s->hostname, s->port, &hints, &sa) != 0) {
//		std::cerr << "Could not get host information" << std::endl;
		fprintf(stderr, "Could not get host information\n");
	    return false;
	}

	addrinfo *p;
	int sock;

	for (p = sa; p != NULL; p = p->ai_next) {
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sock == -1) {
//			std::cerr << "Could not create socket" << std::endl;
			fprintf(stderr, "Could not create socket\n");
		} else {
			if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
				close(sock);
//				std::cerr << "Failed to connect to socket" << std::endl;
				fprintf(stderr, "Failed to connect to socket\n");
			} else {
				break;
			}
		}
	}

	if (p == NULL)
		return false;

	s->socket = sock;
	return true;
}
