/** @file kern/data_structure/st_hash.c
 *  
 *  @brief static hash table implementation
 *         static means that entry and nodes are pre-allocated
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <simics.h>
#include <st_hash.h>
#include <malloc.h>
#include <reporter.h>
#include <string.h>
#include <if_flag.h>

static char *tag = "st_hash";

sht_t *sht_new(skey_compare_fn fn) {
    
    report_misc(tag, "st_hash_new"); 
     
    /* alloc structure */
    sht_t *t = calloc(1, sizeof(sht_t));
    if (t == NULL) {
        report_error(tag, "st_hash_new cannot allocate memory");
        return NULL;
    }
    
    t->fn = fn;
    t->entry_count = 0;

    /* prealloc queues */
    int i;
    for (i = 0; i < ST_TABLE_SIZE; i++) {
        (t->table)[i] = st_queue_new();
        if ((t->table)[i] == NULL) {
            report_error(tag, "st_hash_new cannot allocate memory");
            while (i > 0) {
                i--;
                st_queue_destroy((t->table)[i]);
            }
            return NULL;
        }
    }   
    
    return t;
}

int sht_traverse(sht_t *t, int (*fn)(sht_entry_t *)) {
   
    report_misc(tag, "sht_traverse sht=%p", t); 
    if (t == NULL) {
        report_error(tag, "st_hash_traverse NULL input");
        return -1;
    }
    
    if (t->entry_count == 0)
        return 1;

    int i;
    for (i = 0; i < ST_TABLE_SIZE; i++) {
        if (((t->table)[i] != NULL) && 
            (!st_queue_empty((t->table)[i]))) {
            if (st_queue_traverse((t->table)[i], (int(*)(void *))fn) == 0) {
                return 0;
            }
        }
    }
    
    return 1; 
}

int sht_traverse_all(sht_t *t, int (*fn)(sht_entry_t *)) {
    
    report_misc(tag, "sht_traverse_all sht=%p", t);    
    if (t == NULL) {
        report_error(tag, "st_hash_traverse_all NULL input sht=%p", t);
        return -1;
    }

    if (t->entry_count == 0)
        return 1;

    int i;
    int result = 1;
    for (i = 0; i < ST_TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            result &= st_queue_traverse_all((t->table)[i], (int(*)(void *))fn);
        }
    }
    
    return result; 
}

st_hash_value sht_find(sht_t *t, int (*fn)(sht_entry_t *)) {
    
    report_misc(tag, "sht_find sht=%p", t);
    if (t == NULL) {
        report_error(tag, "st_hash_find NULL input sht=%p", t);
        return NULL;
    }

    if (t->entry_count == 0)
        return NULL;

    int i;
    void *entry;
    for (i = 0; i < ST_TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            if ((entry = st_queue_find_true((t->table)[i], (int(*)(void *))fn))
                    != NULL) {
                return ((sht_entry_t *)entry)->value;
            }
        }
    }
    
    return NULL; 
}

void sht_destroy(sht_t *t) {
    
    report_misc(tag, "sht_destroy sht=%p", t);
    if (t == NULL) {
        report_error(tag, "st_hash_destroy NULL input sht=%p", t);
        return;
    }

    int i;
    for (i = 0; i < ST_TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            st_queue_destroy((t->table)[i]);
            (t->table)[i] = NULL;
        }
    }
    
    free(t);
    return;
}

int sht_empty(sht_t *t) {
    return (t->entry_count == 0);
}

/** @brief hash function taken from stack overflow
 *  http://stackoverflow.com/questions/664014/
 *  what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
 */
int st_hash_get_index(sht_t *t, st_hash_key key) {

    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key);
    key = key % ST_TABLE_SIZE;
    return (key > 0) ? key : -key;
}

