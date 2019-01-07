//
//  lab4b.c
//  p4B
//
//  Created by Keiana Snell on 11/12/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mraa.h>
#include <math.h>
#include <getopt.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>

#define BUFSIZE 256

const int A0 = 1;
const int GPIO_50 = 60;

mraa_gpio_context button;
mraa_aio_context temp_sensor;
char* filename;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
char scale = 'F'; //By default, temperatures should be reported in degrees Fahrenheit
int period_interval = 1; //sampling interval in seconds
int log_flag = 0;
FILE* logfile = NULL;

int off = 0;
int start = 1;
time_t curr_time;
time_t next_time = 0;


void shut_down(){
    time(&curr_time);
    struct tm *now;
    now = localtime(&curr_time);
    fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
    if(log_flag) fprintf(logfile, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
}

void cleanup(){
    shut_down();
    mraa_aio_close(temp_sensor);
    mraa_gpio_close(button);
}

float read_temp(){
    int a = mraa_aio_read(temp_sensor);
    if(a == -1){
        fprintf(stderr, "Error in reading temperature: %s\n", strerror(errno));
        exit(1);
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
    fprintf(stdout, "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temp);

    if(log_flag){
        fprintf(logfile, "%02d:%02d:%02d %.1f\n", now->tm_hour, now->tm_min, now->tm_sec, temp);
    }
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
    
    if(log_flag) fprintf(logfile, "%s\n", command);
}


int read_button(){
    int read = mraa_gpio_read(button);
    if(read == -1){
        fprintf(stderr, "Error in reading button: %s\n", strerror(errno));
        exit(1);
    }
    return read;
}

int main(int argc, char* argv[]){
    atexit(cleanup);
    
    static const struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", optional_argument, NULL, 'l'},
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
                if(logfile == NULL)
                {
                    fprintf(stderr, "ERROR in opening file argument: %s\n", strerror(errno));
                    exit(1);
                }
                log_flag = 1;
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab4b [--period=#] [--scale=[CF]] [--log=FILENAME] accepted. \n");
                exit(1);
                break;
        }
    }
    
    
    temp_sensor = mraa_aio_init(A0);
    if (temp_sensor == NULL) {
        fprintf(stderr, "Error in initializing AIO: %s\n", strerror(errno));
        //return EXIT_FAILURE;
    }
    
    button = mraa_gpio_init(GPIO_50);
    if (button == NULL) {
        fprintf(stderr, "Error in initializing GPIO_50: %s\n", strerror(errno));
        //return EXIT_FAILURE;
    }
    
    
    struct pollfd polls[] = {
        {STDIN_FILENO, POLLIN| POLLHUP | POLLERR, 0},
    };
    
    char buffer[BUFSIZE];
    char cache[BUFSIZE];
    
    memset(buffer, 0, BUFSIZE);
    memset(cache, 0, BUFSIZE);
    
    int index = 0;
    while(!off){
        if(read_button()){
            exit(0);
        }
        float x = read_temp();
        time(&curr_time);
        if (start && curr_time >= next_time) {
            write_temp(x);
        }
        int ret = poll(polls, 1, 0); //not -1
        if (ret < 0) {
            fprintf(stderr, "Error in client starting poll: %s\n", strerror(errno));
        }

        if (polls[0].revents & POLLIN) {
            int num_read = read(STDIN_FILENO, buffer, BUFSIZE);
            if (num_read < 0) {
                fprintf(stderr, "Error in reading from buffer: %s\n", strerror(errno));
                exit(1);
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
    exit(0);
}
