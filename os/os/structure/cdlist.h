#pragma once

#include "enclosing.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// ---------------------------
// CIRCULAR DOUBLY-LINKED LIST
// ---------------------------

// Intended usage:
//
//  myThings is of type CDListItem
//  thing1, thing2, thing3 are structs, where member3 is of type CDListItem
//
//   +------------------------------------------------------------------------------+
//   |                                                                              |
//   |                   +--------------+    +--------------+    +--------------+   |
//   |                   |thing1        |    |thing2        |    |thing3        |   |
//   |                   +--------------+    +--------------+    +--------------+   |
//   |                   |              |    |              |    |              |   |
//   |                   |  member1     |    |  member1     |    |  member1     |   |
//   |                   |              |    |              |    |              |   |
//   |                   |  member2     |    |  member2     |    |  member2     |   |
//   |                   |              |    |              |    |              |   |
//   |   +----------+    | +----------+ |    | +----------+ |    | +----------+ |   |
//   |   |myThings  |    | |member3   | |    | |member3   | |    | |member3   | |   |
//   |   +----------+    | +----------+ |    | +----------+ |    | +----------+ |   |
//   |   |          |    | |          | |    | |          | |    | |          | |   |
//   +-->|   next+-------->|   next----------->|   next----------->|   next---------+
//       |          |    | |          | |    | |          | |    | |          | |
//   +-------prev   |<---------prev   |<-----------prev   |<-----------prev   |<----+
//   |   |          |    | |          | |    | |          | |    | |          | |   |
//   |   +----------+    | +----------+ |    | +----------+ |    | +----------+ |   |
//   |                   |              |    |              |    |              |   |
//   |                   |  member4     |    |  member4     |    |  member4     |   |
//   |                   |              |    |              |    |              |   |
//   |                   +--------------+    +--------------+    +--------------+   |
//   |                                                                              |
//   +------------------------------------------------------------------------------+
//
//
// This structure could be created like this:
//  
//      Thing thing1, thing2, thing3;
//      CDListItem myThings;
//      cdlist_reset(&myThings);
//      cdlist_append(&myThings, &thing1.member3);
//      cdlist_append(&myThings, &thing2.member3);
//      cdlist_append(&myThings, &thing3.member3);
//
//
// To get the enclosing `Thing` from its CDListItem, use `enclosing()` from enclosing.h
//      Thing* thirdThing = enclosing(cdlist_idx(myThings, 2), Thing, member3);
// Or with foreach:
//      cdlist_foreach(listItem, myThings){
//          Thing* thing = enclosing(listItem, Thing, member3);
//          // Do stuff with "thing"
//      }



typedef struct CDListItem CDListItem;
struct CDListItem {
    CDListItem*   next;
    CDListItem*   prev;
};



// Resets the `list`, making prev/next point back to itself
void cdlist_reset(CDListItem* list);

// Checks if the `list` is empty, ie next/prev point to something other than itself
uint8_t cdlist_empty(CDListItem* list);

// Inserts an `entry` at the `idx`'th position of the `list`
// See also `cdlist_append`
void cdlist_insert(CDListItem* list, CDListItem* entry, uint8_t idx);

// Appends an `entry` to the back of the `list`
void cdlist_append(CDListItem* list, CDListItem* entry);

// Unlinks this `entry` from the list it belongs to
//  entry.prev & entry.next still point to its former neighbours
//  This makes it possible to remove items when iterating over a list, 
//  since we can still get the next item. Use `cdlist_reset` to reset
//  the entry after it has been unlinked
void cdlist_unlink(CDListItem* entry);

// Checks if an `entry` is part of a `list`
uint8_t cdlist_contains(CDListItem* list, CDListItem* entry);

// Gets the number of items in the `list`
int8_t cdlist_length(CDListItem* list);

// Gets the `item` at the `idx`'th position of the list
//  0-indexed
//  assert triggered if idx is longer than the length of the list
CDListItem* cdlist_idx(CDListItem* list, uint8_t idx);

// Iterates over all the items in the list
//  `item` is the iteration variable, and is of type `CDListItem*`
#define cdlist_foreach(item, list)          \
    for(CDListItem* item = (list)->next;    \
        item != (list);                     \
        item = item->next                   \
    )















