
#include <cstdio>
#include <map>
#include "socket.h"
#include "constants.h"
#include "message.h"
#include "db.h"

#include "binder.h"

static Database<std::pair<std::string, std::string> > m_db;
static std::map<int, std::pair<std::string, std::string> > m_servers;

static int register_proc(struct message *msg, int socket)
{
	char* server_identifier = message_read_string(msg);
	char* port = message_read_string(msg);
	char* name = message_read_string(msg);
	int* argTypes = message_read_argtypes(msg);

	std::pair<std::string, std::string> server(server_identifier, port);

	int size = m_db.get_node(name, argTypes, true)->add_value(server);
	m_servers[socket] = server;

	return size > 1 ? WARNING_DUPLICATE_FUNCTION : NONE;
}

static int loc_proc(int socket, struct message *msg)
{
	int errcode = NONE;

	char *name = message_read_string(msg);
	int *argTypes = message_read_argtypes(msg);

	struct message rmsg;
	message_start(&rmsg);

	ArgNode<std::pair<std::string, std::string> > *node =  m_db.get_node(name, argTypes, false);
	std::pair<std::string, std::string> *v = (node ? node->get_value() : 0);
	if (v)
	{
		message_set_type(&rmsg, LOC_SUCCESS);
		message_write_string(&rmsg, v->first.c_str());
		message_write_string(&rmsg, v->second.c_str());
	}
	else
	{
		errcode = INVALID_FUNCTION_SIGNATURE;
		message_set_type(&rmsg, LOC_FAILURE);
		message_write_int(&rmsg, errcode);
	}

	message_finish(&rmsg);

	if (!message_send(socket, &rmsg))
		errcode = FAILED_SEND;

	message_destroy(&rmsg);

	return errcode;
}

static bool listen_break = false;

static void terminate()
{
	struct message msg;

	message_start(&msg);
	message_set_type(&msg, TERMINATE);
	message_finish(&msg);

	std::map<int, std::pair<std::string, std::string> >::const_iterator it;
	for (it = m_servers.begin(); it != m_servers.end(); ++it)
	{
		message_send((*it).first, &msg);
	}

	message_destroy(&msg);

	listen_break = true;
}

static bool handler(int socket)
{
	bool status = true;

	struct message msg;
	if (message_receive(socket, &msg))
	{
		int errcode = NONE;
		int type = message_get_type(&msg);
		switch (type)
		{
		case REGISTER:
			errcode = register_proc(&msg, socket);

			message_destroy(&msg);
			message_start(&msg);
			message_set_type(&msg, errcode < 0 ? REGISTER_FAILURE : REGISTER_SUCCESS);
			message_write_int(&msg, errcode);
			message_finish(&msg);

			status = message_send(socket, &msg);
			break;
		case LOC_REQUEST:
			errcode = loc_proc(socket, &msg);
			break;
		case TERMINATE:
			terminate();
			status = false;
			break;
		}
	}

	message_destroy(&msg);

	if (!status)
		m_servers.erase(socket);

	return status;
}

int main()
{
	bool errcode = NONE;

	struct socket s;

	socket_init(&s);
	if (!bind_server_socket(&s))
		return 0;

	printf("BINDER_ADDRESS %s\n", s.hostname);
	printf("BINDER_PORT %s\n", s.port);

	if (!listen_socket(&s, handler, &listen_break))
		errcode = FAILED_SOCKET_LISTEN;
	socket_destroy(&s);

	return errcode;
}
