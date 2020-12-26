#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "error.h"

error_status_t server() {
    error_status_t ret_status = STATUS_SUCCESS;
	int fd = -1;
    char* sub_str = "watch";
    message_check_str_t msg = {0};
     
	/* Create the FIFO if it does not exist */
	mkfifo(FIFO_FILE, S_IFIFO|0640);
	fd = open(FIFO_FILE, O_RDWR);
    CHECK_STR(fd != -1,"Failed to open FIFO file");
    sleep(5);
	while(1) {
        msg.hdr.type = MSG_CHECK_STR;
        strncpy(msg.substr, sub_str, strlen(sub_str));
        CHECK_FUNC(send_message(fd, MSG_CHECK_STR, (message_t*)&msg));
        printf("sent message type: %d\n", msg.hdr.type);
        sleep(2);
        CHECK_FUNC(recv_message(fd, (message_t*)&msg));
        printf("got message: %d, str: %s, pid: %d\n", msg.hdr.type, msg.substr, msg.pid);
	}

cleanup:
    return ret_status;
}

int main() { 
    server();
	return 0;
}
