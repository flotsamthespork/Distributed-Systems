
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include "util.h"
#include "rpc.h"
#include "constants.h"

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
	msg->msg_ptr = msg->message+HEADER_LENGTH;
	msg->alloc_length = len;
}

void message_start(struct message *msg)
{
	msg->length = HEADER_LENGTH;
	msg->message = NULL;
	increase_size(msg, 512*sizeof(int));
}

void message_finish(struct message *msg)
{
	((int*)msg->message)[0] = msg->length - HEADER_LENGTH;
}

void message_destroy(struct message *msg)
{
	free(msg->message);
}

void message_write(struct message *msg,
		const char *buffer, int len)
{
	if (msg->alloc_length < msg->length+len)
	{
		increase_size(msg, msg->alloc_length+len);
	}

	memcpy(msg->message+msg->length, buffer, len);
	msg->length += len;
}

int message_get_type(struct message *msg)
{
	return ((int*)msg->message)[1];
}

void message_set_type(struct message *msg, int type)
{
	((int*)msg->message)[1] = type;
}

void message_write_int(struct message *msg, int value)
{
	message_write(msg, (char*)&value, sizeof(int));
}

int message_read_int(struct message *msg)
{
	int value = (int) ((int*)msg->msg_ptr)[0];
	msg->msg_ptr += sizeof(int);
	return value;
}

void message_write_string(struct message *msg, const char* value)
{
	message_write(msg, value, strlen(value)+1);
}

char* message_read_string(struct message *msg)
{
	char* value = msg->msg_ptr;
	msg->msg_ptr += strlen(value)+1;
	return value;
}

void message_write_argtypes(struct message *msg, int* argtypes)
{
	int len = 0;
	while (argtypes[len++]);
	message_write(msg, (char*)argtypes, len*sizeof(int));
}

int* message_read_argtypes(struct message *msg)
{
	int* argtypes = (int*) msg->msg_ptr;
	int len = 0;
	while (argtypes[len++]);
	msg->msg_ptr += len*sizeof(int);
	return argtypes;
}

void message_write_args(struct message *msg, int* argtypes, void** args)
{
	int len = 0;
	while (argtypes[len++]);
	int offset = 0;
	for (int i = 0; i < len; i++)
	{
		int type = (argtypes[i] >> 16) & 0xFF;
		int arrlen = argtypes[i] & 0xFFFF;
		if (arrlen <= 0)
			arrlen = 1;

		int argsize = get_argsize(type);

		int svs = sizeof(void*);

		message_write_int(msg, (len-i)*sizeof(void*)+offset);
		if (svs == 8)
				message_write_int(msg, 0);
		offset += argsize*arrlen;
	}

	for (int i = 0; i < len; i++)
	{
		int type = (argtypes[i] >> 16) & 0xFF;
		int arrlen = argtypes[i] & 0xFFFF;
		bool is_array = true;
		if (arrlen <= 0)
		{
			arrlen = 1;
			is_array = false;
		}

		int argsize = get_argsize(type);

		if (is_array)
			message_write(msg, (char*)args[i], arrlen*argsize);
		else
		{
			switch (type)
			{
			case ARG_CHAR:
				message_write(msg, (char*)((char*)args[i]), argsize);
				break;
			case ARG_SHORT:
				message_write(msg, (char*)((short*)args[i]), argsize);
				break;
			case ARG_INT:
				message_write(msg, (char*)((int*)args[i]), argsize);
				break;
			case ARG_LONG:
				message_write(msg, (char*)((long*)args[i]), argsize);
				break;
			case ARG_DOUBLE:
				message_write(msg, (char*)((double*)args[i]), argsize);
				break;
			case ARG_FLOAT:
				message_write(msg, (char*)((float*)args[i]), argsize);
				break;
			}
		}

	}
}

void** message_read_args(struct message *msg, int* argtypes)
{
	void** args = (void**) msg->msg_ptr;
	int len = 0;
	while (argtypes[len++]);
	int offset = 0;
	for (int i = 0; i < len; i++)
	{
		int type = (argtypes[i] >> 16) & 0xFF;
		int arrlen = argtypes[i] & 0xFFFF;
		if (arrlen <= 0)
			arrlen = 1;

		args[i] = msg->msg_ptr+((long long)args[i]);
		msg->msg_ptr += sizeof(void*);

		int argsize = get_argsize(type);
		offset += argsize*arrlen;
	}
	msg->msg_ptr += offset;

	return args;
}

bool message_receive(int socket, struct message *msg) {
	char buffer[512];

	message_start(msg);
	msg->length = 0;

	int n = recv(socket, buffer, sizeof(int), 0);
	if (n == -1) {
		fprintf(stderr, "Failed to receive message length\n");
		return false;
	} else if (n == 0) {
		return false;
	}

	message_write(msg, buffer, sizeof(int));
	int len = ((int*)msg->message)[0] + sizeof(int);

	while (len > 0)
	{
		int b = (len < 512 ? len : 512);
		n = recv(socket, buffer, b, 0);
		if (n == -1)
		{
			fprintf(stderr, "Failed to receive message\n");
			return false;
		}
		else if (n == 0)
		{
			return false;
		}
		message_write(msg, buffer, n);
		len -= n;
	}

	return true;
}

bool message_send(int socket, struct message *msg) {
	int bytesLeft = msg->length;
	while (bytesLeft > 0)
	{
		int n = send(socket, msg->message+msg->length-bytesLeft, bytesLeft, 0);
		if (n == -1)
		{
			fprintf(stderr, "Failed to send message\n");
			return false;
		}
		bytesLeft -= n;
	}

	return true;
}
