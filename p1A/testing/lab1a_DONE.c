#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>

#define BUFSIZE 256
#define CARET_D 0x04
#define CARET_C 0x03
#define CARR_RET 0x0D
#define NEW_LINE 0x0A

char buffer[BUFSIZE];
struct termios terminal_info;
struct termios orig_terminal;
struct termios copy_terminal;
char newline[2] = {'\r', '\n'};
char* program = NULL;
pid_t pid;
int parent_to_child[2];
int child_to_parent[2];
char eof[2] = {'^', 'D'};
char kill_c[2] = {'^', 'C'};



void reset_terminal(){
    tcsetattr(0, TCSANOW, &orig_terminal);
}

void set_no_echo_terminal(){
    tcgetattr(0, &copy_terminal);
    
    copy_terminal.c_iflag = ISTRIP; /* only lower 7 bits    */
    copy_terminal.c_oflag = 0;      /* no processing        */
    copy_terminal.c_lflag = 0;      /* no processing        */
    
    if (tcsetattr(0, TCSANOW, &copy_terminal) < 0){
        fprintf(stderr, "Error: Cannot initialize terminal.\n");
        exit(1);
    }
}


void run_without_shell(){
    int t_rue = 1;
    char buf[BUFSIZE];
    while(t_rue){
        ssize_t count = read(0, &buf, BUFSIZE);
        if(count < 0){
            fprintf(stderr, "Error: cannot correctly read buffer.\n");
            exit(1);
        }
        int i;
        for(i = 0; i < count; i++){
            switch(buf[i]){
                case CARET_D:
                    write(STDOUT_FILENO, "d\n", 2);
                    exit(0);
                    break;
                    
                case CARET_C:
                    write(STDOUT_FILENO, "c\n", 2);
                    exit(1);
                    break;
                    
                case CARR_RET:
                case NEW_LINE:
                    write(STDOUT_FILENO, &newline, 2);
                    continue;
                    
                default:
                    write(STDOUT_FILENO, &buf[i], 1);
                    continue;
                }
            }
        }
}


void reap_child() {
    int exit_status;
    waitpid(pid, &exit_status, 0);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", exit_status&0x7f, (exit_status&0xff00) >> 8);
    exit(0);
   // fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(exit_status), WEXITSTATUS(exit_status));
}


void run_shell(char* program){
    if (pipe(parent_to_child) != 0) {
        fprintf(stderr, "Failed to create pipe from terminal to shell.\n");
        exit(1);
    }
    if (pipe(child_to_parent) != 0) {
        fprintf(stderr, "Failed to create pipe from shell to terminal.\n");
        exit(1);
    }

    pid = fork();
    
    if(pid < 0){
        fprintf(stderr, "ERROR: %s Forked process unsuccessful.\n", strerror(errno));
        exit(1);
    }
    
    else if(pid == 0){ // is child
        close(parent_to_child[1]); //close write end for parent
        close(child_to_parent[0]); //close read end for child
        
        close(child_to_parent[STDIN_FILENO]);
        dup2(parent_to_child[0], STDIN_FILENO); //read from parent to child
        dup2(child_to_parent[1], STDOUT_FILENO); //write output from child to parent
        dup2(child_to_parent[1], STDERR_FILENO); //write error from child to parent
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        
        char* const args[] = {program, NULL};
        
        if (execvp(program, args) == -1) {
            fprintf(stderr, "Failed to exec a shell.\n");
            exit(1);
        }
    }
    else { //is parent
        close(parent_to_child[0]); //close read end for parent
        close(child_to_parent[1]); //close write end for child
        
        struct pollfd polls[] = {
            {STDIN_FILENO, POLLIN, 0},
            {child_to_parent[0], POLLIN, 0}
        };
        
    
    while(1){
        int ret = poll(polls, 2, -1);
        if (ret < 0) {
            fprintf(stderr, "Failed to start the poll.\n");
        }
        if(ret > 0){
            if (polls[0].revents & POLLIN) {
                int num = read(STDIN_FILENO, buffer, BUFSIZE);
                if (num < 0) {
                    write(STDOUT_FILENO, "Error: cannot properly read buffer", 5);
                    exit(1);
                }
                int i;
                for (i = 0; i < num; i++) {
                    switch(buffer[i]){
                        case CARET_D:
                            write(STDOUT_FILENO, eof, 2);
                            close(parent_to_child[1]);
                            //may have to close ALL pipes here
                            reap_child();
                            break;
                        case CARET_C:
                            write(STDOUT_FILENO, kill_c, 2);
                            kill(pid, SIGINT);
                            break;
                        case CARR_RET:
                        case NEW_LINE:
                            write(STDOUT_FILENO, &newline, 2);
                            write(parent_to_child[1], newline + 1, 1);
                            break;
                        default:
                            write(STDOUT_FILENO, buffer + i, 1);
                            write(parent_to_child[1], buffer + i, 1);
                            continue;
                    }
                }
            }
        }
    
    if (polls[1].revents & POLLIN) {
        char input[BUFSIZE];
        int num = read(child_to_parent[0], &input, BUFSIZE);
        if (num < 0) {
            write(STDOUT_FILENO, "Error: cannot properly read buffer", 5);
            exit(1);
        }
       // int count = 0;
        int i;
        for (i = 0; i < num; i++) {
            switch(input[i]){
                case CARET_D:
                    write(STDOUT_FILENO, eof, 2);
                    exit(0);
                    break;
                case NEW_LINE:
                    write(STDOUT_FILENO, newline, 2);
                    break;
                default:
                    write(STDOUT_FILENO, input + i, 1);
            }
        }
//        write(STDOUT_FILENO, (input + j), count);
    }
    
    
    if (polls[0].revents & (POLLERR | POLLHUP) || polls[1].revents & (POLLERR | POLLHUP)) {
        exit(0);
    }
}
    
    //    close(parent_to_child[1]);
        close(child_to_parent[0]);
        reap_child();
        exit(0);
    }
}


int main(int argc, char* argv[]){
    tcgetattr(0,&orig_terminal); //save copy of original terminal
    set_no_echo_terminal();
    atexit(reset_terminal); //when program exit, reset normal settings for terminal
    atexit(reap_child);
    
    static struct option long_options[] = {
        {"shell", required_argument, NULL, 's'}
    };


    int shell_flag = 0;
    int c;
    while((c = getopt_long(argc,argv,"s", long_options, NULL)) != -1){
        switch(c){
            case 's':
                program = optarg;
                shell_flag = 1;
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab1a [--shell=program] accepted\n");
                exit(1);
                break;
        }
    }
    
    if(shell_flag == 1){
        run_shell(program);
    }
    else{
        run_without_shell();
    }
    

    exit(0);
}
