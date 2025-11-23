#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>


/*B Tree parameters*/
#define MIN_DEGREE 2 // Also used to refer to minimum nyumber of children
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MIN_KEYS (MIN_DEGREE - 1) 
#define MAX_CHILDREN (MIN_DEGREE * 2)

/*Header columns for database*/
#define ID "ID"
#define NAME "Name"
#define PROGRAMME "Programme"
#define MARK "Mark"

/*Max values for struct*/
#define MAX_NAME 100
#define MAX_PROGRAMME 100



typedef struct StudentRecord{
    int id;
    char name[MAX_NAME];
    char programme[MAX_PROGRAMME];
    float mark;
} StudentRecord;

typedef struct BTreeNode { //16 + 16 + 16 + 1 = 49 bits roughly equivalent to 12 bytes
    int num_keys; // Number of keys currently in the node
    StudentRecord *keys[MAX_KEYS]; //Array of pointers to keys with struct StudentRecord
    struct BTreeNode *children[MAX_CHILDREN]; // Array of pointers to other child nodes
    bool is_leaf;
} BTreeNode;


/*Sorting function*/
int sortmarkASC(const void* a, const void* b) {
    const StudentRecord* studentA = *(const StudentRecord**)a;
    const StudentRecord* studentB = *(const StudentRecord**)b;
    // Since mark is double, we need to return int
    if (studentA->mark < studentB->mark) return -1;
    if (studentA->mark > studentB->mark) return 1;
    return 0;
}
int sortmarkDESC(const void* a, const void* b) {
    const StudentRecord* studentA = *(const StudentRecord**)a;
    const StudentRecord* studentB = *(const StudentRecord**)b;
    if (studentA->mark > studentB->mark) return -1;
    if (studentA->mark < studentB->mark) return 1;
    return 0;
}


/* Helper Functions*/
void collectRecords(BTreeNode *root, StudentRecord **studentRecordsArr, int *num_students){
    
    if (root != NULL){
        int i;
        for (i = 0; i < root->num_keys; i++) {
            collectRecords(root->children[i],studentRecordsArr, num_students);       // left child
            *num_students -= 1;
            studentRecordsArr[*num_students] = root->keys[i];          // store pointer

        }
        collectRecords(root->children[i], studentRecordsArr, num_students);           // last child
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
    
}

/* B Tree Implementation*/
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

    // Move upper keys to new node
    for (int i = 0; i < MIN_KEYS; i++) {
        newNode->keys[i] = child->keys[i + MIN_DEGREE];
        child->keys[i + MIN_DEGREE] = NULL; // clear copied key
    }

    // Move children if not leaf
    if (!child->is_leaf) {
        for (int i = 0; i < MIN_DEGREE; i++) {
            newNode->children[i] = child->children[i + MIN_DEGREE];
            child->children[i + MIN_DEGREE] = NULL;
        }
    }

    child->num_keys = MIN_KEYS;

    // Shift children in parent
    for (int i = parent->num_keys + 1; i > index + 1; i--) {
        parent->children[i] = parent->children[i - 1];
    }
    parent->children[index + 1] = newNode;

    // Shift keys in parent
    for (int i = parent->num_keys; i > index; i--) {
        parent->keys[i] = parent->keys[i - 1];
    }

    // Insert median into parent
    parent->keys[index] = child->keys[MIN_DEGREE - 1];
    child->keys[MIN_DEGREE - 1] = NULL;

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

void createAndInsert(
    BTreeNode **root,
    int id,
    const char *name,
    const char *programme,
    float mark,
    int* num_students){
    //Creates a studentrecord struct, and inserts it into the b tree
    
    if (searchIndex(*root, id) != NULL){
        printf("The record with %s=%d already exists\n", ID, id);
        return;
    }
    else{
        StudentRecord *newRec = malloc(sizeof(StudentRecord));
        *num_students += 1;
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

}

void updateStudentRecord(BTreeNode *root, int search_index,char *field, char *value){
    StudentRecord * p_record = searchIndex(root, search_index);
    if (p_record){
        if (strcmp(field, "mark") == 0) {
            char *endptr;
            float f = strtof(value, &endptr);
            if (*endptr == '\0'){
                p_record->mark = f;
            }
            else{
                printf("Invalid data type! Must be type float!\n");
                return;
            }
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
        printf("Record not found!\n");
    }

}


/* ========== Deletion helpers ========== */

/* Get predecessor: go to child[idx] and then while not leaf go to last child */
StudentRecord *getPredecessor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->is_leaf) cur = cur->children[cur->num_keys];
    return cur->keys[cur->num_keys - 1];
}

/* Get successor: go to child[idx+1], then to leftmost key */
StudentRecord *getSuccessor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx + 1];
    while (!cur->is_leaf) cur = cur->children[0];
    return cur->keys[0];
}

