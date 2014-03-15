#ifndef __SERVER_FACTORY_H__
#define __SERVER_FACTORY_H__

struct socket {
	int socket;
	char *hostname;
	int port;
};

void socket_init(struct socket *s,
		char *hostname = 0,
		int port = -1);
void socket_destroy(struct socket *s);

bool bind_server_socket(struct socket *s);

#endif
