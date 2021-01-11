#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "error.h"
#include "message.h"

#define MAX_WHITELIST (20)
#define BUF_SIZE (1024)

char* whitelist[MAX_WHITELIST] = {"watch", "tmux", "vim", "ps"};

error_status_t create_fifo(const char* fifo_name, int* fd) {
    error_status_t ret_status = STATUS_SUCCESS;
    int ret = -1;
    int tmp_fd = -1;

    CHECK(fd != NULL);

    // Create the FIFO if it does not exist
    ret = mkfifo(fifo_name, S_IFIFO | 0640);
    CHECK(ret != -1);

    tmp_fd = open(fifo_name, O_RDONLY);
    CHECK_STR(tmp_fd != -1, "Failed to open FIFO file");

    *fd = tmp_fd;

cleanup:
    return ret_status;
}

error_status_t remove_fifo(const char* fifo_name) {
    error_status_t ret_status = STATUS_SUCCESS;

    CHECK_STR(unlink(fifo_name) != -1, "unable to remote FIFO file\n");

cleanup:
    return ret_status;
}

error_status_t read_proc_name(int fd, char* proc_name, int max_len) {
    error_status_t ret_status = STATUS_SUCCESS;
    ssize_t r_bytes = -1;
    int name_len = 0;

    r_bytes = read(fd, proc_name, max_len);
    CHECK(r_bytes > 0);
    name_len = strlen(proc_name);
    proc_name[name_len - 1] = '\0';
    printf("got proc name: %s\n", proc_name);

cleanup:
    return ret_status;
}

error_status_t find_in_whitelist(const char* proc_name, bool* exists) {
    error_status_t ret_status = STATUS_SUCCESS;
    int i = 0;
    int ret = 0;
    CHECK(proc_name != NULL);
    CHECK(exists != NULL);

    while (i < MAX_WHITELIST && whitelist[i] != NULL) {
        ret = strncmp(whitelist[i], proc_name, NAME_MAX);
        if (ret == 0) {
            *exists = true;
            break;
        }
        i++;
    }

cleanup:
    return ret_status;
}

error_status_t check_whitelist(char* proc_name, bool* exists) {
    error_status_t ret_status = STATUS_SUCCESS;
    int fd = -1;
    int ret = -1;
    ssize_t r_bytes = -1;

    CHECK(exists != NULL);
    CHECK(proc_name != NULL);

    CHECK_FUNC(create_fifo(FIFO_FILE, &fd));
    CHECK_FUNC(read_proc_name(fd, proc_name, NAME_MAX));
    CHECK_FUNC(find_in_whitelist(proc_name, exists));
    CHECK_FUNC(remove_fifo(FIFO_FILE));

cleanup:
    close(fd);
    return ret_status;
}

error_status_t read_line(int fd, char* line, int max_len, int* len) {
    error_status_t ret_status = STATUS_SUCCESS;
    int read_bytes = 0;
    char tmp = '0';
    int i = 0;

    CHECK(line != NULL);
    CHECK(max_len > 0);
    CHECK(len != NULL);

    read_bytes = read(fd, &tmp, 1);
    CHECK(read_bytes != -1);
    while (read_bytes != 0 && tmp != '\n' && i < max_len) {
        line[i++] = tmp;
        read_bytes = read(fd, &tmp, 1);
        CHECK(read_bytes != -1);
    }
    CHECK_STR(i < max_len, "name too long\n");
    // len doesnt include null byte
    line[i] = '\0';
    *len = i;

cleanup:
    return ret_status;
}

error_status_t parse_ps_output(int fd, const char* proc_name) {
    error_status_t ret_status = STATUS_SUCCESS;
    char buf[NAME_MAX] = {0};
    char* sub = NULL;
    int line_len = 0;

    CHECK_FUNC(read_line(fd, buf, sizeof(buf), &line_len));
    while (line_len != 0) {
        printf("buf %s\n", buf);
        sub = strstr(buf, proc_name);
        if (sub != NULL) {
            printf("found proc_name\n");
            break;
        }
        line_len = 0;
        CHECK_FUNC(read_line(fd, buf, sizeof(buf), &line_len));
    }

cleanup:
    return ret_status;
}

error_status_t check_process_name(const char* proc_name) {
    error_status_t ret_status = STATUS_SUCCESS;
    int pipefd[2] = {0};
    int read_bytes = 0;
    CHECK(pipe(pipefd) != -1);

    // child
    if (fork() == 0) {
        CHECK(close(pipefd[0]) != -1);
        CHECK(dup2(pipefd[1], STDOUT_FILENO) != -1);  // send stdout to the pipe
        CHECK(dup2(pipefd[1], STDERR_FILENO) != -1);  // send stderr to the pipe

        CHECK(close(pipefd[1]) != -1);

        execl("/bin/ps", "ps", "-ade", "--no-headers", "-o", "comm", NULL);
    } else {
        CHECK(close(pipefd[1]) != -1);
        printf("searching for proc\n");
        CHECK_FUNC(parse_ps_output(pipefd[0], proc_name));
    }

cleanup:
    close(pipefd[0]);  // Best effort.
    return ret_status;
}

error_status_t get_whitelist(int fd) {
    error_status_t ret_status = STATUS_SUCCESS;
    message_get_whitelist_t msg = {0};

    msg.hdr.type = MSG_GET_WHITELIST;
    CHECK_FUNC(send_message(fd, (message_t*)&msg));
    CHECK_FUNC(recv_message(fd, (message_t*)&msg));
    //check recved message
    printf("recved message type %d, len %d, first val %s\n", msg.hdr.type, msg.len, msg.list);
    
    msg.hdr.type = MSG_END;
    CHECK_FUNC(send_message(fd, (message_t*)&msg));
cleanup:
    return ret_status;
}

error_status_t client() {
    error_status_t ret_status = STATUS_SUCCESS;
    char proc_name[NAME_MAX] = {0};
    bool whitelisted = false;
    int client_fd = -1;
    struct sockaddr_un addr = {0};

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK(client_fd != -1);

    addr.sun_family = AF_UNIX;
    // TODO: check memcpy return?
    *addr.sun_path = '\0';
    strncpy(addr.sun_path, UNIX_SOCK_PATH, sizeof(addr.sun_path) - 1);

    CHECK(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) != -1);
    CHECK_FUNC(get_whitelist(client_fd));
    CHECK_FUNC(check_whitelist(proc_name, &whitelisted));
    if (whitelisted) {
        printf("proc name %s found in whitelist: %d\n", proc_name, whitelisted);
        CHECK_FUNC(check_process_name(proc_name));
    } else {
        printf("proc name is not white listed\n");
    }

cleanup:
    close(client_fd); // Best effort
    return ret_status;
}

int main() {
    client();
    return 0;
}
