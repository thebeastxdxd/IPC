#include <unistd.h>

#include "message.h"
#include "error.h"

size_t get_msg_type_size(message_type_t type) {
    
    if (type == MSG_CHECK_STR || type == MSG_CHECK_STR_ANS) {
        return sizeof(message_check_str_t);
    } else {
        return sizeof(message_t);
    }
}

error_status_t send_message(int fd, message_type_t type, message_t* msg) { 
    error_status_t ret_status = STATUS_SUCCESS;
    ssize_t w_bytes = -1;
    size_t type_size = 0;

    CHECK(msg != NULL);
    type_size = get_msg_type_size(type);
    w_bytes = write(fd, msg, type_size);
    CHECK(w_bytes > 0);

cleanup:
    return ret_status;
}

error_status_t recv_message(int fd, message_t* msg) {
    error_status_t ret_status = STATUS_SUCCESS;
    ssize_t r_bytes = -1;

    r_bytes = read(fd, msg, sizeof(message_t)); 
    CHECK(r_bytes > 0);

cleanup:
    return ret_status;
}

