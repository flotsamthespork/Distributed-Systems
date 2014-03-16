
#include <cstdio>
#include "socket.h"
#include "constants.h"
#include "message.h"
#include "db.h"

#include "binder.h"

static Database<std::pair<std::string, std::string> > m_db;

static int register_proc(struct message *msg)
{
	char* server_identifier = message_read_string(msg);
	char* port = message_read_string(msg);
	char* name = message_read_string(msg);
	int* argTypes = message_read_argtypes(msg);

	std::pair<std::string, std::string> server(server_identifier, port);

	m_db.get_node(name, argTypes, true)->add_value(server);

	return NONE;
}

static bool handler(int socket)
{
	bool status = false;
	int errcode = NONE;

	struct message msg;
	if (!message_receive(socket, &msg)) return -1;

	int type = message_get_type(&msg);
	switch (type)
	{
	case REGISTER:
		errcode = register_proc(&msg);

		message_destroy(&msg);
		message_start(&msg);
		message_set_type(&msg, errcode < 0 ? REGISTER_FAILURE : REGISTER_SUCCESS);
		message_write_int(&msg, errcode);
		message_finish(&msg);

		if (message_send(socket, &msg))
			status = true;
		break;
	case LOC_REQUEST:

		break;
	case TERMINATE:

		break;
	}

	return status;
}

int main()
{
	struct socket s;

	socket_init(&s);
	if (!bind_server_socket(&s, "40000")) // TODO: Change port back to 0
	{
		return 0;
	}

	printf("BINDER_ADDRESS %s\n", s.hostname);
	printf("BINDER_PORT %s\n", s.port);

	listen_socket(&s, handler);
	socket_destroy(&s);

	return 0;
}
