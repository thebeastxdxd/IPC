#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "message.h"
#include "error.h"

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

static error_status_t join_proc_path(char* path, const char* pid, const char* pid_info) {
    error_status_t ret_status = STATUS_SUCCESS;
    int r_bytes = -1;

    CHECK(pid != NULL);
    CHECK(pid_info != NULL);

    r_bytes = snprintf(path, PATH_MAX, PROC_FORMAT, pid, pid_info);
    // if the bytes printed were more than the size given the output was truncated
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
   
    CHECK(sscanf(stat_buf, STAT_FORMAT, &info->pid, info->name, &info->ppid, &info->gid) == 4);

cleanup:
    close(fp); // Best effort.
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

error_status_t check_pid_name(pid_info_t* info, const char* substr, bool* contains) {
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

error_status_t check_process_name(const char* substr, int* ret_pid) {
    error_status_t ret_status = STATUS_SUCCESS;
    struct dirent* proc_ent = NULL;
    DIR* proc_dir = NULL;
    bool exists = false;

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
                *ret_pid = info.pid; 
                break;
            }
        }
    }
    *ret_pid = -1;

cleanup:
    return ret_status;
}

error_status_t client() {
    error_status_t ret_status = STATUS_SUCCESS;
    int fd = -1;
    int pid = 0;
    message_t msg = {0};
    
    CHECK_FUNC(test_connection(FIFO_FILE));
    fd = open(FIFO_FILE, O_RDWR);
    CHECK(fd != -1);
    while(1) {
        CHECK_FUNC(recv_message(fd, &msg));
        printf("got message type: %d\n", msg.hdr.type);
        if (msg.hdr.type == MSG_CHECK_STR) {
            // this is ugly?
            message_check_str_t* msg_check_str = (message_check_str_t*)&msg;
            //CHECK_FUNC(check_process_name(msg_check_str->substr, &pid));
            printf("found pid: %d\n", pid);
            msg_check_str->hdr.type = MSG_CHECK_STR_ANS;
            msg_check_str->pid = pid;
            send_message(fd, MSG_CHECK_STR_ANS, &msg); 
            printf("send message type: %d\n", msg_check_str->hdr.type);
            sleep(2);
        } else {
            printf("got unknown message type: %d\n", msg.hdr.type);
        }
    }

cleanup:
    return ret_status;
}

int main() {
    client();
    return 0;
}
