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

#define MAX_WHITELIST (20)
#define FIFO_FILE ("/tmp/fifo")

char* whitelist[MAX_WHITELIST] = {"watch", "tmux", "vim"};

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

error_status_t client() {
    error_status_t ret_status = STATUS_SUCCESS;
    int fd = -1;
    bool exists = false;
    int ret = -1;
    ssize_t r_bytes = -1;
    char proc_name[NAME_MAX] = {0};

    CHECK_FUNC(create_fifo(FIFO_FILE, &fd));
    CHECK_FUNC(read_proc_name(fd, proc_name, NAME_MAX));
    CHECK_FUNC(find_in_whitelist(proc_name, &exists));
    printf("proc name %s found in whitelist: %d\n", proc_name, exists);
    CHECK_FUNC(remove_fifo(FIFO_FILE));

cleanup:
    close(fd);
    return ret_status;
}

int main() {
    client();
    return 0;
}