int sht_insert(sht_entry_t *new_entry, struct st_node *n, sht_t *t, 
               st_hash_key key, st_hash_value value) {
    if (t == NULL) {
        report_error(tag, "st_hash_insert, t is NULL sht=%p", t);
        return -1;
    }
    if (new_entry == NULL){
        report_error(tag, "st_hash_insert entry ptr is NULL sht=%p", t);
        return -1;
    }
    if (n == NULL){
        report_error(tag, "st_hash_insert st_node ptr is NULL sht=%p", t);
        return -1;
    }   

    int index = st_hash_get_index(t, key);
    
    report_misc(tag, "sht_insert, key %p, value %p, index %d, sht=%p", 
                (void *)key, (void *)value, index, t);

    new_entry->key = key;
    new_entry->value = value;   
    
    if ((t->table)[index] == NULL) {
        report_error(tag, "st_hash_insert table has a NULL slot sht=%p", t);
        return -1;
    }

    /* sanity check */
    /* try to find the entry with the same key */
    sht_entry_t *old_entry;
    if ((old_entry = st_queue_find((int (*)(void *, void *))t->fn, 
                                (void *)new_entry, 
                                (t->table)[index])) == NULL) {
        st_enqueue(n, (void *)new_entry, (t->table)[index]);
        t->entry_count++;
        return 0;
    }
    else {
        /* sanity check */
        if (old_entry->value == new_entry->value) {
            report_error(tag, 
        "st_hash_insert, same value %p for key 0x%x exists in table sht=%p", 
                (void *)value, (int)key, t);
            return -1;
        }
        else {
            report_warning(tag, "st_hash_insert, value is updated sht=%p", t);

            old_entry->value = new_entry->value;
            return 0;
        }
    }
}

st_hash_value sht_delete(sht_t *t, st_hash_key key) {
    
    if (t == NULL) {
        report_error(tag, "st_hash_delete, input is NULL sht=%p", t);
        return NULL;
    }
    
    if (t->entry_count == 0) {
        report_error(tag, "st_hash_delete, st_hash table is empty");
        return NULL;
    }

    int index = st_hash_get_index(t, key);
    
    report_misc(tag, "sht_delete, key %p, index %d, sht=%p", 
                (void *)key, index, t);
  
    if ((t->table)[index] == NULL) {
        report_error(tag, "st_hash_delete, table has a empty slot sht=%p", 
                     t);
        return NULL;
    }   
    
    sht_entry_t new_entry;
    new_entry.key = key;
   
    st_hash_value result; 
    sht_entry_t *old_entry;

    if ((old_entry = (sht_entry_t *)st_queue_delete((int (*)(void *, void *))t->fn, 
                            (void *)(&new_entry), (t->table)[index])) == NULL) {
        report_error(tag, 
            "st_hash_delete, table has no such key sht=%p", t);
        return NULL;
    } 

    t->entry_count--;
    result = old_entry->value;
    return result;
}

st_hash_value sht_lookup(sht_t *t, st_hash_key key) {
    if (t == NULL) {
        report_error(tag, "st_hash_lookup, input is NULL sht=%p", t);
        return NULL;
    }
    
    if (t->entry_count == 0) {
        report_warning(tag, "st_hash_lookup, st_hash table is empty");
        return NULL;
    }
    
    int index = st_hash_get_index(t, key);
    
    report_misc(tag, "sht_lookup, key %p, index %d, sht=%p", 
                (void *)key, index, t);

    if ((t->table)[index] == NULL) {
        report_warning(tag, 
            "st_hash_lookup, table has a empty slot sht=%p", t);
        return NULL;
    }   
       
    sht_entry_t new_entry;
    new_entry.key = key;
    
    sht_entry_t *old_entry;
    /* try to find the entry with the same key */
    if ((old_entry = st_queue_find((int (*)(void *, void *))t->fn, 
                                (void *)(&new_entry), 
                                (t->table)[index])) == NULL) {
        report_warning(tag, "st_hash_lookup, table has no such key 0x%x sht=%p", 
                        (int)key, t);
        return NULL;
    }
    else {
        return old_entry->value;
    }
}

int sht_size(sht_t *t) {
    return t->entry_count;
}
