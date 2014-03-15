
#include <cstdio>
#include "socket.h"

#include "binder.h"


int main()
{
	struct socket s;

	socket_init(&s);
	if (!bind_server_socket(&s))
	{
		return 0;
	}

	printf("BINDER ADDRESS %s\n", s.hostname);
	printf("BINDER PORT %s\n", s.port);


	socket_destroy(&s);

	return 0;
}
