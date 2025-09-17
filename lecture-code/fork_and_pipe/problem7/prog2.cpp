#include <cstdio>
#include <unistd.h>
int main() {
  char *argv[2];
  argv[0] = (char *) "prog1.exe";
  argv[1] = nullptr;
  printf("PID %d running prog2\n", getpid());
  execv("./prog1.exe", argv);
  printf("PID %d exiting from prog2\n", getpid());
}