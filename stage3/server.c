#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "error.h"

#define MAX_WHITELIST (20)
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
char* whitelist[MAX_WHITELIST] = {"watch", "tmux", "vim", "ps", "pasten"};

error_status_t list_to_buff(char** list, int list_len, char* buf, int buf_len) {
    error_status_t ret_status = STATUS_SUCCESS;
    int i = 0;
    int buf_index = 0;
    size_t proc_len = 0;

    CHECK(list != NULL);
    CHECK(buf != NULL);
    
    while (list[i] != NULL && i < list_len && buf_index < buf_len) { 
        proc_len = strlen(list[i]) + 1;
        CHECK((buf_index + proc_len) < buf_len);
        strncpy((buf + buf_index), list[i], proc_len);
        buf_index += proc_len;
        i++;
    }

cleanup:
    return ret_status;
}

error_status_t handle_whitelist_req(int fd, message_t* msg) {
    error_status_t ret_status = STATUS_SUCCESS;
    message_get_whitelist_t* tmp_msg = (message_get_whitelist_t*)msg;

    tmp_msg->hdr.type = MSG_GET_WHITELIST_ANS;
    tmp_msg->len = ARRAY_LEN(whitelist);
    CHECK_FUNC(list_to_buff(whitelist, tmp_msg->len, tmp_msg->list, sizeof(tmp_msg->list)));
    CHECK_FUNC(send_message(fd, msg));

cleanup:
    return ret_status;
}

error_status_t handle_message(int fd, message_t* msg) {
    error_status_t ret_status = STATUS_SUCCESS;
    
    CHECK(msg != NULL);

    switch(msg->hdr.type) {
        case MSG_GET_WHITELIST:
            CHECK_FUNC(handle_whitelist_req(fd, msg));
            break;
        default:
            printf("got message type: %d, message %s", msg->hdr.type, msg->buf);
    }

cleanup:
    return ret_status;
}

error_status_t handle_client(int fd) {
    error_status_t ret_status = STATUS_SUCCESS;
    message_t msg = {0};
    ssize_t read_bytes = 0;
    printf("handling client\n"); 
    CHECK_FUNC(recv_message(fd, &msg));
    while (!IS_END_MSG(msg)) {
        handle_message(fd, &msg);
        CHECK_FUNC(recv_message(fd, &msg));
    }

cleanup:
    return ret_status;
}

error_status_t server() {
    error_status_t ret_status = STATUS_SUCCESS;
    int server_fd = -1;
    int client_fd = -1;
    struct sockaddr_un addr = {0};

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK(server_fd != -1);
    addr.sun_family = AF_UNIX;
    // TODO: check memcpy return?
    *addr.sun_path = '\0';
    strncpy(addr.sun_path, UNIX_SOCK_PATH, sizeof(addr.sun_path) - 2);


    CHECK_STR(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) != -1, "Failed to bind");
    CHECK_STR(listen(server_fd, 1) != -1, "Failed to listen");

    while (1) {
        printf("Waiting for client\n");
        client_fd = accept(server_fd, NULL, NULL);
        CHECK(client_fd != -1);
        handle_client(client_fd);
        close(client_fd);
    }

cleanup:
    close(server_fd); // Best effort.
    return ret_status;
}

int main() { 
    int ret = 0;
    ret = server();
	return ret;
}
