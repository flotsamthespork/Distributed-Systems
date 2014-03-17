
#include "rpc.h"
#include "util.h"

int
get_argsize(int type)
{
	int argsize = 0;
	switch (type)
	{
	case ARG_CHAR:
		argsize = sizeof(char);
		break;
	case ARG_SHORT:
		argsize = sizeof(short);
		break;
	case ARG_INT:
		argsize = sizeof(int);
		break;
	case ARG_LONG:
		argsize = sizeof(long);
		break;
	case ARG_DOUBLE:
		argsize = sizeof(double);
		break;
	case ARG_FLOAT:
		argsize = sizeof(float);
		break;
	}
	return argsize;
}
