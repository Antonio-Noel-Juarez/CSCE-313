#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h>
#include <iostream>

int main() {
    int count = 0;
    pid_t main_process_pid = getpid();
    for (int i = 0; i < 3; i++) {
        if (fork() == 0) count++;
        else wait(NULL);
    }
    if (getpid() == main_process_pid) {
        printf("%d\n", count);
    }

    return 0;
}