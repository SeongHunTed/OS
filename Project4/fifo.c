#include "algorithm.h"

void printFIFO(int maxpage, node *tmp)
{
    int j;
    while(1)
    {   
        if(tmp == NULL){
            break;
        }
        printf("%d\t", tmp->page);
        if(save == 2) fprintf(resultfp, "%d\t", tmp->page);
        tmp = tmp->next;
    }
    printf("\n");
    if(save == 2) fprintf(resultfp, "\n"); 
}    

List *initial() {
    List *l = (List*)malloc(sizeof(List));
    l->head = NULL;
    l->tail = NULL;

    return l;
}

void add(List *l, int page)
{
    node *tmp = (node*)malloc(sizeof(node));
    tmp->page = page;
    tmp->next = NULL;

    if(l->head == NULL)
        l->head = tmp;
    else
        l->tail->next = tmp;
    l->tail = tmp;
}

void delete(List *l)
{
    node *tmp;

    tmp = l->head;
    l->head = l->head->next;

    free(tmp);
}

// 페이지 교체 함수
int fifo(int *page, int maxpage)
{   
    int nextpage;
    int count = 0;

    // LinkedList 초기화
    List *l = initial();
    for(int i = 0; i < maxpage; i++){
        add(l, -1);  
    }

    // page순회하면 pageframe 할당
    printf("Page reference string\tFrames\n");
    if(save == 2) fprintf(resultfp, "Page reference string\tFrames\n");
    for (int i = 0; i < PAGENUM; i++)
    {
        nextpage = page[i];
        printf("%d\t\t\t", nextpage);
        if(save == 2) fprintf(resultfp, "%d\t\t\t", nextpage);
        if(checkFifo(maxpage, nextpage, l) > 0)
        {
            add(l, nextpage);
            delete(l);
            node *tmp = l->head;
            printFIFO(maxpage, tmp);
            count++;
        }
    }
    printf("\nFIFO Algorithm Page Fault : %d\n\n", count);
    if(save == 2) fprintf(resultfp, "\nFIFO Algorithm Page Fault : %d\n\n", count);
    return count;
}

// Hit = 0, Fault = 1 유무 검사
int checkFifo(int maxpage, int nextpage, struct List *l)
{
    node *tmp;
    tmp = l->head;

    for(int i = 0; i < maxpage; i++)
    {
        if(tmp->page == nextpage){
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
            return 0;
        }
        tmp = tmp->next;
    }
    return 1;
}