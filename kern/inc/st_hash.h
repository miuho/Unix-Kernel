/** @file kern/inc/st_hash.h
 *  @brief This file defines static hash table interface.
 *         Static means that entry are pre-allocated by caller
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_ST_HASH_H_
#define _KERN_INC_ST_HASH_H_

#include <st_queue.h>

#define ST_TABLE_SIZE 257

/* the key type */
typedef unsigned long st_hash_key;

/* the value type */
typedef void *st_hash_value;

/* an entry in the static hash table (key-value) */
typedef struct sht_entry {
    st_hash_key key;
    st_hash_value value;
} sht_entry_t;

/* a general compare function interface for key */
typedef int (*skey_compare_fn)(sht_entry_t *, sht_entry_t *);

/* the hash table structure */
typedef struct sht {
    /* the number of entries currently in the hash table */
    int entry_count;

    /* the compare function of the static hash table */
    skey_compare_fn fn;

    /* the static hash table (as an array linked-list) */
    st_queue table[ST_TABLE_SIZE];
} sht_t;

/** @brief create a new hash table
 *
 *  @param key_compare_fn the compare function
 *  @return address of the new table. NULL on error
 */
sht_t *sht_new(skey_compare_fn fn);

/** @brief traverse a hash table and apply a function on each entry to
 *         test a condition (can be short-circuited)
 *   
 *  @param t the hash table pointer
 *  @param fn the function we want to apply
 *  @return 1 if fn applied to all entry return 1, 0 if there is any 0, 
 *          -1 on error
 */
int sht_traverse(sht_t *t, int (*fn)(sht_entry_t *));

/** @brief traverse all entries of a hash table with a function
 *
 *  @param t the hash table pointer
 *  @param fn the function we want to apply
 *  @return 1 if fn applied to all entry return 1, 0 if there is any 0,
 *          -1 on error
 */
int sht_traverse_all(sht_t *t, int (*fn)(sht_entry_t *));

/** @brief find a value in the hash table by applying fn to values
 *
 *  @param t the hash table pointer
 *  @param fn the "filter" function
 *  @return the hash value if found, NULL if not found or error
 */
st_hash_value sht_find(sht_t *t, int (*fn)(sht_entry_t *));

/** @brief destroy a hash table
 *
 *  @param t the hash table
 *  @return Void
 */
void sht_destroy(sht_t *t);

/** @brief check if a hash table is empty
 *
 *  @param t the hash table
 *  @return 1 if empty, 0 if not, -1 on error
 */
int sht_empty(sht_t *t);

/** @brief insert a key value pair into the hash table
 *
 *  @param new_entry pointer to the new entry
 *  @param n pointer to the st_node
 *  @param t the hash table
 *  @param key the key 
 *  @param value the value
 *  @return 0 on success, -1 on error
 */
int sht_insert(sht_entry_t *new_entry, struct st_node *n, sht_t *t, 
               st_hash_key key, st_hash_value value);

/** @brief delete an entry in the hash table
 *
 *  @param key the key of the entry we want to delete
 *  @return the hash_value of deleted pair on success, NULL on error
 */
st_hash_value sht_delete(sht_t *t, st_hash_key key);

/** @brief look up an entry in the hash table
 *
 *  @param t the hash table
 *  @param key the key of the entry we want to lookup
 *  @return hash value of the entry if found, NULL if not found or error
 */
st_hash_value sht_lookup(sht_t *t, st_hash_key key);

/** @brief check the size of a hash table
 *
 *  @param t the hash table
 *  @return the size of the hash table, -1 on error
 */
int sht_size(sht_t *t);

#endif
