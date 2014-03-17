
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <pthread.h>
#include "socket.h"
#include "message.h"
#include "constants.h"
#include "db.h"
#include "util.h"

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

static int execute_remote(char* server_identifier, char* port, char* name, int* argTypes, void** args)
{
	int errcode = NONE;

	struct socket s;
	socket_init(&s, server_identifier, port);
	if (connect_socket(&s))
	{
		struct message msg;
		message_start(&msg);
		message_set_type(&msg, EXECUTE);
		message_write_string(&msg, name);
		message_write_argtypes(&msg, argTypes);
		message_write_args(&msg, argTypes, args);
		message_finish(&msg);

		if (message_send(s.socket, &msg))
		{
			message_destroy(&msg);
			if (message_receive(s.socket, &msg))
			{
				int type = message_get_type(&msg);
				if (type == EXECUTE_SUCCESS)
				{
					message_read_string(&msg);
					message_read_argtypes(&msg);
					void **exec_args = message_read_args(&msg, argTypes);

					int offset = 0;
					for (int i = 0; argTypes[i] != 0; i++)
					{
						if ((argTypes[i] >> ARG_OUTPUT) & 1)
						{
							int type = (argTypes[i] >> 16) & 0xFF;
							int arrlen = argTypes[i] & 0xFFFF;
							if (arrlen <= 0)
								arrlen = 1;

							int argsize = get_argsize(type);

							memcpy(args[i], exec_args[i], argsize*arrlen);
							offset += argsize*arrlen;
						}
					}
				}
				else if (type == EXECUTE_FAILURE)
					errcode = message_read_int(&msg);
			}
			else
				errcode = FAILED_RECEIVE;
		}
		else
			errcode = FAILED_SEND;

		message_destroy(&msg);
	}

	socket_destroy(&s);

	return errcode;
}

static bool server_handler(int socket)
{
	bool failed = false;

	struct message msg;
	if (message_receive(socket, &msg))
	{
		int type = message_get_type(&msg);
		if (type == EXECUTE)
		{
			int errcode = NONE;

			char* name = message_read_string(&msg);
			int* argTypes = message_read_argtypes(&msg);
			void** args = message_read_args(&msg, argTypes);

			ArgNode<skeleton> *node = m_db.get_node(name, argTypes, false);
			if (node != 0)
			{
				if ((*node->get_value())(argTypes, args) < 0)
					errcode = FAILED_FUNCTION_CALL;
			}
			else
				errcode = INVALID_FUNCTION_SIGNATURE;

			struct message rmsg;
			message_start(&rmsg);
			if (errcode < 0)
			{
				message_set_type(&rmsg, EXECUTE_FAILURE);
				message_write_int(&rmsg, errcode);
			}
			else
			{
				message_set_type(&rmsg, EXECUTE_SUCCESS);
				message_write_string(&rmsg, name);
				message_write_argtypes(&rmsg, argTypes);
				message_write_args(&rmsg, argTypes, args);
			}
			message_finish(&rmsg);

			if (!message_send(socket, &rmsg))
				failed = true;

			message_destroy(&rmsg);
		}
		else if (type == TERMINATE)
		{
			failed = true;
		}
	}
	else
		failed = true;

	message_destroy(&msg);
	return failed;
}

int rpcInit()
{
	int errcode = NONE;
	if (!init_rpc_client()) return FAILED_BINDER_CONNECT;

	if (!bind_server_socket(&rpc_server))
		errcode = FAILED_SOCKET_BIND;

	return errcode;
}

int rpcCall(char* name, int* argTypes, void** args)
{
	int errcode = NONE;
	if (!init_rpc_client()) return FAILED_BINDER_CONNECT;

	struct message msg;

	message_start(&msg);
	message_set_type(&msg, LOC_REQUEST);
	message_write_string(&msg, name);
	message_write_argtypes(&msg, argTypes);
	message_finish(&msg);

	if (message_send(rpc_client.socket, &msg))
	{
		message_destroy(&msg);
		if (message_receive(rpc_client.socket, &msg))
		{
			int type = message_get_type(&msg);
			if (type == LOC_SUCCESS)
			{
				char* server_identifier = message_read_string(&msg);
				char* port = message_read_string(&msg);

				errcode = execute_remote(server_identifier, port, name, argTypes, args);
			}
			else if (type == LOC_FAILURE)
				errcode = message_read_int(&msg);
		}
		else
			errcode = FAILED_RECEIVE;
	}
	else
		errcode = FAILED_SEND;

	message_destroy(&msg);

	return errcode;
}

int rpcRegister(char* name, int* argTypes, skeleton f)
{
	int errcode = NONE;
	if (!init_rpc_client()) return FAILED_BINDER_CONNECT;

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
			}
			else if (type == REGISTER_FAILURE)
				errcode = message_read_int(&msg);
		}
		else
			errcode = FAILED_RECEIVE;
	}
	else
		errcode = FAILED_SEND;

	message_destroy(&msg);

	return errcode;
}

static bool listen_break = false;

static void* binder_thread(void* arg)
{
	struct message msg;
	message_receive(rpc_client.socket, &msg);
	int type = message_get_type(&msg);
	assert(type == TERMINATE);
	listen_break = true;
	GRACEFUL_EXIT;
}

int rpcExecute()
{
	int errcode = NONE;
	if (!init_rpc_client()) return FAILED_BINDER_CONNECT;

	if (socket_isvalid(&rpc_server))
	{
		pthread_t thread;
		pthread_create(&thread, 0, &binder_thread, 0);
		if (!listen_socket(&rpc_server, server_handler, &listen_break))
			errcode = FAILED_SOCKET_LISTEN;
	}
	else
		errcode = INVALID_SOCKET;

	return errcode;
}

int rpcTerminate()
{
	int errcode = NONE;

	struct message msg;

	message_start(&msg);
	message_set_type(&msg, TERMINATE);
	message_finish(&msg);

	if (!message_send(rpc_client.socket, &msg))
		errcode = FAILED_SEND;

	message_destroy(&msg);

	return errcode;
}
