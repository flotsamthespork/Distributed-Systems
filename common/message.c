
#include "message.h"

#define HEADER_LENGTH (2*sizeof(int))

static void
increase_size(struct message *msg, int len = -1)
{
	if (len == -1)
	{
		len = msg->alloc_length*2;
	}

	if (msg->message)
		msg->message = (char*) realloc(msg->message, len);
	else
		msg->message = (char*) malloc(len);
	msg->alloc_length = len;
}

void message_init(struct message *msg)
{
	msg->length = HEADER_LENGTH;
	msg->message = 0;
	increase_size(msg, 512*sizeof(int));
}

void message_destroy(struct message *msg)
{
	free(msg->message);
}

void message_finish(struct message *msg)
{
	((int*)msg->message)[0] = msg->length - HEADER_LENGTH;
}

void message_write(struct message *msg,
		char *buffer, int len)
{
	if (msg->alloc_length < (msg->length+len))
	{
		increase_size(msg, msg->alloc_length+len);
	}

	memcpy(msg->message+msg->length, buffer, len);
	msg->length += len;
}


void message_set_type(struct message *msg, int type)
{
	((int*)msg->message)[1] = type;
}