/* Borrow from previous sibling (idx-1) into child idx */
void borrowFromPrev(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx - 1];

    // shift child's keys and children right by 1
    for (int i = child->num_keys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }
    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    // bring key from parent down to child
    child->keys[0] = node->keys[idx - 1];

    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
        sibling->children[sibling->num_keys] = NULL;
    }

    // move sibling's last key up to parent
    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];
    sibling->keys[sibling->num_keys - 1] = NULL;

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

/* Borrow from next sibling (idx+1) into child idx */
void borrowFromNext(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    // parent key goes to child's last position
    child->keys[child->num_keys] = node->keys[idx];

    if (!child->is_leaf) {
        child->children[child->num_keys + 1] = sibling->children[0];
        // shift sibling's children left
        for (int i = 0; i < sibling->num_keys; i++) {
            sibling->children[i] = sibling->children[i + 1];
        }
        sibling->children[sibling->num_keys] = NULL;
    }

    // move sibling's first key up to parent
    node->keys[idx] = sibling->keys[0];

    // shift sibling's keys left
    for (int i = 0; i < sibling->num_keys - 1; i++) {
        sibling->keys[i] = sibling->keys[i + 1];
    }
    sibling->keys[sibling->num_keys - 1] = NULL;

    child->num_keys += 1;
    sibling->num_keys -= 1;
}

/* Merge child[idx] with child[idx+1]. The key at parent[idx] moves down. */
void mergeChild(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx + 1];

    // Pull the key from parent down into child
    child->keys[MIN_KEYS] = node->keys[idx];

    // Copy keys from sibling to child
    for (int i = 0; i < sibling->num_keys; i++) {
        child->keys[i + MIN_DEGREE] = sibling->keys[i];
        sibling->keys[i] = NULL;
    }

    // Copy children pointers
    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->num_keys; i++) {
            child->children[i + MIN_DEGREE] = sibling->children[i];
            sibling->children[i] = NULL;
        }
    }

    // shift keys in parent to fill the gap
    for (int i = idx + 1; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->keys[node->num_keys - 1] = NULL;

    // shift children in parent
    for (int i = idx + 2; i <= node->num_keys; i++) {
        node->children[i - 1] = node->children[i];
    }
    node->children[node->num_keys] = NULL;

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    // free sibling node
    free(sibling);
}

/* Remove a key present in a leaf node at index idx */
void removeFromLeaf(BTreeNode *node, int idx) {
    // free the StudentRecord memory 
    if (node->keys[idx]) {
        free(node->keys[idx]);
        node->keys[idx] = NULL;
    }
    for (int i = idx + 1; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->keys[node->num_keys - 1] = NULL;
    node->num_keys--;
}

/* fill ensures that child[idx] has at least t keys by borrowing or merging */
void fill(BTreeNode *node, int idx) {
    if (idx != 0 && node->children[idx - 1]->num_keys >= MIN_DEGREE) {
        borrowFromPrev(node, idx);
    } else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= MIN_DEGREE) {
        borrowFromNext(node, idx);
    } else {
        if (idx != node->num_keys) {
            mergeChild(node, idx);
        } else {
            mergeChild(node, idx - 1);
        }
    }
}

