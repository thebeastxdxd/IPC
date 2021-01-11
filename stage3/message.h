#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <linux/limits.h>
#include "error.h"

#define FIFO_FILE ("/tmp/fifo")
#define UNIX_SOCK_PATH ("\0sock_name")
#define MAX_MSG_DATA (4096)


typedef enum {
    MSG_GENERIC,
    MSG_GENERIC_ANS,
    MSG_CHECK_STR,
    MSG_CHECK_STR_ANS,
    MSG_GET_WHITELIST,
    MSG_GET_WHITELIST_ANS,
    MSG_END
} message_type_t;

typedef struct {
    message_type_t type;

} message_hdr_t;

typedef struct {
    message_hdr_t hdr;
    char buf[MAX_MSG_DATA];

} message_t;

typedef struct {
    message_hdr_t hdr;
    char substr[NAME_MAX];
    int pid;
    char pad[MAX_MSG_DATA - NAME_MAX - sizeof(int)];
} message_check_str_t; 

typedef struct {
    message_hdr_t hdr;
    int len;
    char list[MAX_MSG_DATA - sizeof(int)];
} message_get_whitelist_t;

#define IS_END_MSG(m) (m.hdr.type == MSG_END)

error_status_t send_message(int fd, message_t* msg);
error_status_t recv_message(int fd, message_t* msg);

#endif // __MESSAGE_H__
