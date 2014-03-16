
#include <cstdio>
#include "socket.h"

#include "binder.h"

static int handler(int socket)
{
	fprintf(stdout, "HANDLED SON");
}

int main()
{
	struct socket s;

	socket_init(&s);
	if (!bind_server_socket(&s))
	{
		return 0;
	}

	printf("BINDER_ADDRESS %s\n", s.hostname);
	printf("BINDER_PORT %s\n", s.port);

	listen_socket(&s, handler);

	socket_destroy(&s);

	return 0;
}
