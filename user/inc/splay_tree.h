/** @file user/inc/splay_tree.h
 *  @brief a splay tree implementation
 *
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _SPLAY_TREE_H
#define _SPLAY_TREE_H

typedef int st_key;     /* key type for splay tree nodes */
typedef void *st_value; /* value type for splay tree nodes */

typedef int (*st_compare_fn)(st_key, st_key);   /* compare function on key */
typedef struct st_node *st_node;        /* make st_node a pointer */

/* splay tree node struct */
struct st_node {
    st_key key;
    st_value value;
    st_node parent;
    st_node left;
    st_node right;
};

/* splay tree struct */
typedef struct splay_tree {
    st_node root;
    st_compare_fn comp;
} *splay_tree;

/** @brief construct a new splay tree
  *
  * @param fn the compare function for keys
  * @return the pointer to the splay tree. NULL if fail
  */
splay_tree st_new(st_compare_fn fn);

/** @brief check if a splay tree is empty
 *
 *  @param st the splay tree
 *  @return 1 if empty, 0 if not
 */
int st_empty(splay_tree st);

/** @brief destroy a splay tree (and free its resource
  *
  * @param st the splay_tree
  * @return Void
  */
void st_destroy(splay_tree st);

/** @brief insert a key value pair into the splay tree
  *
  * @param st the splay_tree
  * @param key the key
  * @param value the value
  * @return Void
  */
void st_insert(splay_tree st, st_key key, st_value value);

/** @brief lookup a key in the splay tree 
  * 
  * @param st the splay_tree
  * @param key the key we want to look up
  * @return the value binded to key. NULL if st invalid or key not exist
  */
st_value st_lookup(splay_tree st, st_key key);

/** @brief delete a node in the splay tree 
  * 
  * @param st the splay_tree
  * @param key the key the node is associated to 
  * @return the value binded to key. NULL if st invalid or key not exist
  */
st_value st_delete(splay_tree, st_key);

#endif /* !_SPLAY_TREE_H */


