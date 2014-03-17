#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

// Message Types
#define REGISTER 0
#define REGISTER_SUCCESS 1
#define REGISTER_FAILURE 2
#define LOC_REQUEST 3
#define LOC_SUCCESS 4
#define LOC_FAILURE 5
#define EXECUTE 6
#define EXECUTE_SUCCESS 7
#define EXECUTE_FAILURE 8
#define TERMINATE 9

// Error codes (+ are warnings, - are errors)
#define NONE 0
#define FAILED_SEND -1
#define FAILED_RECEIVE -2
#define FAILED_BINDER_CONNECT -3
#define FAILED_SOCKET_BIND -4
#define FAILED_SOCKET_LISTEN -5
#define INVALID_SOCKET -6
#define INVALID_FUNCTION_SIGNATURE -7
#define FAILED_FUNCTION_CALL -8
#define WARNING_DUPLICATE_FUNCTION 1


#define GRACEFUL_EXIT exit(0)

#endif
