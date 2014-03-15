
#include <cstdlib>
#include <cstring>
#include "socket.h"

#include "rpc.h"


static struct socket rpc_server;
static struct socket rpc_client;

static bool init_rpc_client()
{
	if (!socket_isvalid(&rpc_client))
	{
		const char *hostname = getenv("BINDER_ADDRESS");
		const char *port = getenv("BINDER_PORT");

		strcpy(rpc_client.hostname, hostname);
		strcpy(rpc_client.port, port);

		return connect_socket(&rpc_client);
	}

	return true;
}


int rpcInit()
{
	if (bind_server_socket(&rpc_server))
	{
		return 0;
	}
	return -1;
}

int rpcRegister(char* name, int* argTypes, skeleton f)
{
	return -1;
}
