//
//  lab2_list.c
//  p2B
//
//  Created by Keiana Snell on 11/04/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "SortedList.h"

#define NUM 1000000000
int num_threads = 1;
int num_its = 10;
int num_lists = 1;
int num_elements = 0;
char test_name[16] = "list"; /* `list-yieldopts-syncopts`:
                              yieldopts = {none, i,d,l,id,il,dl,idl}
                              syncopts = {none,s,m} */
char lock = 'n';
int l = 0;
int opt_yield = 0;
int y_flag = 0;
int length = 0;
pthread_mutex_t* m_lock;
int* s_lock;
SortedList_t* list;
SortedListElement_t* elements;
long long * wait_times;


void cleanup(){
    if(elements) {
        int i;
        for(i = 0; i < num_elements; i++){
            free((char*)elements[i].key);
        }
        free(elements);
    }
    if(list) free(list);
   // if(wait_times) free(wait_times);
    
    if(m_lock){
        int i;
        for(i = 0; i < num_lists; i++){
            pthread_mutex_destroy(&m_lock[i]);
        }
    }
    else if(s_lock){
        free(s_lock);
    }
}

void signal_handler() {
    fprintf(stderr, "Error: Signal was caught, %s\n", strerror(errno));
    exit(2);
}

unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash%num_lists;
}