/* The main recursive removal routine: remove key with id from subtree rooted at node */
void removeKey(BTreeNode *node, int id, int* num_students) {
    int idx = 0;
    while (idx < node->num_keys && node->keys[idx]->id < id) idx++;

    // Case 1: key is present in this node
    if (idx < node->num_keys && node->keys[idx]->id == id) {
        *num_students -= 1; // Since key is successfully found, it will be removed ,and thus num of students will decrease by 1
        printf("ID %d deleted succesfully\n", id);
        if (node->is_leaf) {
            // found in leaf
            removeFromLeaf(node, idx);
        } 
        else {
            // found in internal node
            // Handle using predecessor/successor/merge approach
            if (node->children[idx]->num_keys >= MIN_DEGREE) {
               StudentRecord *pred = getPredecessor(node, idx);

                // copy pred content into node->keys[idx]
                StudentRecord *copy = malloc(sizeof(StudentRecord));
                if (!copy) { 
                    perror("malloc"); exit(EXIT_FAILURE); 
                }
                *copy = *pred;
                free(node->keys[idx]);
                node->keys[idx] = copy;

                // recursively delete pred->id from children[idx]
                removeKey(node->children[idx], pred->id, num_students);
            } 
            else if (node->children[idx + 1]->num_keys >= MIN_DEGREE) {
                
                StudentRecord *succ = getSuccessor(node, idx);

                StudentRecord *copy = malloc(sizeof(StudentRecord));
                if (!copy) { perror("malloc"); exit(EXIT_FAILURE); }
                *copy = *succ;
                free(node->keys[idx]);
                node->keys[idx] = copy;

                removeKey(node->children[idx + 1], succ->id, num_students);
            } else {
                // merge and then recurse to the merged child
                mergeChild(node, idx);
                removeKey(node->children[idx], id, num_students);
            }
        }
    } else { // Case 2: key is not present in this node
        if (node->is_leaf) {
            printf("ID %d not found in database!\n", id);
            // Key not present in tree
            return;
        }
        // Determine if the child where the key may exist has at least t keys; if not, fill it
        bool flag = (idx == node->num_keys); // indicates if key is in last child

        if (node->children[idx]->num_keys == MIN_KEYS) {
            fill(node, idx);
        }

        // If we merged, the index may have changed
        if (flag && idx > node->num_keys) {
            removeKey(node->children[idx - 1], id, num_students);
        } else {
            removeKey(node->children[idx], id, num_students);
        }
    }
}

/* Public wrapper to delete key id from tree rooted at *root */
void deleteKey(BTreeNode **rootRef, int id, int*num_students) {
    if (*rootRef == NULL) return;

    removeKey(*rootRef, id, num_students);

    // If root has 0 keys, make its first child the new root (if any)
    if ((*rootRef)->num_keys == 0) {
        BTreeNode *oldRoot = *rootRef;
        if (oldRoot->is_leaf) {
            // tree becomes empty
            free(oldRoot);
            *rootRef = NULL;
        } else {
            BTreeNode *newRoot = oldRoot->children[0];
            free(oldRoot);
            *rootRef = newRoot;
        }
    }
}


/*Printing Records*/
void printRecord(StudentRecord *rec) {
    printf("%-10d %-15s %-25s %-5.1f\n",
        rec->id,
        rec->name,
        rec->programme,
        rec->mark
    );

}
void printHeader(){
    printf("%-10s %-15s %-25s %-5s\n", 
    ID, NAME, PROGRAMME, MARK);
}

