/** @file user/libthread/splay_tree.c
  * 
  * @brief the splay tree implementation
  * @author An Wu (anwu@andrew.cmu.edu)
  */

#include <simics.h>

#include <malloc.h>
#include <queue.h>

#include <splay_tree.h>

void st_splay(splay_tree st, st_node node); /* forward declaration */

splay_tree st_new(st_compare_fn fn) {
    splay_tree st = calloc(1, sizeof(struct splay_tree));
    if (st == NULL) return NULL;
    st->comp = fn;
    return st;
}

int st_empty(splay_tree st) {
    return (st->root == NULL);
}

void st_destroy(splay_tree st) {
    free(st);
}

/** @brief build a child-parent relationship
  *
  * @param child the child
  * @param parent the parent
  * @param left if 1, put child on the left, else right
  * @return Void
  */
void make_child(st_node child, st_node parent, int left) {
    if (child != NULL) {
        child->parent = parent;
    }
    if (parent != NULL) {
        if (left) {
            parent->left = child;
        }
        else {
            parent->right = child;
        }
    }
}

/** @brief build association for node and its great grandparent
  *
  * @param st the splay_tree
  * @param node the node
  * @param gg the great grandparent of node
  * @param grandparent the grandparent of node
  * @return Void
  */
void check_great_grandparent(splay_tree st, st_node node, st_node gg, 
                             st_node grandparent) {
    if (gg == NULL) {   /* grandparent is root */
        st->root = node;
        return;
    }
    else {  
        if (gg->left == grandparent) {
            make_child(node, gg, 1);
            st_splay(st, node);
            return;
        }
        else {
            make_child(node, gg, 0);
            st_splay(st, node);
            return;
        }
    }
}

/** @brief splay a node in a splay tree
  * 
  * @param st the splay_tree
  * @param node the node we want to splay
  * @return Void
  */
void st_splay(splay_tree st, st_node node) {
    st_node parent = node->parent;
    if (parent == NULL) return;
    st_node grandparent = parent->parent;
    
    if (grandparent == NULL) {      /* parent is root */
        /* make node root */
        node->parent = NULL;
        st->root = node;
        /* modify relation with parent */
        if (parent->right == node) {    /* node on the right */
            make_child(node->left, parent, 0);
            make_child(parent, node, 1);
        }
        else {                          /* node on the left */
            make_child(node->right, parent, 1);
            make_child(parent, node, 0);
        }
    }
    else {                          /* zig-zig and zig-zag */
        st_node gg = grandparent->parent;
        node->parent = gg;
        if (grandparent->left == parent) {  
            if (parent->left == node) {      /* left-left */
                make_child(parent->right, grandparent, 1);
                make_child(node->right, parent, 1);
                make_child(grandparent, parent, 0);
                make_child(parent, node, 0);
                check_great_grandparent(st, node, gg, grandparent);
                return;
            }
            else {                          /* left-right */
                make_child(node->right, grandparent, 1);
                make_child(node->left, parent, 0);
                make_child(parent, node, 1);
                make_child(grandparent, node, 0);
                check_great_grandparent(st, node, gg, grandparent);
                return;
            }
        }
        else { 
            if (parent->left == node) {     /* right-left */
                make_child(node->left, grandparent, 0);
                make_child(node->right, parent, 1);
                make_child(grandparent, node, 1);
                make_child(parent, node, 0);
                check_great_grandparent(st, node, gg, grandparent);
                return;
            }
            else {                          /* right-right */
                make_child(parent->left, grandparent, 0);
                make_child(node->left, parent, 0);
                make_child(grandparent, parent, 1);
                make_child(parent, node, 1);
                check_great_grandparent(st, node, gg, grandparent);
                return;
            }
        }
    }
}

/** @brief print the splay tree 
  *
  * @param st the splay tree
  * @return Void
  */
void print_st(splay_tree st) {
    queue q1 = queue_new();
    queue q2 = queue_new();

    if (st->root == NULL) {
        return;
    }
    else {
        enqueue(st->root, q1);
    }
    lprintf("Start print tree...\n");
    int count = 0;
    while (!queue_empty(q1)) {
        lprintf("level %d: ", count++);
        while (!queue_empty(q1)) {
            st_node n = dequeue(q1);
            lprintf("%d ", n->key);
            if (n->left != NULL)
                enqueue(n->left, q2);
            if (n->right != NULL)
                enqueue(n->right, q2);
        }
        lprintf("\n");
        while (!queue_empty(q2)) {
            enqueue(dequeue(q2), q1);
        }
    }

}

void st_insert(splay_tree st, st_key key, st_value value) {
    st_node node = calloc(1, sizeof(struct st_node));
    node->key = key;
    node->value = value;
    
    st_node x = st->root;
    if (x == NULL) {
        st->root = node;
    }
    else {      /* find the right place */
        while (1) {
            st_key x_key = x->key;
            int comp = st->comp(key, x_key);
            
            if (comp == 0) {        /* node exists. Update value */
                x->value = value;
                break;
            }
            else if (comp > 0) {    /* check right */
                st_node right = x->right;
                if (right == NULL) {
                    make_child(node, x, 0);
                    break;
                }
                else {
                    x = x->right;
                    continue;
                }
            }
            else {                  /* check left */
                st_node left = x->left;
                if (left == NULL) {
                    make_child(node, x, 1);
                    break;
                }
                else {
                    x = x->left;
                    continue;
                }
            }
        }
    }
    st_splay(st, node);
}

st_value st_lookup(splay_tree st, st_key key) {
    st_node node = st->root;
    while (node != NULL) {
        st_key node_key = node->key;
        int comp = st->comp(key, node_key);
        if (comp == 0) { 
            return node->value;
        }
        else if (comp < 0) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return NULL; /* not found */
}

st_value st_delete(splay_tree st, st_key key) {
    st_node node = st->root;
    while (node != NULL) {
        st_key node_key = node->key;
        int comp = st->comp(key, node_key);
        if (comp == 0) {
            st_splay(st, node); /* splay this node */
            st_value value = node->value;
            st_node left = node->left;
            st_node right = node->right;
            
            if (right == NULL) {
                if (left == NULL) {
                    st->root = NULL;
                }
                else {
                    st->root = left;
                    left->parent = NULL;
                }
            }
            else {
                if (left == NULL) {
                    st->root = right;
                    right->parent = NULL;
                }
                else {      /* use node's immediate successor as root */
                    st_node successor = right;
                    while (successor->left != NULL) {
                        successor = successor->left;
                    }
                    st->root = successor;
                    make_child(left, successor, 1);
                    if (successor != right) {   /* move successor up */
                        make_child(successor->right, successor->parent, 1);
                        make_child(right, successor, 0);
                    }
                    successor->parent = NULL;                       
                }
            }
            free(node);
            return value;
        }
        else if (comp < 0) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return NULL;  /* not found */
}
