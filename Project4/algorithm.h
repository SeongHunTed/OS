#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PAGENUM 500
extern int save;
extern FILE *resultfp;

// Optimal
int optimal(int *page, int maxpage);
int predict(int *page, int maxpage, int i, int *pageframe);
int checkOptimal(int maxpage, int nextpage, int *pageframe);

// FIFO
typedef struct node {
    int page;
    struct node *next;
    struct node *prev;
} node;
typedef struct List {
    node *head;
    node *tail;
} List;
List *initial();
void add(List *l, int page);
void delete(List *l);
int fifo(int *page, int maxpage);
int checkFifo(int maxpage, int nextpage, List *l);

// LIFO
int checkLifo(int maxpage, int nextpage, int *pageframe);
int lifo(int *page, int maxpage);

// LRU (Least Recently Used)
void pushLRU(struct node **head, int nextpage);
void deleteLRU(struct node *head, int deletepage, int maxpage);
void deleteTail(struct node *target);
void printList(struct node *n);
int checkLRU(int maxpage, int nextpage, struct node *head);
int lru(int *page, int maxpage);

// LFU
int checkLFU(int maxpage, int nextpage, int *pageframe);
int lfu(int *page, int maxpage);

// Second Chance(One Handed)
int sc(int *page, int maxpage);

// Enhanced Second Chance
int esc(int *page, int maxpage);
int *makePageESC(int *escStream, int *page, int *wrBit);