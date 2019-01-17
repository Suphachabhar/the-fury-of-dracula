// DLList.h - Interface to doubly-linked list ADT
// Written by John Shepherd, March 2013
// Last modified, August 2014

#ifndef DLLIST_H
#define DLLIST_H

#include "places.h"

// External view of DLList
// Implementation given in DLList.c
// Implements a DLList of strings (i.e. items are strings)

typedef struct DLListNode
{
    location_t location; // value of this list item (string)
    struct DLListNode *prev;
    // pointer previous node in list
    struct DLListNode *next;
    // pointer to next node in list
} DLListNode;

typedef struct DLListRep
{
    size_t nitems;        // count of items in list
    DLListNode *first; // first node in list
    DLListNode *curr;  // current node in list
    DLListNode *last;  // last node in list
} DLListRep;

typedef struct DLListRep *DLList;

// create a new empty DLList
DLList newDLList();


// free list
void freeDLList(DLList L);

//insert at back
void DLListInsert(DLList L, location_t location);

//insert at back with no duplicated location
void DLListInsert_nodup(DLList L, location_t location);



#endif