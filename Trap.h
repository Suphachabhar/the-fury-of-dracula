// Trap.h - Interface to doubly-linked list ADT

#ifndef TRAP_H
#define TRAP_H

#include "places.h"

// External view of Trap
// Implementation given in Trap.c
// Implements a Trap of strings (i.e. items are strings)

typedef struct TrapNode
{
    size_t turn;
    struct TrapNode *next;
} TrapNode;

typedef struct TrapLocation
{
    location_t location;
    size_t n_traps;
    struct TrapNode *trap_first;
    struct TrapNode *trap_last;
    struct TrapLocation *next;
} TrapLocation;

typedef struct TrapRep
{
    size_t n_locations;  // count of items in list
    TrapLocation *first; // first node in list
    TrapLocation *last;  // last node in list
} TrapRep;

typedef struct TrapRep *Trap;

// create a new empty Trap
Trap newTrap();

// free list
void freeTrap(Trap L);

//add trap
void TrapInsert(Trap L, location_t location);

void turn_pass(Trap L);

void remove_trap(Trap L);

#endif