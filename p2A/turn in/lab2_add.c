//
//  lab2_add.c
//  p2A
//
//  Created by Keiana Snell on 10/31/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

int num_threads = 1;
int num_its = 1;
char* test_name = "add-none"; //"add-yield-none"
char lock = 'n';
int l = 0;
pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

//void add(long long *pointer, long long value) {
//    long long sum = *pointer + value;
//    *pointer = sum;
//}

int opt_yield = 0;
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

void m_add(long long *pointer, long long value) {
    pthread_mutex_lock(&m_lock);
    long long sum = *pointer + value;
    if (opt_yield) {
        sched_yield();
    }
    *pointer = sum;
    pthread_mutex_unlock(&m_lock);
}

void s_add(long long *pointer, long long value) {
    while (__sync_lock_test_and_set(&l, 1));
    long long sum = *pointer + value;
    if (opt_yield) {
        sched_yield();
    }
    *pointer = sum;
    __sync_lock_release(&l);
}

void c_add(long long *pointer, long long value) {
    long long cur;
    long long new;
    do {
        cur = *pointer;
        new = cur + value;
        if(opt_yield)
            sched_yield();
    } while(__sync_val_compare_and_swap(pointer, cur, new) != cur);
}

void* add_thread(void* counter_ptr){
    long long *ptr = (long long *) counter_ptr;
    int i;
    for(i = 0; i < num_its; i++){
        if(lock == 'm'){
            if(opt_yield) test_name = "add-yield-m";
            else test_name = "add-m";
            m_add(ptr, 1);
        }
        else if(lock == 's'){
            if(opt_yield) test_name = "add-yield-s";
            else test_name = "add-s";
            s_add(ptr, 1);
        }
        else if(lock == 'c'){
            if(opt_yield) test_name = "add-yield-c";
            else test_name = "add-c";
            c_add(ptr, 1);
        }
        else { //lock does not exist
            if(opt_yield) test_name = "add-yield-none";
            add(ptr, 1);
        }
    }
    
    for(i = 0; i < num_its; i++){
        if(lock == 'm'){
            m_add(ptr, -1);
        }
        else if(lock == 's'){
            s_add(ptr, -1);
        }
        else if(lock == 'c'){
            c_add(ptr, -1);
        }
        else {
            add(ptr, -1);
        }
    }
    
    return NULL;
}


int main(int argc, char* argv[]){
    static const struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0,0,0,0},
    };
    
    
    int c;
    while((c = getopt_long(argc, argv, "", long_options, NULL)) != -1){
        switch(c){
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_its = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                test_name = "add-yield";
                break;
            case 's':
                switch (*optarg) {
                    case 'm':
                    case 's':
                    case 'c':
                        lock = optarg[0];
                        break;
                    default:
                        fprintf(stderr, "ERROR: Unrecognized --sync argument. \n");
                        exit(1);
                        break;
                }
                break;
            default:
                fprintf(stderr, "ERROR: Unrecognized argument; only ./lab2a_add [--threads=#] [--iterations=#] [--yield] [--sync=ms] accepted. \n");
                exit(1);
                break;
        }
    }
    
    struct timespec start_time;
    struct timespec end_time;
    
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1){
        fprintf(stderr,"Error in starting clock: %s\n", strerror(errno));
        exit(2);
    }
    
    //run threads here
    
    pthread_t threads[num_threads];
    long long counter = 0;

    //initialize threads[]
    int i;
    for(i = 0; i < num_threads; i++) {
        int err = pthread_create(&threads[i], NULL, add_thread, &counter);
        if (err) {
            fprintf(stderr, "Error in pthread_create(): %s\n", strerror(errno));
            exit(2);
        }
    }
    
    for(i = 0; i < num_threads; i++) {
        int err = pthread_join(threads[i], NULL);
        if (err) {
            fprintf(stderr, "Error in pthread_join(): %s\n", strerror(errno));
            exit(2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    
    long long total = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
    int num_ops = num_threads * num_its * 2;
    
    long long average_time = total / ((long long)num_ops);
    
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", test_name, num_threads, num_its, num_ops, total, average_time, counter);
    
    
    
    exit(0);
}
