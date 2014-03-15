
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

#include "socket.h"

void
socket_init(struct socket *s,
		char *hostname,
		int port)
{
	s->socket = -1;
	s->hostname = hostname;
	s->port = port;
}

void
socket_destroy(struct socket *s)
{
	// TODO - unbind the socket 
	if (s->hostname)
		free(s->hostname);
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

	s->hostname = (char*) malloc(sizeof(char)*(strlen(hostname)));
	s->port = ntohs(addr.sin_port);

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
