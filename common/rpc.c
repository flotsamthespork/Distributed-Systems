
#include "server_factory.h"

#include "rpc.h"


static struct server_socket rpc_server;

int rpcInit()
{
	if (bind_server_socket(&rpc_server))
	{
		return 0;
	}
	return -1;
}
