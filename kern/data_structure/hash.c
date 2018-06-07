/** @file kern/hash.c
 *
 *  @brief hash table implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <simics.h>
#include <hash.h>
#include <malloc.h>
#include <reporter.h>
#include <if_flag.h>

static char *tag = "hash";

ht_t *ht_new(key_compare_fn fn) {
    
    report_misc(tag, "hash_new"); 
     
    /* alloc structure */
    ht_t *t = calloc(1, sizeof(ht_t));
    if (t == NULL) {
        report_error(tag, "hash_new cannot allocate memory");
        return NULL;
    }
    
    t->fn = fn;
    t->entry_count = 0;

    /* init lock */ 
    if (mutex_init(&(t->mp)) < 0) {
        report_error(tag, "hash_new cannot init mutex");

        free(t);
        return NULL;
    }
    
    return t;
}

int ht_traverse(ht_t *t, int (*fn)(ht_entry_t *)) {
   
    report_misc(tag, "ht_traverse ht=%p", t); 
    if (t == NULL) {
        report_error(tag, "hash_traverse NULL input");
        return -1;
    }
    
    if (t->entry_count == 0)
        return 1;

    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            if (queue_traverse((t->table)[i], (isTrueElem)fn) == 0) {
                return 0;
            }
        }
    }
    
    return 1; 
}

int ht_traverse_all(ht_t *t, int (*fn)(ht_entry_t *)) {
    
    report_misc(tag, "ht_traverse_all ht=%p", t);    
    if (t == NULL) {
        report_error(tag, "hash_traverse_all NULL input ht=%p", t);
        return -1;
    }

    if (t->entry_count == 0)
        return 1;

    int i;
    int result = 1;
    for (i = 0; i < TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            result &= queue_traverse_all((t->table)[i], (isTrueElem)fn);
        }
    }
    
    return result; 
}

hash_value ht_find(ht_t *t, int (*fn)(ht_entry_t *)) {
    
    report_misc(tag, "ht_find ht=%p", t);
    if (t == NULL) {
        report_error(tag, "hash_find NULL input ht=%p", t);
        return NULL;
    }

    if (t->entry_count == 0)
        return NULL;

    int i;
    void *entry;
    for (i = 0; i < TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            if ((entry = queue_find_true((t->table)[i], (isTrueElem)fn)) 
                    != NULL) {
                return ((ht_entry_t *)entry)->value;
            }
        }
    }
    
    return NULL; 
}

void ht_destroy(ht_t *t) {
    
    report_misc(tag, "ht_destroy ht=%p", t);
    if (t == NULL) {
        report_error(tag, "hash_destroy NULL input ht=%p", t);
        return;
    }

    int i;
    ht_entry_t *entry;
    for (i = 0; i < TABLE_SIZE; i++) {
        if ((t->table)[i] != NULL) {
            while (!queue_empty((t->table)[i])) {
                entry = (ht_entry_t *)dequeue((t->table)[i]);
                free(entry);
            }
            queue_destroy((t->table)[i]);
            (t->table)[i] = NULL;
        }
    }
    
    mutex_destroy(&(t->mp));
    free(t);
    return;
}

int ht_empty(ht_t *t) {
    return (t->entry_count == 0);
}

/** @brief hash function taken from stack overflow
 *  http://stackoverflow.com/questions/664014/
 *  what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
 */
int hash_get_index(ht_t *t, hash_key key) {

    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key);
    key = key % TABLE_SIZE;
    return (key > 0) ? key : -key;
}

