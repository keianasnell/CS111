//
//  lab1b-server.c
//  cs111 p1b
//
//  Created by Keiana Snell on 10/14/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <mcrypt.h> 
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h> //socket(), connect(), bind(), listen()
#include <netdb.h> //gethostbyname()
#include <getopt.h>
#include <arpa/inet.h>

#define BUFSIZE 256
#define CARET_D 0x04
#define CARET_C 0x03
#define CARR_RET 0x0D
#define NEW_LINE 0x0A

const char c_d = CARET_D;
const char c_c = CARET_C;
char buffer[BUFSIZE];
pid_t pid;
int parent_to_child[2];
int child_to_parent[2];
int port = 0;
int newsockfd = 0;
char eof[2] = {'^', 'D'};
char kill_c[2] = {'^', 'C'};
char newline[2] = {'\r', '\n'};
char SHELL[] = "/bin/bash";
int crypt_flag = 0;
char key[128];
int key_len = sizeof(key);
char *IV;


MCRYPT encrypt, decrypt;

void shut_down(){
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        printf("Error in wait status: %s\n", strerror(errno));
        exit(1);
    }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\r\n", WIFSIGNALED(status), WEXITSTATUS(status));
	exit(1);
}

void get_key(char* filename){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        fprintf(stderr, "Error opening my.key %s\n", strerror(errno));
        exit(1);
    }
    size_t newlen = fread(key, sizeof(char), 99, fp);
    if (newlen == 0) {
        fprintf(stderr, "Error reading my.key: %s\n", strerror(errno));
    }
    else {
        key[newlen++] = '\0';
    }
    fclose(fp);
    
}

void init_crypt(char* key, int key_len){
    if((encrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL)) == MCRYPT_FAILED) {
        fprintf(stderr, "Error in opening mcrypt library %s\n", strerror(errno));
        exit(1);
    }
    
    IV = malloc(mcrypt_enc_get_iv_size(encrypt));
    int i;
	for (i = 0; i < mcrypt_enc_get_iv_size(encrypt); i++) {
        IV[i] = rand();
    }
    
    if(mcrypt_generic_init(encrypt, key, key_len, IV) < 0) {
        fprintf(stderr, "Error while initializing encrypt %s\n", strerror(errno));
        exit(1);
    }
    
    if((decrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL)) == MCRYPT_FAILED) {
        fprintf(stderr, "Error in opening mcrypt library %s\n", strerror(errno));
        exit(1);
    }
    if(mcrypt_generic_init(decrypt, key, key_len, IV) < 0) {
        fprintf(stderr, "Error while initializing decrypt %s\n", strerror(errno));
        exit(1);
    }
    
}

void deinit_crypt()
{
    mcrypt_generic_deinit(encrypt);
    mcrypt_module_close(encrypt);
    mcrypt_generic_deinit(decrypt);
    mcrypt_module_close(decrypt);
}

void encrypt_buf(char *buff,int crypt_len)
{
    if(mcrypt_generic(encrypt, buff, crypt_len) != 0) {
        fprintf(stderr, "Error in encryption: %s\n", strerror(errno));
        exit(1);
    }
}

void decrypt_buf(char * buff,int decrypt_len)
{    if(mdecrypt_generic(decrypt, buff, decrypt_len) != 0) {
    fprintf(stderr, "Error in decryption: %s\n", strerror(errno));
    exit(1);
}
}

