// DLList.c - Implementation of doubly-linked list ADT
// Written by John Shepherd, March 2013
// Modified by John Shepherd, August 2014, August 2015

#include <assert.h>

#include "places.h"
#include "DDList.h"

// data structures representing DLList

// typedef struct DLListNode DLListNode;

// create a new DLListNode (private function)

static DLListNode *newDLListNode(location_t location)
{
    DLListNode *new;
    new = malloc(sizeof(DLListNode));
    assert(new != NULL);
    new->location = location;
    new->prev = new->next = NULL;
    return new;
}

// create a new empty DLList
DLList newDLList()
{
    struct DLListRep *new;
    new = malloc(sizeof(struct DLListRep));
    assert(new != NULL);
    new->nitems = 0;
    new->curr = NULL;
    new->first = NULL;
    new->last = NULL;
    return new;
}

// free up all space associated with list
void freeDLList(DLList L)
{
    assert(L != NULL);
    DLListNode *curr, *prev;
    curr = L->first;
    while (curr != NULL)
    {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(L);
}

void DLListInsert(DLList L, location_t location)
{
    assert(L != NULL);
    DLListNode *ins = newDLListNode(location);
    if (L->nitems == 0)
    {
        L->first = ins;
    }
    else
    {
        L->last->next = ins;
        ins->prev = L->last;
    }
    L->curr = L->first;
    L->last = ins;
    L->nitems++;
}

void DLListInsert_nodup(DLList L, location_t location)
{
    assert(L != NULL);
    for (DLListNode *curr = L->first; curr != NULL; curr = curr->next)
    {
        if (curr->location == location)
            return;
    }
    DLListInsert(L, location);
}