//
//  lab1b-client.c
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
#define BUFSIZE 256
#define CARET_D 0x04
#define CARET_C 0x03
#define CARR_RET 0x0D
#define NEW_LINE 0x0A

const char c_d = CARET_D;
const char c_c = CARET_C;
const char lr = NEW_LINE;
char eof[2] = {'^', 'D'};
char kill_c[2] = {'^', 'C'};
char newline[2] = {'\r', '\n'};
char buffer[BUFSIZE];
int port = 0;
int crypt_flag = 0;
struct termios orig_terminal;
struct termios copy_terminal;
char key[128];
int key_len = sizeof(key);
int sockfd;
int log_flag = 0;
FILE *fptr;


MCRYPT encrypt, decrypt;


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
        fprintf(stderr, "Error in opening mcrypt: %s\n", strerror(errno));
        exit(1);
    }
    if(mcrypt_generic_init(encrypt, key, key_len, NULL) < 0) {
        fprintf(stderr, "Error while initializing encrypt: %s\n", strerror(errno));
        exit(1);
    }
    
    if((decrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL)) == MCRYPT_FAILED) {
        fprintf(stderr, "Error in opening mcrypt: %s\n", strerror(errno));
        exit(1);
    }
    if(mcrypt_generic_init(decrypt, key, key_len, NULL) < 0) {
        fprintf(stderr, "Error while initializing decrypt: %s\n", strerror(errno));
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
{    if(mdecrypt_generic(encrypt, buff, decrypt_len) != 0) {
        fprintf(stderr, "Error in decryption: %s\n", strerror(errno));
        exit(1);
    }
}


void reset_terminal(){
    if(fclose(fptr) != 0){
        fprintf(stderr, "Error in closing file %s\n", strerror(errno));
    }
    tcsetattr(0, TCSANOW, &orig_terminal);
    
    if(crypt_flag){
        deinit_crypt();
    }
}

void set_no_echo_terminal(){
    tcgetattr(0, &copy_terminal);
    
    copy_terminal.c_iflag = ISTRIP; /* only lower 7 bits    */
    copy_terminal.c_oflag = 0;      /* no processing        */
    copy_terminal.c_lflag = 0;      /* no processing        */
    
    if (tcsetattr(0, TCSANOW, &copy_terminal) < 0){
        fprintf(stderr, "Error in initializing terminal: %s\n", strerror(errno));
        exit(1);
    }
}

void read_and_write(){
    struct pollfd polls[] = {
        {STDIN_FILENO, POLLIN| POLLHUP | POLLERR, 0},
        {sockfd, POLLIN | POLLHUP | POLLERR, 0}
    };
    
    while(1){
        int ret = poll(polls, 2, -1);
        if (ret < 0) {
            fprintf(stderr, "Error in client starting poll: %s\n", strerror(errno));
        }
        
        if (polls[0].revents & POLLIN) {
            int num = read(STDIN_FILENO, buffer, BUFSIZE);
            if (num < 0) {
                fprintf(stderr, "Error in client reading stdin buffer: %s\n", strerror(errno));
                exit(1);
            }
            if (num == 0) {
                fprintf(stderr, "Error: client reading empty stdin buffer: %s\n", strerror(errno));
                exit(0);
            }
            
            if(crypt_flag) {
                encrypt_buf(buffer, BUFSIZE);
            }
            
            int i;
            for (i = 0; i < num; i++) {
                switch(buffer[i]){
                    case CARET_D:
                       // if(log_flag) fprintf(fptr, eof, 2);
                        write(STDOUT_FILENO, eof, 2);
                        write(sockfd, &c_d, 1);
                        break;
                    case CARET_C:
                      //  if(log_flag) fprintf(fptr, kill_c, 2);
                        write(STDOUT_FILENO, kill_c, 2);
                        write(sockfd, &c_c, 1);
                        break;
                    case CARR_RET:
                    case NEW_LINE:
                     //   if(log_flag) fprintf(fptr, newline, 2);
                        write(STDOUT_FILENO, &newline, 2);
                        write(sockfd, newline + 1, 1);
                        break;
                    default:
                     //   if(log_flag) fprintf(fptr, buffer + i, 1);
                        write(STDOUT_FILENO, buffer + i, 1);
                        write(sockfd, buffer + i, 1);
                        continue;
                }
//                if(log_flag){
//                    char sent[] = "SENT ";
//                    char num_bytes[20];
//                    char bytes[] = " bytes: ";
//                    sprintf(num_bytes, "%d", num);
//                    fprintf(fptr, sent, sizeof(sent));
//                    fprintf(fptr, num_bytes, strlen(num_bytes));
//                    fprintf(fptr, bytes, sizeof(bytes));
//                    fprintf(fptr, buffer, num);
//                    fprintf(fptr, &lr, 1);
//                }
            }
            fprintf(fptr, "SENT %d bytes: %s \n", num, buffer);
        }

        if (polls[1].revents & POLLIN) {

            char input[BUFSIZE];
            int num = read(sockfd, &input, BUFSIZE);
            if (num < 0) {
                fprintf(stderr, "Error in client reading socket buffer: %s\n", strerror(errno));
                exit(1);
            }
            if (num == 0) {
                fprintf(stderr, "Error: client empty socket buffer: %s\n", strerror(errno));
                exit(0);
            }
            
            if(crypt_flag) {
                decrypt_buf(buffer, BUFSIZE);
            }
            
            int i;
            for (i = 0; i < num; i++) {
                switch(input[i]){
                    case CARET_D:
                        write(STDOUT_FILENO, eof, 2);
                        break;
                    case NEW_LINE:
                        write(STDOUT_FILENO, newline, 2);
                        break;
                    default:
                        write(STDOUT_FILENO, input + i, 1);
                        continue;
                }
            }
            fprintf(fptr, "RECEIVED %d bytes: %s \n", num, input);

        }
        
        if (polls[0].revents & (POLLERR | POLLHUP) || polls[1].revents & (POLLERR | POLLHUP)) {
        exit(0);
        }
    }
}


void connect_to_server(){
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
  
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"Error finding host: %s\n", strerror(errno));
        exit(1);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
    
    
    read_and_write();
}


int main(int argc, char* argv[]){
    tcgetattr(0,&orig_terminal); //save copy of original terminal
    set_no_echo_terminal();
    atexit(reset_terminal); //when program exit, reset normal settings for terminal

    static struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"encrypt", required_argument, NULL, 'e'},
        {"log", required_argument, NULL, 'l'}
    };
    
    
    int port_flag = 0;
    int c;
    while((c = getopt_long(argc, argv, "p:e:l:", long_options, NULL)) != -1){
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
            case 'l':
                log_flag = 1;
                fptr = fopen(optarg, "r+");
                if(fptr == NULL) //if file does not exist, create it
                {
                    fptr = fopen(optarg, "w+");
                }
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab1b [--port=port] [--encrypt=filename] [--log=filename] accepted\n");
                exit(1);
                break;
        }
    }
    
    if(port_flag == 1){
        connect_to_server();
    }
    else{
        fprintf(stderr, "ERROR: Unrecognized argument; only ./lab1b [--port=port] [--encrypt=filename] [--log=filename] accepted\n");
        exit(1);
        
    }

    exit(0);
}
