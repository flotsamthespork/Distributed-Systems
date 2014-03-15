#ifndef __SERVER_FACTORY_H__
#define __SERVER_FACTORY_H__

struct server_socket {
	int socket;
	char *hostname;
	int port;
};

void server_socket_init(struct server_socket *s);
void server_socket_destroy(struct server_socket *s);


bool bind_server_socket(struct server_socket *s);

#endif
