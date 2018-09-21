#include "cdlist.h"

#include <stdlib.h>
#include <stdint.h>

#include "../util/assert.h"

    
void cdlist_reset(CDListItem* list){
    list->next = list;
    list->prev = list;
}

uint8_t cdlist_empty(CDListItem* list){
    return (
        list->next == list && 
        list->prev == list
    );
}
    
void cdlist_insert(CDListItem* list, CDListItem* entry, uint8_t idx){
    CDListItem* l = cdlist_idx(list, idx)->prev;
    entry->next = l->next;
    entry->prev = l;
    l->next->prev = entry;
    l->next = entry;
}

void cdlist_append(CDListItem* list, CDListItem* entry){
    entry->next = list;
    entry->prev = list->prev;
    entry->prev->next = entry;
    list->prev = entry;
}    
    
void cdlist_unlink(CDListItem* entry){
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}


uint8_t cdlist_contains(CDListItem* list, CDListItem* entry){
    cdlist_foreach(item, list){
        if(item == entry){
            return 1;
        }
    }
    return 0;
}

int8_t cdlist_length(CDListItem* list){
    int8_t len = 0;
    cdlist_foreach(item, list){
        os_assert(item, "cdlist_length foreach: list has nullptrs");
        len++;
    }
    return len;
}

CDListItem* cdlist_idx(CDListItem* list, uint8_t idx){
    os_assert(idx < cdlist_length(list), 
        "cdlist_idx: out of bounds (idx:%d, len:%d)", idx, cdlist_length(list));
    CDListItem* l = list->next;
    for(int8_t i = 0; i < idx; i++){
        l = l->next;
    }
    return l;
}















