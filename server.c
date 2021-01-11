#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "error.h"

error_status_t server() {
    error_status_t ret_status = STATUS_SUCCESS;
    int server_fd = -1;
    int client_fd = -1;
    int read_bytes = 0;
    struct sockaddr_un addr = {0};
    message_t msg = {0};

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK(server_fd != -1);
    addr.sun_family = AF_UNIX;
    // TODO: check memcpy return?
    memcpy(addr.sun_path, UNIX_SOCK_PATH, sizeof(addr.sun_path)-1);

    CHECK(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr) != -1));
    CHECK(listen(server_fd, 5) != -1);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        CHECK(client_fd != -1);
        read_bytes = recv(client_fd, (void*)&msg, sizeof(msg), 0);
        CHECK(read_bytes == sizeof(msg));
        while (read_bytes != 0) {
            printf("got message type: %d, message %s", msg.hdr.type, msg.buf);
            read_bytes = recv(client_fd, (void*)&msg, sizeof(msg), 0);
            CHECK(read_bytes == sizeof(msg));
        }
        close(client_fd);
    }

cleanup:
    close(client_fd); // Best effort.
    close(server_fd); // Best effort.
    return ret_status;
}



int main() { 
    int ret = 0;
    ret = server();
	return ret;
}
