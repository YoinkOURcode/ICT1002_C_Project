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
    StudentRecord *keys[MAX_KEYS]; //Array of pointers to keys with struct StudentRecord
    struct BTreeNode *children[MAX_CHILDREN]; // Array of pointers to other child nodes
    bool is_leaf;
} BTreeNode;


// Function to create a new node
BTreeNode *createNode(bool is_leaf) {
    BTreeNode *newNode = (struct BTreeNode *)malloc(sizeof(BTreeNode));
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
void splitChild(BTreeNode *parent, int index) {
    BTreeNode *child = parent->children[index];
    BTreeNode *newNode = createNode(child->is_leaf);
    
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
void insertNonFull(BTreeNode *node, StudentRecord* rec) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Insert key into the sorted order
        while (i >= 0 && node->keys[i]->id > rec->id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = rec;
        node->num_keys++;
    } else {
        // Find the child to insert the key
        while (i >= 0 && node->keys[i]->id > rec->id) {
            i--;
        }
        i++;
        
        if (node->children[i]->num_keys == MAX_KEYS) {
            // Split child if it's full
            splitChild(node, i);
            
            // Determine which of the two children is the new one
            if (node->keys[i]->id < rec->id) {
                i++;
            }
        }
        insertNonFull(node->children[i], rec);
    }
}

// Function to insert a key into the B-tree
void insert(BTreeNode **root, StudentRecord *key) {
    BTreeNode *node = *root;

    if (node == NULL) {
        // Create a new root node
        *root = createNode(true);
        (*root)->keys[0] = key;
        (*root)->num_keys = 1;
    } else {
        if (node->num_keys == MAX_KEYS) {
            // Split the root if it's full
            struct BTreeNode *new_root = createNode(false);
            new_root->children[0] = node;
            splitChild(new_root, 0);
            *root = new_root;
        }
        insertNonFull(*root, key);
    }
}


StudentRecord* searchIndex(BTreeNode *root,int search_index){
    if (root){
        int nth_child;//determines which child we continue looking in depending on where we get a value where our search key is less than our node key
        for (nth_child = 0; nth_child < root->num_keys;nth_child++){
            if (root->keys[nth_child]->id == search_index){
                StudentRecord* p_record = root->keys[nth_child];
                return p_record;
            }
            else if (search_index < root->keys[nth_child]->id){
                break;
            }
        }
        searchIndex(root->children[nth_child], search_index);

    }
    else{
        printf("Not found!");
    }
}

void showAll(BTreeNode *root) {
    if (root != NULL) {
        int i;
        for (i = 0; i < root->num_keys; i++) {
            showAll(root->children[i]);
            printf("%d\t%s\t%s\t%.1f\n", root->keys[i]->id, root->keys[i]->name,root->keys[i]->programme,root->keys[i]->mark);
        }
        showAll(root->children[i]);
    }
}

void updateStudentRecord(BTreeNode *root, int search_index,char *field, char *value){
    StudentRecord * p_record = searchIndex(root, search_index);
    if (p_record){
        if (strcmp(field, "mark") == 0) {
            p_record->mark = (float)atof(value);
        }
        // if field is 'name', update the name
        else if (strcmp(field, "name") == 0) {
            strcpy(p_record->name, value);
        }
        // if field is 'programme', update the programme
        else if (strcmp(field, "programme") == 0) {
            strcpy(p_record->programme, value);
        }
        printf("The record with ID=%d is successfully updated.\n", search_index);
    }
    else{
        printf("Update went wrong");
    }

}

void createAndInsert(BTreeNode **root,
                     int id,
                     const char *name,
                     const char *programme,
                     float mark)
{
    StudentRecord *newRec = malloc(sizeof(StudentRecord));
    if (!newRec) {
        printf("Memory allocation failed.\n");
        return;
    }

    newRec->id = id;
    strncpy(newRec->name, name, sizeof(newRec->name)-1);
    newRec->name[sizeof(newRec->name)-1] = '\0';

    strncpy(newRec->programme, programme, sizeof(newRec->programme)-1);
    newRec->programme[sizeof(newRec->programme)-1] = '\0';

    newRec->mark = mark;

    insert(root, newRec);

}

int main(){
    BTreeNode *root = NULL;


    createAndInsert(&root,
                2502841,
                "Alicia Tan",
                "Computer Science",
                72.5);

    createAndInsert(&root,
                    2509174,
                    "Marcus Lim",
                    "Information Security",
                    64.0);

    createAndInsert(&root,
                    2505532,
                    "Samantha Ong",
                    "Data Analytics",
                    81.0);

    createAndInsert(&root,
                    2503328,
                    "Rahul Nair",
                    "Software Engineering",
                    49.5);

    createAndInsert(&root,
                    2507769,
                    "Chloe Wong",
                    "Business Analytics",
                    90.0);

    createAndInsert(&root,
                    2504417,
                    "Nicholas Lee",
                    "Applied AI",
                    58.0);

    createAndInsert(&root,
                    2506355,
                    "Emily Chan",
                    "Cybersecurity",
                    73.0);

    char op[100];
    int id;
    searchIndex(root,2506358 );



    // while (1) {
    //     printf("\nEnter your command:");
    //     fgets(op, sizeof(op), stdin);
    //     op[strcspn(op, "\n")] = 0;

    //     // Lower user input
    //     for (int i = 0; op[i]; i++) {
    //         op[i] = tolower(op[i]);
    //     }

    //     // OPEN
    //     if (strcmp(op, "open") == 0) {
            
    //     }
    //     // SHOW ALL
    //     else if (strcmp(op, "show all") == 0) {
    //         int found = -1;
    //         printf("Here are all the records found in the table \"%s\"\n", tablename);
    //         ShowAll(students, student_count, found);
    //     }
    //     // SHOW ALL SORTED
    //     else if (strstr(op, "show all sort") != NULL) {
    //         char sortby[10];
    //         char order[10];
    //         if (sscanf(op, "show all sort by %s %s", sortby, order) == 2) {
    //             ShowSorted(students, student_count, sortby, order, tablename);
    //         }
    //         else {
    //             printf("Follow this format to sort the data: SHOW ALL SORT BY ID/MARK ASC/DESC.\n");
    //         }
    //     }
    //     // INSERT 
    //     else if (strstr(op, "insert") != NULL) {
    //         if (sscanf(op, "insert id=%d", &id) == 1) {
    //             Insert(students, id, &student_count);
    //         }
    //         else {
    //             printf("Follow this format to insert new data: INSERT ID=<ID NUMBER>.\n");
    //         }
    //     }
    //     // QUERY
    //     else if (strstr(op, "query") != NULL) {
    //         if (sscanf(op, "query id=%d", &id) == 1) {
    //             Query(students, student_count, id);
    //         }
    //         else {
    //             printf("Follow this format to make a query: QUERY ID=<ID NUMBER>.\n");
    //         }
    //     }
    //     // UPDATE
    //     else if (strstr(op, "update") != NULL) {
    //         char field[100];
    //         char value[100];
    //         if (sscanf(op, "update id=%d %[^=]=%[^\n]", &id, field, value) == 3) {
    //             Update(students, student_count, id, field, value);
    //         }
    //         else {
    //             printf("Follow this format to update: UPDATE ID=<ID Number> <Field>=<Value>.\nExample: UPDATE ID=2801234 MARK=98.7\n.");
    //         }
    //     }
    //     // DELETE
    //     else if (strstr(op, "delete") != NULL) {
    //         if (sscanf(op, "delete id=%d", &id) == 1) {
    //             Delete(students, &student_count, id);
    //         }
    //         else {
    //             printf("Follow this format to delete data: DELETE ID=<ID NUMBER>.\n");
    //         }
    //     }
    //     // SAVE
    //     else if (strcmp(op, "save") == 0) {
    //         Save(students, student_count, filename);
    //     }
    //     // SUMMARY
    //     else if (strcmp(op, "show summary") == 0) {
    //         Summary(students, student_count);
    //     }
    //     else if (strcmp(op, "report programme") == 0) {
    //         ShowProgrammeReport(students, student_count);
    //     }

    //     // Report by Mark (Wei En)
    //     else if (strcmp(op, "report mark") == 0) {
    //         ShowMarkReport(students, student_count);
    //     }
    //     else {
    //         printf("Unrecognised input.\n");
    //     }
    // }

    

    
    return 0;

}