void traversal(BTreeNode *root, bool descending, bool needToPrintRecord) {
    if (root != NULL) {
        int i;
        int start_index  = descending ? root->num_keys : 0;
        int end_index = descending ? 0 : root->num_keys ;
        int step = descending ? -1 : 1;

        for (i = start_index; i != end_index ; i += step) {
            traversal(root->children[i], descending, needToPrintRecord);
            if (needToPrintRecord) {
                int key_to_print = descending ? i - 1 : i;
                printRecord(root->keys[key_to_print]);
                
            }
        }
        traversal(root->children[i], descending, needToPrintRecord);
       
    }
}


void showAllByMarks(BTreeNode *root, int *p_num_students, bool descending){
    StudentRecord **studentRecordsArr = calloc(*p_num_students, sizeof(StudentRecord *));
    if (studentRecordsArr == NULL) {
      fprintf(stderr, "Memory allocation failed!\n");
    }
    else{
        int counter = *p_num_students;
        int*p_counter = &counter;//Copying num_students so we dont end up modifying the original value
        collectRecords(root, studentRecordsArr, p_counter);
        descending ? qsort(studentRecordsArr, *p_num_students, sizeof(StudentRecord *), sortmarkDESC) : qsort(studentRecordsArr, *p_num_students, sizeof(StudentRecord *), sortmarkASC);
        for (int i = 0; i < *p_num_students; i++){
            printRecord(studentRecordsArr[i]);
        }
        free(studentRecordsArr);
    }
}

void showSummaryStatistics(BTreeNode *root){
    if (root != NULL) {
        int i;
        // traversal();
    }   
}



void input_showSorted(BTreeNode *root, int *num_students, char *sortby, char *order){
    bool isDescending = strcmp(order, "desc") == 0 ;
    if (strcmp(sortby, "id") == 0){
        traversal(root,  isDescending, true);
    }
    else if (strcmp(sortby, "mark") == 0){
        showAllByMarks(root, num_students, isDescending);
    }
    else{
        printf("Follow this format to sort the data: SHOW ALL SORT BY ID/MARK ASC/DESC.\n");
    }
}

void input_insert(BTreeNode **root, int *id,int* num_students,char *key1, char *val1, char *key2, char *val2, char *key3, char *val3){
    if (strcmp(key1, "name") != 0 || strcmp(key2, "programme") != 0 || strcmp(key3, "mark") != 0){
        printf("The order matters! Please follow the format given: INSERT ID=<ID NUMBER> NAME=<NAME> NAME=<PROGRAMME>MARK=<MARK>\n");
        return;
    }
    else{
        char *endptr;

        float f = strtof(val3, &endptr);

        if (*endptr == '\0') {// string converted
            createAndInsert(root, *id, val1, val2, f, num_students);
        }
    }

}

