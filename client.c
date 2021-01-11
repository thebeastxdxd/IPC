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

#define _STR(x) #x
#define STR(x) _STR(x)

#define BUF_SIZE (4096)
#define STAT ("stat")
#define PROC_PATH "/proc"
#define PROC_FORMAT (PROC_PATH "/%s/%s")
#define STAT_FORMAT ("%d (%" STR(NAME_MAX) "[^)]) %*c %d %d")

#define IS_PID(proc_ent_name) (atoi((proc_ent_name)) != 0)

typedef struct {
    int pid;
    int ppid;
    int gid;
    char name[NAME_MAX];

} pid_info_t;

error_status_t test_connection(const char* filename) {
    error_status_t ret_status = STATUS_SUCCESS;
    struct stat buffer = {0};
    CHECK(filename != NULL);
    CHECK(stat(filename, &buffer) == 0);

cleanup:
    return ret_status;
}

static error_status_t join_proc_path(char* path, const char* pid,
                                     const char* pid_info) {
    error_status_t ret_status = STATUS_SUCCESS;
    int r_bytes = -1;

    CHECK(pid != NULL);
    CHECK(pid_info != NULL);

    r_bytes = snprintf(path, PATH_MAX, PROC_FORMAT, pid, pid_info);
    // if the bytes printed were more than the size given the output was
    // truncated
    CHECK(r_bytes > 0 && r_bytes < PATH_MAX);

cleanup:
    return ret_status;
}

error_status_t parse_stat(pid_info_t* info, const char* stat_path) {
    error_status_t ret_status = STATUS_SUCCESS;
    int fp = -1;
    int r_bytes = -1;
    char stat_buf[BUF_SIZE] = {0};

    CHECK(info != NULL);
    CHECK(stat_path != NULL);

    fp = open(stat_path, O_RDONLY);
    CHECK(fp != -1);

    r_bytes = read(fp, stat_buf, BUF_SIZE);
    CHECK(r_bytes > 0 && r_bytes < BUF_SIZE);

    CHECK(sscanf(stat_buf, STAT_FORMAT, &info->pid, info->name, &info->ppid,
                 &info->gid) == 4);

cleanup:
    close(fp);  // Best effort.
    return ret_status;
}

error_status_t parse_pid(pid_info_t* info, char* pid) {
    error_status_t ret_status = STATUS_SUCCESS;
    char stat_path[PATH_MAX] = {0};

    CHECK(info != NULL);
    CHECK(pid != NULL);

    CHECK_FUNC(join_proc_path(stat_path, pid, STAT));
    CHECK_FUNC(parse_stat(info, stat_path));

cleanup:
    return ret_status;
}

error_status_t check_pid_name(pid_info_t* info, const char* substr,
                              bool* contains) {
    error_status_t ret_status = STATUS_SUCCESS;
    char* sub = NULL;

    CHECK(info != NULL);
    CHECK(substr != NULL);

    sub = strstr(info->name, substr);
    if (sub != NULL)
        *contains = true;
    else
        *contains = false;

cleanup:
    return ret_status;
}

static error_status_t check_process_name(const char* substr, int* ret_pid) {
    error_status_t ret_status = STATUS_SUCCESS;
    struct dirent* proc_ent = NULL;
    DIR* proc_dir = NULL;
    bool exists = false;
    int temp_pid = -1;

    CHECK(substr != NULL);
    CHECK(ret_pid != NULL);

    proc_dir = opendir(PROC_PATH);
    CHECK(proc_dir != NULL);

    proc_ent = readdir(proc_dir);
    CHECK(proc_ent != NULL);

    while (proc_ent != NULL) {
        // this struct is inside the loop to clear it on each iteration
        pid_info_t info = {0};
        if (IS_PID(proc_ent->d_name)) {
            CHECK_FUNC(parse_pid(&info, proc_ent->d_name));
            CHECK_FUNC(check_pid_name(&info, substr, &exists));
            if (exists) {
                temp_pid = info.pid;
                break;
            }
        }
        proc_ent = readdir(proc_dir);
    }
    *ret_pid = temp_pid;

cleanup:
    return ret_status;
}

error_status_t client_pipe() {
    error_status_t ret_status = STATUS_SUCCESS;
    int fd = -1;
    int pid = 0;
    ssize_t r_bytes = -1;
    char proc_name[NAME_MAX] = {0};
    int name_len = 0;

    /* Create the FIFO if it does not exist */
    mkfifo(FIFO_FILE, S_IFIFO | 0640);
    fd = open(FIFO_FILE, O_RDWR);
    CHECK_STR(fd != -1, "Failed to open FIFO file");
    r_bytes = read(fd, proc_name, NAME_MAX);
    CHECK(r_bytes > 0);
    name_len = strlen(proc_name);
    proc_name[name_len - 1] = '\0';
    printf("got proc name: %s\n", proc_name);
    CHECK_FUNC(check_process_name(proc_name, &pid));
    printf("found pid: %d, for proc name: %s\n", pid, proc_name);

cleanup:
    return ret_status;
}

error_status_t client() {
    error_status_t ret_status = STATUS_SUCCESS;
    int client_fd = -1;
    int read_bytes = 0;
    struct sockaddr_un addr = {0};
    message_t msg = {0};

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK(client_fd != -1);
    addr.sun_family = AF_UNIX;
    // TODO: check memcpy return?
    memcpy(addr.sun_path, UNIX_SOCK_PATH, sizeof(addr.sun_path) - 1);

    CHECK(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) != -1);

cleanup:
    close(client_fd);  // Best effort.
    return ret_status;
}

int main() {
    client();
    return 0;
}
