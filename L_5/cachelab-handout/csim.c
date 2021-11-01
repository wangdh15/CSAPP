#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define BOOL int
#define TRUE 1
#define FALSE 0
#define ULL unsigned long long
#define MAX_BUFF_SIZE 255


const char* const defalut_args = "hvs:E:b:t:";
BOOL flag_v = FALSE;
// const int MAX_BUFF_SIZE = 255;
char buff[MAX_BUFF_SIZE];

int set_idx_bits = -1;
int num_pre_line = -1;
int num_blk_bits = -1;
char* trace_file_path = NULL;

int hit = 0, miss = 0, eviction = 0;


struct st{
    BOOL _miss;
    BOOL _hit;
    BOOL _eviction;
};

typedef struct st st;

st state;

struct LinkListNode{
    struct LinkListNode *prev, *next;
    ULL tag;
};

typedef struct LinkListNode LinkListNode;

struct LinkList{
    LinkListNode *head;
    int len;
};

typedef struct LinkList LinkList;

/**
 * @brief init the link list;
 * 
 * @param link_list 
 */
void init_link_list(LinkList *link_list) {
    link_list -> head = (LinkListNode*) malloc(sizeof(LinkListNode));
    if (link_list -> head == NULL) {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }
    link_list -> head -> next = link_list -> head;
    link_list -> head -> prev = link_list -> head;
    link_list -> len = 0;
}

// free the memory
void free_link_list(LinkList* link_list) {

    LinkListNode* cur = link_list -> head -> next;
    while (cur != link_list -> head) {
        LinkListNode* ne = cur -> next;
        free(cur);
        cur = ne;
    }
    free(link_list -> head);
    // free(link_list);
}

LinkList* cache;


/**
 * @brief insert a new node with tag to the link list;
 * 
 * @param tag 
 * @param head 
 */
void insert(ULL tag, LinkList* link_list) {

    LinkListNode* new_node = (LinkListNode*)malloc(sizeof(LinkListNode));
    if (new_node == NULL) {
        fprintf(stderr, "malloc error!\n");
        exit(1);
    }
    new_node -> tag = tag;
    new_node -> next = link_list -> head -> next;
    new_node -> prev = link_list -> head;
    link_list -> head -> next = new_node;
    new_node -> next -> prev = new_node;
    link_list -> len ++;
}

/**
 * @brief find a tag in a link list.
 * 
 * @param link_list 
 * @param tag 
 * @return LinkListNode* 
 */
LinkListNode* find_tag(LinkList* link_list, ULL tag) {

    LinkListNode* cur = link_list -> head;
    while (cur -> next != link_list -> head) {
        cur = cur -> next;
        if (cur -> tag == tag) return cur;
    }
    return NULL;

}

/**
 * @brief delete a node from the link_list 
 * 
 * @param node 
 */
void delete_node(LinkListNode* node) {
    node -> prev -> next = node -> next;
    node -> next -> prev = node -> prev;
    free(node);
}



// init the state
void init(st* p_st) {
    p_st -> _miss = FALSE;
    p_st -> _hit = FALSE;
    p_st -> _eviction = FALSE;
}





/**
 * @brief help function.
 * 
 */
void print_help_info() {

    printf("error args\n");
    return;

}

/**
 * @brief check the args if valid or not.
 * 
 * @return BOOL 
 */
BOOL check_args() {

    // printf("args: v : %d, s : %d, E : %d, b : %d,  t : %s\n", flag_v, set_idx_bits, num_pre_line, num_blk_bits, trace_file_path);
    return set_idx_bits != -1 && 
           num_pre_line != -1 && 
           num_blk_bits != -1 && 
           trace_file_path != NULL;

}

/**
 * @brief extract the args
 * 
 * @param argc 
 * @param argv 
 */
void process_args(int argc, char* argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, defalut_args)) != -1) {
        switch (opt) {
            case 'h':
                print_help_info();
                exit(0);
            case 'v':
                flag_v = TRUE;
                break;
            case 's':
                set_idx_bits = atoi(optarg);
                break;
            case 'E':
                num_pre_line = atoi(optarg);
                break;
            case 'b':
                num_blk_bits = atoi(optarg);
                break;
            case 't':
                trace_file_path = optarg;
                break;
        }
    }
    if (!check_args()) {
        print_help_info();
        exit(1);
    }
}

// get the set number
ULL get_set_idx(ULL addr) {
    return (addr >> num_blk_bits) & ((1ULL << set_idx_bits) - 1);
}

// get the tag
ULL get_tag(ULL addr) {
    return addr >> (num_blk_bits + set_idx_bits);
}

/**
 * @brief process each line.
 * 
 * @param type 
 * @param addr 
 */
void proc(char type, ULL addr) {

    ULL set_idx = get_set_idx(addr);
    ULL tag = get_tag(addr);
    LinkList* line = cache + set_idx;
    LinkListNode* find_result = find_tag(line, tag);
    if (find_result == NULL) {
        miss ++;
        state._miss = TRUE;
        if (line -> len == num_pre_line) {
            delete_node(line -> head -> prev);
            line -> len --;
            eviction ++;
            state._eviction = TRUE;
        } 
    } else {
        hit ++;
        state._hit = TRUE;
        delete_node(find_result);
        line -> len --;
    }
    insert(tag, line);
    if (type == 'M') {
        hit ++;
        state._hit = TRUE;
    }
    return;

}


int main(int argc, char* argv[]) {
    
    process_args(argc, argv);  // firstly, get_all_the_args.

    FILE* fp = NULL;
    fp = fopen(trace_file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "file %s not exist\n", trace_file_path);
        exit(1);
    }


    // init the cache data structure.
    cache = (LinkList*)malloc(sizeof(LinkList) * (1 << set_idx_bits));
    if (cache == NULL) {
        fprintf(stderr, "malloc error!\n");
        exit(1);
    }
    for (int i = 0; i < (1 << set_idx_bits); i ++) {
        init_link_list(cache + i);
    }

    // process the trace file.
    while (fgets(buff, MAX_BUFF_SIZE, fp) != NULL) {
        int len = strlen(buff);
        buff[len - 1] = '\0'; // delete the \n
        if (buff[0] != ' ') continue; // ignore instruction.

        // get the ref type, L, M, S
        char type = buff[1];  

        // get the address
        ULL addr = 0;
        for (int i = 3; buff[i] != ','; i ++) {
            addr <<= 4;
            if (buff[i] >= '0' && buff[i] <= '9') addr += buff[i] - '0';
            else addr += buff[i] - 'a' + 10;
        }
        // the size doesn't matter
        
        init(&state);
        proc(type, addr);
        if (flag_v) {
            printf("%s ", buff + 1);
            if (state._miss) printf("miss ");
            if (state._eviction) printf("eviction ");
            if (state._hit) printf("hit");
            putchar('\n');
        }

    }
    
    fclose(fp);
    // manage memory leak.s
    for (int i = 0; i < (1 << set_idx_bits); i ++) {
        free_link_list(cache + i);
    }
    free(cache);

    
    printSummary(hit, miss, eviction);
    return 0;
}
