#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MIN_DEGREE 2 // Also used to refer to minimum nyumber of children
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MIN_KEYS (MIN_DEGREE - 1) 
#define MAX_CHILDREN (MIN_DEGREE * 2)

typedef struct StudentRecord{
    int id;
    char name[100];
    char programme[100];
    float mark;
} StudentRecord;

typedef struct BTreeNode {
    int num_keys; // Number of keys currently in the node
    StudentRecord keys[MAX_KEYS]; //Array of keys with struct StudentRecord
    struct BTreeNode *children[MAX_CHILDREN]; // Array of pointers to other child nodes
    bool is_leaf;
} BTreeNode;


// Function to create a new node
struct BTreeNode *createNode(bool is_leaf) {
    struct BTreeNode *newNode = (struct BTreeNode *)malloc(sizeof(struct BTreeNode));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    newNode->num_keys = 0;
    newNode->is_leaf = is_leaf;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

// Function to split a full child node
void splitChild(struct BTreeNode *parent, int index) {
    struct BTreeNode *child = parent->children[index];
    struct BTreeNode *newNode = createNode(child->is_leaf);
    
    newNode->num_keys = MIN_KEYS;
    
    // Move keys and children to the new node
    for (int i = 0; i < MIN_KEYS; i++) {
        newNode->keys[i] = child->keys[i + MIN_DEGREE];
    }
    
    if (!child->is_leaf) {
        for (int i = 0; i < MIN_DEGREE; i++) {
            newNode->children[i] = child->children[i + MIN_DEGREE];
        }
    }
    
    child->num_keys = MIN_KEYS;
    
    // Shift parent's children to make space for the new node
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    
    parent->children[index + 1] = newNode;
    
    // Shift parent's keys to insert the middle key from the child
    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    
    parent->keys[index] = child->keys[MIN_KEYS];
    parent->num_keys++;
}

// Function to insert a key into a non-full node
void insertNonFull(struct BTreeNode *node, StudentRecord rec) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Insert key into the sorted order
        while (i >= 0 && node->keys[i].id > rec.id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = rec;
        node->num_keys++;
    } else {
        // Find the child to insert the key
        while (i >= 0 && node->keys[i].id > rec.id) {
            i--;
        }
        i++;
        
        if (node->children[i]->num_keys == (MIN_KEYS + 1)) {
            // Split child if it's full
            splitChild(node, i);
            
            // Determine which of the two children is the new one
            if (node->keys[i].id < rec.id) {
                i++;
            }
        }
        insertNonFull(node->children[i], rec);
    }
}

// Function to insert a key into the B-tree
void insert(struct BTreeNode **root, StudentRecord key) {
    struct BTreeNode *node = *root;

    if (node == NULL) {
        // Create a new root node
        *root = createNode(true);
        (*root)->keys[0] = key;
        (*root)->num_keys = 1;
    } else {
        if (node->num_keys > MAX_KEYS) {
            // Split the root if it's full
            struct BTreeNode *new_root = createNode(false);
            new_root->children[0] = node;
            splitChild(new_root, 0);
            *root = new_root;
        }
        insertNonFull(*root, key);
    }
}

// Function to traverse and print the B-tree in-order
void traverse(struct BTreeNode *root) {
    if (root != NULL) {
        int i;
        for (i = 0; i < root->num_keys; i++) {
            traverse(root->children[i]);
            printf("Student ID: %d, Name: %s, Programme: %s, Marks: %.1f\n", root->keys[i].id, root->keys[i].name, root->keys[i].programme, root->keys[i].mark);
        }
        traverse(root->children[i]);
    }
}



int main(){
    BTreeNode *root = NULL;
    StudentRecord x = {2501103, "Kevin Koh", "Applied AI", 56.0};
    insert(&root, x);
    StudentRecord y = {2501103, "Klevin Koh", "Applied AlI", 53.0};
    insert(&root, y);

    StudentRecord z = {2501100, "Sakon Madek", "Genna Talia", 58.59};
    insert(&root, z);
    
    printf("In-order traversal of the B-tree: ");
    traverse(root);
    printf("\n");

    return 0;


    // char operation_input[200];
    // int char_limit_for_inp = 200;
    // BTreeNode *root = NULL;
    // printf("Input your operation(OPEN, SHOW ALL, INSERT, QUERY, UPDATE, DELETE, SAVE): ");
    

    
}