void run_shell(){
    if (pipe(parent_to_child) != 0) {
        fprintf(stderr, "Failed to create pipe from server to shell.: %s\n", strerror(errno));
        exit(1);
    }
    if (pipe(child_to_parent) != 0) {
        fprintf(stderr, "Failed to create pipe from shell to server: %s\n", strerror(errno));
        exit(1);
    }
    
    pid = fork();
    
    if(pid < 0){
        fprintf(stderr, "Error forked process unsuccessful: %s\n", strerror(errno));
        exit(1);
    }
    
    else if(pid == 0){ // is child
        
        close(parent_to_child[1]); //close write end for parent
        close(child_to_parent[0]); //close read end for child
        dup2(parent_to_child[0], STDIN_FILENO); //read from parent to child
        dup2(child_to_parent[1], STDOUT_FILENO); //write output from child to parent
        dup2(child_to_parent[1], STDERR_FILENO); //write error from child to parent
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        char *shell_args[2];
        shell_args[0] = SHELL;
        shell_args[1] = NULL;

        if (execvp(shell_args[0] , shell_args) == -1) {
            fprintf(stderr, "Error in executing shell: %s\n", strerror(errno));
            exit(1);
        }
    }
    
    else { //is parent        
        close(parent_to_child[0]); //close read end for parent
        close(child_to_parent[1]); //close write end for child
                
        atexit(shut_down);
        
        struct pollfd polls[] = {
            {newsockfd, POLLIN | POLLHUP | POLLERR, 0},
            {child_to_parent[0], POLLIN | POLLHUP | POLLERR, 0}
        };
        
        
        while(1){
            int ret = poll(polls, 2, -1);
            if (ret < 0) {
                fprintf(stderr, "Error in server starting the poll: %s\n", strerror(errno));
                exit(1);
            }
            if(polls[0].revents & POLLIN) {
                int num = read(newsockfd, buffer, BUFSIZE);
                if (num < 0) {
                    fprintf(stderr, "Error in server reading socket buffer: %s\n", strerror(errno));
                    exit(1);
                }
                
                if (num == 0) {
                    fprintf(stderr, "Error: server reading empty socket buffer: %s\n", strerror(errno));
                    exit(0);
                }
                
                if(crypt_flag) {
                    decrypt_buf(buffer, BUFSIZE);
                }
                
                int i;
                for (i = 0; i < num; i++) {
                    switch(buffer[i]){
                        case CARET_D:
                            close(parent_to_child[1]);
                            shut_down();
                            break;
                        case CARET_C:
                            kill(pid, SIGINT);
				break;
                        case CARR_RET:
                        case NEW_LINE:
                            write(parent_to_child[1], &newline[1], 1); //(proper NL->CRLF mapping)
                            break;
                        default:
                            write(parent_to_child[1], buffer + i, 1);
                            continue;
                    }
                }
            }
                
            if (polls[1].revents & POLLIN) {
                char input[BUFSIZE];
                int num = read(child_to_parent[0], &input, BUFSIZE);
                if (num < 0) {
                    fprintf(stderr, "Error in server reading shell buffer: %s\n", strerror(errno));
                    exit(1);
                }
                
                if(crypt_flag) {
                    encrypt_buf(buffer, BUFSIZE);
                }
                
                int i;
                for (i = 0; i < num; i++) {
                    switch(input[i]){
                        case CARET_D:
                            write(newsockfd, &c_d, 1);
                            shut_down();
                            break;
                        case NEW_LINE:
                            write(newsockfd, newline, 2);
                            break;
                        default:
                            write(newsockfd, input + i, 1);
                            continue;
                    }
                }
            }
        
            if (polls[0].revents & (POLLERR | POLLHUP) || polls[1].revents & (POLLERR | POLLHUP)) {
                exit(0);
            }
        }
        close(child_to_parent[0]);
        exit(0);
    }

}

void connect_to_client(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    if(sockfd < 0){
        fprintf(stderr,"Error in opening socket: %s\n", strerror(errno));
        exit(1);
    }
    else {
        bzero((char*) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            fprintf(stderr, "Error on binding: %s\n", strerror(errno));
        
        listen(sockfd,5);
        
        socklen_t cliLen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cliLen);
        if (newsockfd < 0)
            fprintf(stderr, "Error on accept: %s\n", strerror(errno));
        

    }
    run_shell();
}



int main(int argc, char* argv[]){
    
    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"encrypt", required_argument, NULL, 'e'}
    };
    
    
    int port_flag = 0;
    int c;
    while((c = getopt_long(argc, argv, "p:e:", long_options, NULL)) != -1){
        switch(c){
            case 'p':
                port = atoi(optarg);
                port_flag = 1;
                break;
            case 'e':
                crypt_flag = 1;
                get_key(optarg);
                init_crypt(key, key_len);
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab1b [--port=port] [--encrypt=filename] [--log=filename] accepted\n");
                exit(1);
                break;
        }
    }
    
    if(port_flag == 1){
        connect_to_client();
        //run_shell();
    }
    else{
        fprintf(stderr, "ERROR: Unrecognized argument; only ./lab1b [--port=port] [--encrypt=filename] [--log=filename] accepted\n");
        exit(1);

    }
    
    
    exit(0);

    
    
    
}