int main(){
    BTreeNode *root = NULL;
    int num_students = 0;
    int* p_num_students = &num_students;


    createAndInsert(&root,
                2502841,
                "Alicia Tan",
                "Computer Science",
                72.5,
            p_num_students);

    createAndInsert(&root,
                    2509174,
                    "Marcus Lim",
                    "Information Security",
                    64.0,
                p_num_students);

    createAndInsert(&root,
                    2505532,
                    "Samantha Ong",
                    "Data Analytics",
                    81.0,
                p_num_students);

    createAndInsert(&root,
                    2503328,
                    "Rahul Nair",
                    "Software Engineering",
                    49.5,
                p_num_students);

    createAndInsert(&root,
                    2507769,
                    "Chloe Wong",
                    "Business Analytics",
                    90.0,
                p_num_students);

    createAndInsert(&root,
                    2504417,
                    "Nicholas Lee",
                    "Applied AI",
                    58.0,
                p_num_students);

    createAndInsert(&root,
                    2506355,
                    "Emily Chan",
                    "Cybersecurity",
                    73.0,
                p_num_students);

    char op[100];
    int id;

    
    
    
    
    

    
    while (1) {
        printf("\nEnter your command:");
        fgets(op, sizeof(op), stdin);
        op[strcspn(op, "\n")] = 0;

        // Lower user input
        for (int i = 0; op[i]; i++) {
            op[i] = tolower(op[i]);
        }

        // // OPEN
        // if (strcmp(op, "open") == 0) {
        //     student_count = Open(students, filename);
        //     if(student_count != 0){
        //         printf("The database file \"%s\" is successfully opened.\n", filename);
        //     }
        // }
        // SHOW ALL
        if (strcmp(op, "show all") == 0) {
            int found = -1;
            printf("Here are all the records found in StudentRecords \"%s\"\n");
            printHeader();
            traversal(root, false, true);
            // showAllById(root, false);
        }
        // SHOW ALL SORTED
        else if (strstr(op, "show all sort") != NULL) {
            char sortby[10];
            char order[10];
            if (sscanf(op, "show all sort by %s %s", sortby, order) == 2 
                && ((strcmp(order, "desc") == 0) || (strcmp(order, "asc") == 0))){
                printHeader();
                input_showSorted(root, p_num_students, sortby, order);
            }
            else {
                printf("Follow this format to sort the data: SHOW ALL SORT BY ID/MARK ASC/DESC.\n");
            }
        }
        // INSERT 
        else if (strstr(op, "insert") != NULL) {
            char key1[MAX_PROGRAMME];
            char key2[MAX_PROGRAMME];
            char key3[MAX_PROGRAMME];

            char val1[MAX_PROGRAMME];
            char val2[MAX_PROGRAMME];
            char val3[MAX_PROGRAMME];


            if (sscanf(op,"insert id=%d " "%[^=]=%[^p] " "%[^=]=%[^m] " "%[^=]=%s", &id,
                key1, val1,
                key2, val2,
                key3, val3) == 7) {
                input_insert(&root, &id, p_num_students,key1,val1,key2,val2,key3,val3 );
                printf("ID %d successfully inserted\n", id);

                
            }
            else {
                printf("Follow this format to insert new data: INSERT ID=<ID NUMBER> NAME=<NAME> PROGRAMME=<PROGRAMME>MARK=<MARK>\n");
            }
        }
        // QUERY
        else if (strstr(op, "query") != NULL) {
            if (sscanf(op, "query id=%d", &id) == 1) {
                StudentRecord * rec = searchIndex(root, id);
                if(rec) {
                    printHeader();
                    printRecord(rec);
                }
                else{
                    printf("ID %d not found!\n",id );
                }
                
            }
            else {
                printf("Follow this format to make a query: QUERY ID=<ID NUMBER>.\n");
            }
        }
        // UPDATE
        else if (strstr(op, "update") != NULL) {
            char field[100];
            char value[100];
            if (sscanf(op, "update id=%d %[^=]=%[^\n]", &id, field, value) == 3) {
                updateStudentRecord(root, id, field, value);
            }
            else {
                printf("Follow this format to update: UPDATE ID=<ID Number> <Field>=<Value>.\nExample: UPDATE ID=2801234 MARK=98.7\n.");
            }
        }
        // DELETE
        else if (strstr(op, "delete") != NULL) {
            if (sscanf(op, "delete id=%d", &id) == 1) {
                deleteKey(&root, id, p_num_students);
            }
            else {
                printf("Follow this format to delete data: DELETE ID=<ID NUMBER>.\n");
            }
        }
    //     // SAVE
    //     else if (strcmp(op, "save") == 0) {
    //         Save(students, student_count, filename);
    //     }
        // SUMMARY
        // else if (strcmp(op, "show summary") == 0) {
        //     Summary(students, student_count);
        // }
        // else if (strcmp(op, "report programme") == 0) {
        //     ShowProgrammeReport(students, student_count);
        // }

        // // Report by Mark (Wei En)
        // else if (strcmp(op, "report mark") == 0) {
        //     ShowMarkReport(students, student_count);
        // }
        // else {
        //     printf("Unrecognised input.\n");
        // }
    }

    
    
    return 0;

}