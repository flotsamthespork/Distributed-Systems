#ifndef __MESSAGE_H__
#define __MESSAGE_H__

struct message {
	int alloc_length;
	int length;
	char *message;
};

void message_init(struct message *msg);
void message_destroy(struct message *msg);

void message_finish(struct message *msg);

void message_write(struct message *msg,
		char *buffer, int len);

void message_set_type(struct message *msg, int type);

#endif
