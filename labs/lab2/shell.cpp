/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h>
#include <iostream>

using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // Save original input/output
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    // TODO: add functionality
    // Create pipe
    int fds[2];
    if (pipe(fds) == -1) {
	cerr << "pipe failed\n";
    }
    // Create child to run first command
    // In child, redirect output to write end of pipe
    // Close the read end of the pipe on the child side.
    // In child, execute the command
    pid_t pid = fork();
    if (pid == -1) {
	cerr << "fork failed\n";
	return 1;
    }

    if (pid == 0) {
	// Child Process

	close(fds[0]); 			// Close unused read
	dup2(fds[1], STDOUT_FILENO); 	// Write process to pipe write
	execvp(cmd1[0], cmd1); 		// Execute ls command

	// If execvp returns, something went wrong
	cerr << "Exec failed\n";
	return 1;
    }

    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.
    
    pid = fork();
    if (pid == -1) {
	cerr << "fork failed\n";
	return 1;
    }

    if (pid == 0) {
	// Child process
	
	close(fds[1]);			// Close unused write
	dup2(fds[0], STDIN_FILENO); 	// Redirect read of pipe to input
	execvp(cmd2[0], cmd2);		// Execute the tr command

	// If execvp returns, something went wrong
	cerr << "Exec failed\n";
	return 1;
    }

    // Reset the input and output file descriptors of the parent.
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);

}
