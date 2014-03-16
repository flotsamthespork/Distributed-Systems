#ifndef __SERVER_FACTORY_H__
#define __SERVER_FACTORY_H__

struct socket {
	int socket;
	char hostname[512];
	char port[10];
};

void socket_init(struct socket *s,
		char *hostname = 0,
		char *port = 0);
void socket_destroy(struct socket *s);
bool socket_isvalid(struct socket *s);

bool bind_server_socket(struct socket *s);

bool connect_socket(struct socket *s);
int listen_socket(struct socket *s, int(*handler)(int));

#endif
