#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MIN_DEGREE 2 // Also used to refer to minimum nyumber of children
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MIN_KEYS (MIN_DEGREE - 1) 
#define MAX_CHILDREN (MIN_DEGREE * 2)

/*Header columns*/
#define ID "ID"
#define NAME "Name"
#define PROGRAMME "Programme"
#define MARK "Score"


typedef struct StudentRecord{
    int id;
    char name[100];
    char programme[100];
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
            printf("Num Students: %d\n", *num_students);
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
    else{
        printf("Not found!");
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

void createAndInsert(BTreeNode **root,
                     int id,
                     const char *name,
                     const char *programme,
                     float mark,
                    int* num_students)
{
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
    printf("%-10s %-15s %-25s %-5.1f\n", 
    ID, NAME, PROGRAMME, MARK);
}

/* Show All functions for both ID and Marks*/
void showAllById(BTreeNode *root, bool descending) {
    if (root != NULL) {
        int i;
        if (!descending){
            for (i = 0; i < root->num_keys; i++) {
                showAllById(root->children[i], descending);
                printRecord(root->keys[i]);
            }
            showAllById(root->children[i], descending);
        }
        else{
            for (i = root->num_keys; i > 0; i--) {
                showAllById(root->children[i], descending);
                printRecord(root->keys[i-1]);
            }
            showAllById(root->children[i], descending);
        }
    }
}

void showAllByMarks(BTreeNode *root, int *p_num_students, bool descending){
    StudentRecord **studentRecordsArr = calloc(*p_num_students, sizeof(StudentRecord *));
    if (studentRecordsArr == NULL) {
      fprintf(stderr, "Memory allocation failed!\n");
    }
    else{
        int counter = *p_num_students;
        int*p_counter = &counter;
        collectRecords(root, studentRecordsArr, p_counter);
        descending ? qsort(studentRecordsArr, *p_num_students, sizeof(StudentRecord *), sortmarkDESC) : qsort(studentRecordsArr, *p_num_students, sizeof(StudentRecord *), sortmarkASC);
        for (int i = 0; i < *p_num_students; i++){
            printRecord(studentRecordsArr[i]);
        }
        free(studentRecordsArr);
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

    // char op[100];
    // int id;
    // searchIndex(root,2506358 );
    showAllById(root, true);

    printf("\n");
    printf("Num of students: %d\n", num_students);

    showAllByMarks(root, p_num_students, false);




    


   
    return 0;

}