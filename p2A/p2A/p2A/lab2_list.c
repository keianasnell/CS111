//
//  lab2_list.c
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
#include <signal.h>
#include <time.h>
#include "SortedList.h"

int num_threads = 1;
int num_its = 10;
int num_elements = 0;
char test_name[16] = "list"; /* `list-yieldopts-syncopts`:
                              yieldopts = {none, i,d,l,id,il,dl,idl}
                              syncopts = {none,s,m} */
char lock = 'n';
int l = 0;
int opt_yield = 0;
int y_flag = 0;
int length = 0;
pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;
SortedList_t* list;
SortedListElement_t* elements;

void cleanup(){
    if(elements) {
        int i;
        for(i = 0; i < num_elements; i++){
            free((char*)elements[i].key);
        }
        free(elements);
    }
    if(list) free(list);
}

void signal_handler() {
    fprintf(stderr, "Error: Signal was caught, %s\n", strerror(errno));
    exit(2);
}

void m_insert(SortedList_t* list, SortedListElement_t*  element) {
    pthread_mutex_lock(&m_lock);
    SortedList_insert(list, element);
    pthread_mutex_unlock(&m_lock);
}

void s_insert(SortedList_t* list, SortedListElement_t*  element) {
    while (__sync_lock_test_and_set(&l, 1));
    SortedList_insert(list, element);
    __sync_lock_release(&l);
}


void* list_thread(void* arg){
    /*
     starts with a set of pre-allocated and initialized elements (--iterations=#)
     inserts them all into a (single shared-by-all-threads) list
     gets the list length
     looks up and deletes each of the keys it had previously inserted
     exits to re-join the parent thread
     */
    int *thread_ID = (int*) arg;
    int i;
    for(i = *thread_ID; i < num_elements; i += num_threads){
        if(lock == 'm'){
            //m_insert(list, &elements[i]);
            pthread_mutex_lock(&m_lock);
            SortedList_insert(list, &elements[i]);
            
            pthread_mutex_unlock(&m_lock);
        }
        else if(lock == 's'){
            //s_insert(list, &elements[i]);
            while (__sync_lock_test_and_set(&l, 1));
            SortedList_insert(list, &elements[i]);
            __sync_lock_release(&l);
        }
        else {
            SortedList_insert(list, &elements[i]);
        }
    }
    
    switch (lock) {
        case 'n':
            if (SortedList_length(list) < 0) {
                fprintf(stderr, "Error: Corrupted list detected while getting length.\n");
                exit(2);
            }
            break;
        case 'm':
            pthread_mutex_lock(&m_lock);
            if (SortedList_length(list) < 0) {
                fprintf(stderr, "Error: Corrupted list detected while getting length.\n");
                exit(2);
            }
            pthread_mutex_unlock(&m_lock);
            break;
        case 's':
            while (__sync_lock_test_and_set(&lock, 1) == 1) ;
            if (SortedList_length(list) < 0) {
                fprintf(stderr, "Error: Corrupted list detected while getting length.\n");
                exit(2);
            }
            __sync_lock_release (&lock);
            break;
    }
    
    
    SortedListElement_t *inserted;
    for (i = *thread_ID; i < num_elements; i += num_threads) {
        switch(lock) {
            case 'm':
                pthread_mutex_lock(&m_lock);
                inserted = SortedList_lookup(list, elements[i].key);
                if (inserted == NULL) {
                    fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                    exit(2);
                }
                if (SortedList_delete(inserted) == 1) {
                    fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                    exit(2);
                }
                pthread_mutex_unlock(&m_lock);
                break;
            case 's':
                while (__sync_lock_test_and_set(&l, 1));
                inserted = SortedList_lookup(list, elements[i].key);
                if (inserted == NULL) {
                    fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                    exit(2);
                }
                if (SortedList_delete(inserted) == 1) {
                    fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                    exit(2);
                }
                __sync_lock_release(&l);
                break;
            default: //no lock
                inserted = SortedList_lookup(list, elements[i].key);
                if (inserted == NULL) {
                    fprintf(stderr, "Error: Corrupted list. Specified element key not found.\n");
                    exit(2);
                }
                if (SortedList_delete(inserted) == 1) {
                    fprintf(stderr, "Error: Corrupted list detected while deleting.\n");
                    exit(2);
                }
                break;
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
            case 'y':
                opt_yield = 0;
                y_flag = 1;
                unsigned long num = strlen(optarg);
                y_opt = (char*) malloc(num);
                for(i = 0; i < num; i++){
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
    

/*
 initializes an empty list.
 creates and initializes (with random keys) the required number (threads x iterations) of list elements. Note that we do this before creating the threads so that this time is not included in our start-to-finish measurement. Similarly, if you free elements at the end of the test, do this after collecting the test execution times.
 */
    list = malloc(sizeof(SortedList_t));
    list->next = list;
    list->prev = list;
    list->key = NULL;
    
    num_elements = num_threads*num_its;
    elements = malloc(sizeof(SortedListElement_t)*num_elements);
    srand((unsigned)time(NULL));
    char key[3];
    key[2] = '\0';

    for(i = 0; i < num_elements; i++){
        char key[3];
        key[0] = (char)(rand() % 256);
        key[1] = (char)(rand() % 256);
        elements[i].key = strdup(key);
    }
    
    for (i = 0; i < num_elements; i++) {
        printf("%s\n", elements[i].key);
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
    int num_lists = 1;
    
    long long average_time = total / ((long long)num_ops);
    
    fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%lld\n", test_name, num_threads, num_its, num_lists, num_ops, total, average_time);

    
    exit(0);
}
