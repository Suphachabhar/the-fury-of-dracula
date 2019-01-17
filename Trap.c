// Trap.c - Implementation of doubly-linked list ADT

#include <assert.h>

#include "places.h"
#include "Trap.h"

// data structures representing Trap

// typedef struct TrapNode TrapNode;

// create a new TrapNode (private function)

static void TrapLocationInsert(TrapLocation *location);

static TrapNode *newTrapNode()
{
    TrapNode *new;
    new = malloc(sizeof(TrapNode));
    assert(new != NULL);
    new->turn = 6;
    new->next = NULL;

    return new;
}

static TrapLocation *newTrapLocation(location_t location)
{
    TrapLocation *new;
    new = malloc(sizeof(TrapLocation));
    assert(new != NULL);
    new->n_traps = 0;
    new->trap_first = NULL;
    new->trap_last = NULL;
    new->next = NULL;
    return new;
}

// create a new empty Trap
Trap newTrap()
{
    struct TrapRep *new;
    new = malloc(sizeof(struct TrapRep));
    assert(new != NULL);
    new->n_locations = 0;
    new->first = NULL;
    new->last = NULL;
    return new;
}

// free up all space associated with list
void freeTrap(Trap L)
{
    assert(L != NULL);
    TrapLocation *curr, *prev;
    curr = L->first;
    while (curr != NULL)
    {
        prev = curr;
        curr = curr->next;
        if (prev->n_traps > 0)
        {
            TrapNode *trap_curr, *trap_prev;
            trap_curr = prev->trap_first;
            while (trap_curr != NULL)
            {
                trap_prev = trap_curr;
                trap_curr = trap_curr->next;
                free(trap_prev);
            }
        }
        free(prev);
    }
    free(L);
}

void TrapInsert(Trap L, location_t location)
{
    assert(L != NULL);

    TrapLocation *curr;
    for (curr = L->first; curr != NULL; curr = curr->next)
    {
        if (curr->location == location)
            break;
    }
    if (curr == NULL)
    {
        TrapLocation *new_location = newTrapLocation(location);
        TrapLocationInsert(new_location);
        L->n_locations++;
        L->last->next = new_location;
        L->last = new_location;
    }
    else
    {
        TrapLocationInsert(curr);
    }
}

void turn_pass(Trap L)
{
    assert(L != NULL);
    for (TrapLocation *curr = L->first; curr != NULL; curr = curr->next)
    {
        if (curr->n_traps > 0)
        {
            for (TrapNode *trap_curr = curr->trap_first; trap_curr != NULL; trap_curr = trap_curr->next)
            {
                if (trap_curr->turn > 0)
                    trap_curr->turn--;
                // else vanished?
            }
        }
    }
}

void remove_trap(Trap L)
{
    for (TrapLocation *curr = L->first; curr != NULL; curr = curr->next)
    {
        if (curr->n_traps > 0)
        {
            TrapNode *trap_prev = NULL;
            for (TrapNode *trap_curr = curr->trap_first; trap_curr != NULL; trap_curr = trap_curr->next)
            {
                if (trap_curr->turn == 0)
                {
                    if (trap_prev == NULL)
                        curr->trap_first = trap_curr->next;
                    else
                        trap_prev->next = trap_curr->next;
                    free(trap_curr);
                }
                curr->n_traps--;
            }
        }
    }
}

static void TrapLocationInsert(TrapLocation *location)
{
    TrapNode *newtrap = newTrapNode();
    if (location->n_traps == 0)
        location->trap_first = newtrap;
    else
        location->trap_last->next = newtrap;
    location->trap_last = newtrap;
    location->n_traps++;
}