int ht_insert(ht_t *t, hash_key key, hash_value value) {
    if (t == NULL) {
        report_error(tag, "hash_insert, input is NULL ht=%p", t);
        return -1;
    }   
    
    int index = hash_get_index(t, key);
    
    report_misc(tag, "ht_insert, key %p, value %p, index %d, ht=%p", 
                (void *)key, (void *)value, index, t);

    ht_entry_t *new_entry = calloc(1, sizeof(ht_entry_t));
    if (new_entry == NULL){
        report_error(tag, "hash_insert cannot allocate memory ht=%p", t);
        return -1;
    }
    new_entry->key = key;
    new_entry->value = value;   
    
    if ((t->table)[index] == NULL) {
        (t->table)[index] = queue_new();
        if ((t->table)[index] == NULL) {
            report_error(tag, "hash_insert cannot allocate memory ht=%p", t);

            free(new_entry);
            return -1;
        }
        
        enqueue((void *)new_entry, (t->table)[index]);
        t->entry_count++;
        return 0;
    }

    ht_entry_t *old_entry;

    /* try to find the entry with the same key */
    if ((old_entry = queue_find((int (*)(void *, void *))t->fn, 
                                (void *)new_entry, 
                                (t->table)[index])) == NULL) {
        enqueue((void *)new_entry, (t->table)[index]);
        t->entry_count++;
        return 0;
    }
    else {
        if (old_entry->value == new_entry->value) {
            report_error(tag, 
            "hash_insert, same value %p for key 0x%x exists in table ht=%p", 
                (void *)value, (int)key, t);
            free(new_entry);
            return -1;
        }
        else {
            report_warning(tag, "hash_insert, value is updated ht=%p", t);

            old_entry->value = new_entry->value;
            free(new_entry);
            return 0;
        }
    }
}

hash_value ht_delete(ht_t *t, hash_key key) {
    
    if (t == NULL) {
        report_error(tag, "hash_delete, input is NULL ht=%p", t);
        return NULL;
    }
    
    if (t->entry_count == 0) {
        report_error(tag, "hash_delete, hash table is empty");
        return NULL;
    }

    int index = hash_get_index(t, key);
    
    report_misc(tag, "ht_delete, key %p, index %d, ht=%p", 
                (void *)key, index, t);
  
    if ((t->table)[index] == NULL) {
        report_error(tag, "hash_delete, table has no such key 0x%x ht=%p", 
                (int)key, t);
        return NULL;
    }   
    
    ht_entry_t new_entry;
    new_entry.key = key;
   
    hash_value result; 
    ht_entry_t *old_entry;

    if ((old_entry = (ht_entry_t *)queue_delete((int (*)(void *, void *))t->fn, 
                        (void *)(&new_entry), (t->table)[index])) == NULL) {
        report_error(tag, 
            "hash_delete, table has no such key ht=%p", t);
        return NULL;
    } 
        
    if (queue_empty((t->table)[index])) {
        queue_destroy((t->table)[index]);
        (t->table)[index] = NULL;
    }

    t->entry_count--;
    result = old_entry->value;
    free(old_entry);
    return result;
}

hash_value ht_lookup(ht_t *t, hash_key key) {
    if (t == NULL) {
        report_error(tag, "hash_lookup, input is NULL ht=%p", t);
        return NULL;
    }
    
    if (t->entry_count == 0) {
        report_warning(tag, "hash_lookup, hash table is empty");
        return NULL;
    }
    
    int index = hash_get_index(t, key);
    
    report_misc(tag, "ht_lookup, key %p, index %d, ht=%p", 
                (void *)key, index, t);

    if ((t->table)[index] == NULL) {
        report_warning(tag, 
                "hash_lookup, table has no such key 0x%x ht=%p", (int)key, t);
        return NULL;
    }   
       
    ht_entry_t new_entry;
    new_entry.key = key;
    
    ht_entry_t *old_entry;

    /* try to find the entry with the same key */
    if ((old_entry = queue_find((int (*)(void *, void *))t->fn, 
                                (void *)(&new_entry), 
                                (t->table)[index])) == NULL) {
        report_warning(tag, "hash_lookup, table has no such key 0x%x ht=%p", 
                        (int)key, t);
        return NULL;
    }
    else {
        return old_entry->value;
    }
}

int ht_size(ht_t *t) {
    return t->entry_count;
}
