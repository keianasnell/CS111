//
//  SortedList.c
//  p2A
//
//  Created by Keiana Snell on 10/31/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <sched.h>
#include "SortedList.h"


void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if (list == NULL || element == NULL) return;
    if(list->next == NULL) {
        list->next = element;
        list->prev = element;
        element->next = list;
        element->prev = list;
        return;
    }
    
    SortedListElement_t* curr = list->next;
    while((curr->key != NULL) && strcmp(element->key, curr->key) > 0) {
        if (opt_yield & INSERT_YIELD) sched_yield();
        curr = curr->next;
    }
    
    if (opt_yield & INSERT_YIELD) sched_yield();
    
    element->next = curr;
    element->prev = curr->prev;
    curr->prev->next = element;
    curr->prev= element;
}


int SortedList_delete( SortedListElement_t *element){
  //  if (element == NULL) return 1;
    
    if ((element->next->prev == element) && (element->prev->next == element)) {
        if (opt_yield & DELETE_YIELD) sched_yield();
        element->next->prev = element->prev;
        element->prev->next = element->next;
        return 0;
    }
    return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if (list == NULL || key == NULL) return NULL;
    
    SortedListElement_t* element = list->next;
    
    while (element->key != NULL) {
        if (strcmp(key, element->key) == 0) {
            return element;
        }
        if (opt_yield & LOOKUP_YIELD) {
            sched_yield();
        }
        element = element->next;
    }
    return NULL;
}


int SortedList_length(SortedList_t *list){
    //if (list == NULL) return -1;
    
    int count = 0;
    SortedListElement_t *curr = list->next;
    while(curr->key != NULL) {
        count = count + 1;
        if (opt_yield & LOOKUP_YIELD) sched_yield();
        if ((curr->next->prev != curr) || (curr->prev->next != curr))
            return -1;
        curr = curr->next;
    }
    return count;
}


