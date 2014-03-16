#ifndef __MESSAGE_H__
#define __MESSAGE_H__

struct message {
	int alloc_length;
	int length;
	char *message;
	char *msg_ptr;
};

void message_init(struct message *msg);
void message_destroy(struct message *msg);

void message_finish(struct message *msg);

void message_write(struct message *msg,
		char *buffer, int len);

void message_set_type(struct message *msg, int type);

void message_write_int(struct message *msg, int value);
int message_read_int(struct message *msg);

void message_write_string(struct message *msg, char* value);
char* message_read_string(struct message *msg);

void message_write_argtypes(struct message *msg, int* argtypes);
int* message_read_argtypes(struct message *msg);

void message_write_args(struct message *msg, int* argtypes, void** args);
void** message_read_args(struct message *msg, int* argtypes);

#endif
