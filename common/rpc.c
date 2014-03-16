
#include <cstdlib>
#include <cstring>
#include "socket.h"

#include "rpc.h"


static struct socket rpc_server;
static struct socket rpc_client;
static bool rpc_client_is_init = false;

static bool init_rpc_client()
{
	if (!rpc_client_is_init)
	{
		socket_init(&rpc_client);
		rpc_client_is_init = true;
	}
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
	init_rpc_client();

	if (bind_server_socket(&rpc_server))
	{
		return 0;
	}
	return -1;
}

int rpcCall(char* name, int* argTypes, void** args)
{
	init_rpc_client();



	return -1;
}

int rpcRegister(char* name, int* argTypes, skeleton f)
{
	init_rpc_client();



	return -1;
}

int rpcExecute()
{
	init_rpc_client();
	if (socket_isvalid(&rpc_server)) {
		listen_socket(&rpc_server, NULL);
		return 0;
	}
	return -1;
}

int rpcTerminate()
{
	return -1;
}
