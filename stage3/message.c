#include <unistd.h>

#include "message.h"
#include "error.h"

error_status_t send_message(int fd, message_t* msg) { 
    error_status_t ret_status = STATUS_SUCCESS;
    ssize_t w_bytes = -1;
    size_t type_size = 0;

    CHECK(msg != NULL);
    w_bytes = write(fd, (void*)msg, sizeof(message_t));
    CHECK(w_bytes > 0);

cleanup:
    return ret_status;
}

error_status_t recv_message(int fd, message_t* msg) {
    error_status_t ret_status = STATUS_SUCCESS;
    ssize_t r_bytes = -1;

    r_bytes = read(fd, (void*)msg, sizeof(message_t)); 
    CHECK(r_bytes > 0);

cleanup:
    return ret_status;
}

