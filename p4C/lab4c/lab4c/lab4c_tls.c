//
//  lab4c_tcp.c
//  lab4c
//
//  Created by Keiana Snell on 12/1/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mraa.h>
#include <math.h>
#include <netinet/in.h>
#include <getopt.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <signal.h>

#define BUFSIZE 256

const int A0 = 1;

//mraa_gpio_context button;
mraa_aio_context temp_sensor;
char* filename;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
char scale = 'F'; //By default, temperatures should be reported in degrees Fahrenheit
int period_interval = 1; //sampling interval in seconds
FILE* logfile = NULL;
char* host = NULL;
char* id = NULL;
int port = 0;
int sockfd;
int tls = 0;
SSL_CTX* context = NULL;
SSL *ssl = NULL;
char SSL_buf[BUFSIZE];


int off = 0;
int start = 1;
time_t curr_time;
time_t next_time = 0;


void shut_down(){
    time(&curr_time);
    struct tm *now;
    now = localtime(&curr_time);
    sprintf(SSL_buf, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
    SSL_write(ssl, SSL_buf, strlen(SSL_buf));
    fprintf(logfile, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
}

float read_temp(){
    int a = mraa_aio_read(temp_sensor);
    if(a == -1){
        fprintf(stderr, "Error in reading temperature: %s\n", strerror(errno));
        exit(2);
    }
    
    float R = 1023.0/((float)a) - 1.0;
    R = R0*R;
    float temp = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
    if (scale == 'F') return (temp * 9)/5 + 32;
    else return temp;
}

void write_temp(float temp){
    time(&curr_time);
    struct tm *now;
    now = localtime(&curr_time);
    sprintf(SSL_buf, "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temp);
    SSL_write(ssl, SSL_buf, strlen(SSL_buf));
    fprintf(logfile, "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temp);
    next_time = curr_time + period_interval;
    
}

void commands(char* command){
    if(strncmp(command, "PERIOD=", 7*sizeof(char)) == 0){
        period_interval = (int)atoi(command+7);
    }
    else if(strcmp(command, "SCALE=F") == 0) {scale = 'F';}
    else if(strcmp(command, "SCALE=C") == 0) {scale = 'C';}
    else if(strcmp(command, "STOP") == 0) {start = 0;}
    else if(strcmp(command, "START") == 0) {start = 1;}
    else if(strcmp(command, "OFF") == 0) {off = 1;}
    
    fprintf(logfile, "%s\n", command);
}


void connect_to_server(){
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error in opening socket: %s\n", strerror(errno));
        exit(2);
    }
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"Error finding host: %s\n", strerror(errno));
        exit(2);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
        exit(2);
    }
}

void connect_SSL(){
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    context = SSL_CTX_new(TLSv1_client_method());
    if (context == NULL) {
        fprintf(stderr, "Error in retrieving SSL context: %s\n", strerror(errno));
        exit(2);
    }
    ssl = SSL_new(context);
    if (ssl == NULL) {
        fprintf(stderr, "Error in completing SSL setup: %s\n", strerror(errno));
        exit(2);
    }
    if (!SSL_set_fd(ssl, sockfd)) {
        fprintf(stderr, "Error in connecting file descriptor and SSL: %s\n", strerror(errno));
        exit(2);
    }
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "Error, SSL connection rejected: %s\n", strerror(errno));
        exit(2);
    }
}



int main(int argc, char* argv[]){
    static const struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {0,0,0,0},
    };
    
    int c;
    while((c = getopt_long(argc, argv, "", long_options, NULL)) != -1){
        switch(c){
            case 'p':
                period_interval = atoi(optarg);
                break;
            case 's':
                scale = *optarg;
                break;
            case 'l':
                logfile = fopen(optarg, "w+");
                if(logfile == NULL) {
                    fprintf(stderr, "ERROR in opening file argument: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 'i':
                id = optarg;
                if (strlen(id) != 9) {
                    fprintf(stderr, "Error: ID should be 9 digits.\n");
                    exit(1);
                }
                break;
            case 'h':
                host = optarg;
                if(!strlen(host)){
                    fprintf(stderr, "Error in determining host name.\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab4c [--period=#] [--scale=[CF]] [--log=FILENAME] --id=[ID_NUM] --host=[HOSTNAME] portnumber accepted. \n");
                exit(1);
                break;
        }
    }
    
    if (!host | !id | !logfile) {
        fprintf(stderr, "ERROR: Unrecognized argument; only ./lab4c [--period=#] [--scale=[CF]] [--log=FILENAME] --id=[ID_NUM] --host=[HOSTNAME] portnumber accepted. \n");
        exit(1);
    }
    
    if (optind < argc) {
        port = (int)atoi(argv[optind]);
        if (port <= 0) {
            fprintf(stderr, "ERROR: Unrecognized argument; only ./lab4c [--period=#] [--scale=[CF]] [--log=FILENAME] --id=[ID_NUM] --host=[HOSTNAME] portnumber accepted. \n");
            exit(1);
        }
    }
    
    connect_to_server();
    connect_SSL();
    
    sprintf(SSL_buf, "ID=%s\n", id);
    SSL_write(ssl, SSL_buf, strlen(SSL_buf));
    fprintf(logfile, "ID=%s\n", id);
    
    temp_sensor = mraa_aio_init(A0);
    if (temp_sensor == NULL) {
        fprintf(stderr, "Error in initializing AIO: %s\n", strerror(errno));
        exit(2);
    }
    
    struct pollfd polls[] = {
        {sockfd, POLLIN| POLLHUP | POLLERR, 0},
    };
    
    char buffer[BUFSIZE];
    char cache[BUFSIZE];
    
    memset(buffer, 0, BUFSIZE);
    memset(cache, 0, BUFSIZE);
    
    int index = 0;
    while(!off){
        float x = read_temp();
        time(&curr_time);
        if (start && curr_time >= next_time) {
            write_temp(x);
        }
        int ret = poll(polls, 1, 0); //not -1
        if (ret < 0) {
            fprintf(stderr, "Error in client starting poll: %s\n", strerror(errno));
            exit(2);
        }
        
        if (polls[0].revents & POLLIN) {
            int num_read = SSL_read(ssl, buffer, BUFSIZE);
            if (num_read < 0) {
                fprintf(stderr, "Error in reading from buffer: %s\n", strerror(errno));
                exit(2);
            }
            
            int i;
            for (i = 0; i < num_read && index < BUFSIZE; i++) {
                if (buffer[i] == '\n') {
                    commands((char*)&cache);
                    index = 0;
                    memset(cache, 0, BUFSIZE);
                } else {
                    cache[index] = buffer[i];
                    index++;
                }
            }
        }
    }
    
    shut_down();
    mraa_aio_close(temp_sensor);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    
    exit(0);
}
