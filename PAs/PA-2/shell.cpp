#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

#include <fcntl.h>

#include <pwd.h>
#include <ctime>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    for (;;) {
        // need date/time, username, and absolute path to current dir
        
        // username
        struct passwd *pw = getpwuid(getuid());
        string username;

        if (pw != nullptr) username = pw->pw_name;
        else username = "user";

        // month
        time_t t = time(nullptr);
        tm* local_time = localtime(&t);
        char month_buf[4];  // 3-letter month + null
        strftime(month_buf, sizeof(month_buf), "%b", local_time);
        string month(month_buf);

        cout << YELLOW << username << " " << month << " Shell$ " << NC;
        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        if (tknr.commands.size() == 1 && !tknr.commands[0]->args.empty()) {
            Command* command = tknr.commands[0];

            if (command->args[0] == "cd") {
                static string prev_dir;
                char cwd_buf[4096];      // fixed-size stack buffer
                getcwd(cwd_buf, sizeof(cwd_buf));
                string curr_dir = cwd_buf;

                const char* target = nullptr;

                if (command->args.size() == 1) {
                    // cd with no args returns to home dir
                    target = getenv("HOME");
                } else if (command->args[1] == "-") {
                    // cd - returns to prev dir
                    target = prev_dir.c_str();
                } else {
                    target = command->args[1].c_str();
                }

                if (chdir(target) != 0) {
                    perror("cd");
                } else {
                    prev_dir = curr_dir;
                }

                continue; // skip forking
            }
        }

        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }

        int num_commands = tknr.commands.size();
        vector<int> pipes(2*(num_commands - 1));

        // create all pipes
        for (int i = 0; i < num_commands - 1; i++) {
            if (pipe(&pipes[2 * i]) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        vector<pid_t> pids(num_commands);
        
        for (int i = 0; i < num_commands; i++) {
            Command* command = tknr.commands[i];
            
            // fork to create child
            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            
            if (pid == 0) {  // if child, exec to run command
                // If not first command
                if (i > 0) {
                    dup2(pipes[2 * (i - 1)], STDIN_FILENO);
                } else if (command->hasInput()) {
                    int fd = open(command->in_file.c_str(), O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // If not last command
                if (i < num_commands-1) {
                    dup2(pipes[i*2 + 1], STDOUT_FILENO);
                } else if (command->hasOutput()) {
                    int fd = open(command->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                // Close all pipes
                for (int j = 0; j < 2 * (num_commands - 1); j++) {
                    close(pipes[j]);
                }

            
                // build argument array
                vector<char*> args;
                for (long unsigned int i = 0; i < command->args.size(); i++) {
                    const char* arg = command->args[i].c_str();
                    args.push_back((char*) arg);
                }
                args.push_back(nullptr);
                
                if (execvp(args[0], args.data()) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else {  // if parent, wait for child to finish
                pids[i] = pid;
            }
        }

        // Close parent pipes
        for (int j = 0; j < 2 * (num_commands - 1); j++) {
            close(pipes[j]);
        }

        // Wait for all children
        for (int i = 0; i < num_commands; i++) {
            int status = 0;
            waitpid(pids[i], &status, 0);
        }
    }
}
