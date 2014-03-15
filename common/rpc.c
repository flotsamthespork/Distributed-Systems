
#include "socket.h"

#include "rpc.h"


static struct socket rpc_server;

int rpcInit()
{
	if (bind_server_socket(&rpc_server))
	{
		return 0;
	}
	return -1;
}