void* list_thread(void* arg){
    int *thread_ID = (int*) arg;
    int i;
    
    struct timespec start_t;
    struct timespec end_t;
    
    //insert
    for(i = *thread_ID; i < num_elements; i += num_threads){
        int index = (int)hash(elements[i].key);
        if(lock == 'm'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            pthread_mutex_lock(&m_lock[index]);
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            
            SortedList_insert(&list[index], &elements[i]);
            pthread_mutex_unlock(&m_lock[index]);
        }
        else if(lock == 's'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            while (__sync_lock_test_and_set(&s_lock[index], 1));
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            SortedList_insert(&list[index], &elements[i]);
            __sync_lock_release(&s_lock[index]);
        }
        else { //no lock
            SortedList_insert(&list[index], &elements[i]);
        }
    }
    
    //get list length
    for(i = *thread_ID; i < num_elements; i += num_threads){
        int index = (int)hash(elements[i].key);
        if(lock == 'm'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            pthread_mutex_lock(&m_lock[index]);
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            
            SortedList_length(&list[index]);
            pthread_mutex_unlock(&m_lock[index]);
        }
        else if(lock == 's'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            while (__sync_lock_test_and_set(&s_lock[index], 1));
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            SortedList_length(&list[index]);
            __sync_lock_release(&s_lock[index]);
        }
        else { //no lock
            SortedList_length(&list[index]);
        }
    }
    
    
    //delete elements
    SortedListElement_t *inserted;
    for(i = *thread_ID; i < num_elements; i += num_threads){
        int index = (int)hash(elements[i].key);
        if(lock == 'm'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            pthread_mutex_lock(&m_lock[index]);
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            
            inserted = SortedList_lookup(&list[index], elements[i].key);
            if (inserted == NULL) {
                fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                exit(2);
            }
            if (SortedList_delete(inserted) == 1) {
                fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                exit(2);
            }
            pthread_mutex_unlock(&m_lock[index]);
        }
        else if(lock == 's'){
            if (clock_gettime(CLOCK_MONOTONIC, &start_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            while (__sync_lock_test_and_set(&s_lock[index], 1));
            if (clock_gettime(CLOCK_MONOTONIC, &end_t) == -1){
                fprintf(stderr, "Error: Cannot acquired clock time. \n");
                exit(1);
            }
            wait_times[*thread_ID] += (end_t.tv_sec - start_t.tv_sec) * NUM + (end_t.tv_nsec - start_t.tv_nsec);
            inserted = SortedList_lookup(&list[index], elements[i].key);
            if (inserted == NULL) {
                fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                exit(2);
            }
            if (SortedList_delete(inserted) == 1) {
                fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                exit(2);
            }
            __sync_lock_release(&s_lock[index]);
        }
        else { //no lock
            SortedList_length(&list[index]);
            inserted = SortedList_lookup(&list[index], elements[i].key);
            if (inserted == NULL) {
                fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                exit(2);
            }
            if (SortedList_delete(inserted) == 1) {
                fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                exit(2);
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]){
    atexit(cleanup);
    
    signal(SIGSEGV, signal_handler);
    
    static const struct option long_options[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", optional_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
        {0,0,0,0},
    };
    
    char* x = "-";
    char* y_opt = NULL;
    int c, i;
    while((c = getopt_long(argc, argv, "", long_options, NULL)) != -1){
        switch(c){
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'i':
                num_its = atoi(optarg);
                break;
            case 'l':
                num_lists = atoi(optarg);
                break;
            case 'y':
                opt_yield = 0;
                y_flag = 1;
                unsigned long num = strlen(optarg);
                y_opt = (char*) malloc(num);
                for(i = 0; i < (int)num; i++){
                    y_opt[i] = optarg[i];
                    switch (*optarg){
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "ERROR: Unrecognized --yield argument. \n");
                            exit(1);
                            break;
                    }
                }
                break;
            case 's':
                switch (*optarg) {
                    case 'm':
                    case 's':
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
    
    
    if(y_flag){
        strcat(test_name, x);
        strcat(test_name, y_opt);
    }
    else {
        strcat(test_name, "-none");
    }
    if(lock != 'n'){
        strcat(test_name, x);
        strcat(test_name, &lock);
    }
    else {
        strcat(test_name, "-none");
    }
    
    list = malloc(num_lists*sizeof(SortedList_t));
    for(i = 0; i < num_lists; i++){
        list[i].next = &list[i];
        list[i].prev = &list[i];
        list[i].key = NULL;
    }

    if(lock == 'm'){
        m_lock = malloc(num_lists*sizeof(pthread_mutex_t));
        for (i = 0; i < num_lists; i++)
            pthread_mutex_init(&m_lock[i], NULL);
    }
    else if(lock == 's'){
        s_lock = malloc(num_lists*sizeof(int));
        for (i = 0; i < num_lists; i++)
            s_lock[i] = 0;
    }
    
    
    wait_times = malloc(num_threads*sizeof(int));
    for(i = 0; i < num_threads; i++){
        wait_times[i] = 0;
    }
    
    
    num_elements = num_threads*num_its;
    elements = malloc(sizeof(SortedListElement_t)*num_elements);
    srand((unsigned)time(NULL));
    char key[3];
    key[2] = '\0';
    
    for(i = 0; i < num_elements; i++){
        key[0] = (char)(rand() % 256);
        key[1] = (char)(rand() % 256);
        elements[i].key = strdup(key);
    }
    
    struct timespec start_time;
    struct timespec end_time;
    
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1){
        fprintf(stderr,"Error in starting clock: %s\n", strerror(errno));
        exit(2);
    }
    
    pthread_t threads[num_threads];
    int thread_IDs[num_threads];
    
    for(i = 0; i < num_threads; i++){
        thread_IDs[i] = i;
    }
    
    //initialize threads[]
    for(i = 0; i < num_threads; i++) {
        int err = pthread_create(&threads[i], NULL, list_thread, &thread_IDs[i]);
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
    
    //checks the length of the list to confirm that it is zero.
    if(SortedList_length(list) != 0){
        fprintf(stderr, "Error: list did not end up with length 0 %s\n", strerror(errno));
        exit(2);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    long long total = (end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec);
    int num_ops = num_threads * num_its * 3;
    long long average_time = total / ((long long)num_ops);
    long long total_wait_time = 0;
    for (i = 0; i < num_threads; i++)
        total_wait_time += wait_times[i];
    int num_locks = (2*num_its + 1) * num_threads;
    long long average_wait = total_wait_time / num_locks;
    
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld,%lld\n", test_name, num_threads, num_its, num_lists, num_ops, total, average_time, average_wait);
    
    
    exit(0);
}
