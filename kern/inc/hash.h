/** @file kern/inc/hash.h
 *  @brief This file defines hash table interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */


#ifndef _KERN_INC_HASH_H_
#define _KERN_INC_HASH_H_

#include <queue.h>
#include <mutex.h>

#define TABLE_SIZE 57

/* the key type */
typedef unsigned long hash_key;

/* the value type */
typedef void *hash_value;

/* an entry in the hash table (key-value) */
typedef struct ht_entry {
    hash_key key;
    hash_value value;
} ht_entry_t;

/* a general compare function interface for key */
typedef int (*key_compare_fn)(ht_entry_t *, ht_entry_t *);

/* the hash table structure */
typedef struct ht {
    /* the lock */
    mutex_t mp;
    
    /* the number of entries currently in the hash table */
    int entry_count;

    /* the compare function of the hash table */
    key_compare_fn fn;

    /* the hash table (as an array linked-list) */
    queue table[TABLE_SIZE];
} ht_t;

/** @brief create a new hash table
 *
 *  @param key_compare_fn the compare function
 *  @return address of the new table. NULL on error
 */
ht_t *ht_new(key_compare_fn fn);

/** @brief traverse a hash table and apply a function on each entry to
 *         test a condition (can be short-circuited)
 *   
 *  @param t the hash table pointer
 *  @param fn the function we want to apply
 *  @return 1 if fn applied to all entry return 1, 0 if there is any 0, 
 *          -1 on error
 */
int ht_traverse(ht_t *t, int (*fn)(ht_entry_t *));

/** @brief traverse all entries of a hash table with a function
 *
 *  @param t the hash table pointer
 *  @param fn the function we want to apply
 *  @return 1 if fn applied to all entry return 1, 0 if there is any 0,
 *          -1 on error
 */
int ht_traverse_all(ht_t *t, int (*fn)(ht_entry_t *));

/** @brief find a value in the hash table by applying fn to values
 *
 *  @param t the hash table pointer
 *  @param fn the "filter" function
 *  @return the hash value if found, NULL if not found or error
 */
hash_value ht_find(ht_t *t, int (*fn)(ht_entry_t *));

/** @brief destroy a hash table
 *
 *  @param t the hash table
 *  @return Void
 */
void ht_destroy(ht_t *t);

/** @brief check if a hash table is empty
 *
 *  @param t the hash table
 *  @return 1 if empty, 0 if not, -1 on error
 */
int ht_empty(ht_t *t);

/** @brief insert a key value pair into the hash table
 *
 *  @param t the hash table
 *  @param key the key 
 *  @param value the value
 *  @return 0 on success, -1 on error
 */
int ht_insert(ht_t *t, hash_key key, hash_value value);

/** @brief delete an entry in the hash table
 *
 *  @param key the key of the entry we want to delete
 *  @return the hash_value of deleted pair on success, NULL on error
 */
hash_value ht_delete(ht_t *t, hash_key key);

/** @brief look up an entry in the hash table
 *
 *  @param t the hash table
 *  @param key the key of the entry we want to lookup
 *  @return hash value of the entry if found, NULL if not found or error
 */
hash_value ht_lookup(ht_t *t, hash_key key);

/** @brief check the size of a hash table
 *
 *  @param t the hash table
 *  @return the size of the hash table, -1 on error
 */
int ht_size(ht_t *t);

#endif
