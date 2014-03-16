
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "socket.h"
#include "message.h"
#include "constants.h"
#include "db.h"

#include "rpc.h"

static Database<skeleton> m_db;
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
	int status = -1;
	if (!init_rpc_client()) return -1;

	if (bind_server_socket(&rpc_server))
	{
		status = 0;
	}
	return status;
}

int rpcCall(char* name, int* argTypes, void** args)
{
	int status = -1;
	if (!init_rpc_client()) return -1;

	struct message msg;


	return status;
}

int rpcRegister(char* name, int* argTypes, skeleton f)
{
	int status = -1;
	if (!init_rpc_client()) return -1;

	struct message msg;
	message_start(&msg);
	message_set_type(&msg, REGISTER);
	message_write_string(&msg, rpc_server.hostname);
	message_write_string(&msg, rpc_server.port);
	message_write_string(&msg, name);
	message_write_argtypes(&msg, argTypes);
	message_finish(&msg);

	if (message_send(rpc_client.socket, &msg))
	{
		message_destroy(&msg);
		if (message_receive(rpc_client.socket, &msg))
		{
			int type = message_get_type(&msg);
			if (type == REGISTER_SUCCESS)
			{
				m_db.get_node(name, argTypes, true)->add_value(f);
				status = 0;
			}
			else if (type == REGISTER_FAILURE)
			{
				int error = message_read_int(&msg);
				// TODO: Handle error
			}
		}
	}

	message_destroy(&msg);

	return status;
}

int rpcExecute()
{
	int status = -1;
	if (!init_rpc_client()) return -1;

	if (socket_isvalid(&rpc_server))
	{
		listen_socket(&rpc_server, NULL);
		status = 0;
	}
	return status;
}

int rpcTerminate()
{
	int status = -1;

	return status;
}
