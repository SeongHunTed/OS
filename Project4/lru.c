#include "algorithm.h"

void pushLRU(struct node **head, int nextpage)
{
    struct node *new = (struct node*)malloc(sizeof(struct node));

    new->page = nextpage;
    
    new->next = (*head);
    new->prev = NULL;

    if((*head) != NULL){
        (*head)->prev = new;
    }

    (*head) = new;
}

// head와 tail이 아닌 page가 HIT 되었을 때 해당 위치에서 삭제 후 head로 옮길때 사용
void deleteLRU(struct node *head, int deletepage, int maxpage)
{
    struct node *del = head;
    for (int i = 0; i < maxpage; i++)
    {
        if (del->page == deletepage){
            break;
        }
        del = del->next;
    }

    if(del == NULL || head == NULL)
        return;
    if(head == del)
        head = del->next;
    if(del->next != NULL)
        del->next->prev = del->prev;
    if(del->prev != NULL)
        del->prev->next = del->next;
    free(del);
    return;
}

// 맨마지막 노드 지우기
void deleteTail(struct node *target)
{   
    while(1)
    {
        if(target->next == NULL){
            target->prev->next = NULL;
            target->prev = NULL;
            free(target);
            break;
        }
        target = target->next;
    }
}

void printList(struct node *n)
{   
    while (n != NULL) {
        printf("%d\t", n->page);
        if(save == 2) fprintf(resultfp, "%d\t", n->page);
        n = n->next;
    }
    printf("\n");
    if(save == 2) fprintf(resultfp, "\n");
}

int checkLRU(int maxpage, int nextpage, struct node *head)
{
    struct node *target = head;
    // 순회하며, Hit, Fault 판별
    while(1)
    {   
        if(target->page == nextpage){       // Hit
            return 0;
        } else if(target->page == -1){      // Page Fault -> Empty Page
            return 1;
        }
        if(target->next == NULL)            // rear
            break;
        target = target->next;
    }
    return 2;                               // Page Fault
}

int lru(int *page, int maxpage)
{
    int nextpage;                           // 교체페이지
    int count = 0;                          // pagefault count
    int flag;                               // checkLRU return value
    struct node *head = NULL;               // List's Head

    // DLL 초기화
    for (int i = 0; i < maxpage; i++){
        pushLRU(&head, -1);
    }
    printf("Page reference string\tFrames\n");
    if(save == 2) fprintf(resultfp, "Page reference string\tFrames\n");
    for (int i = 0; i < PAGENUM; i++)
    {
        nextpage = page[i];
        printf("%d\t\t\t", nextpage);
        if(save == 2) fprintf(resultfp, "%d\t\t\t", nextpage);
        // 똑같은 페이지가 연속으로 들어올 경우 처리
        if(head->page == nextpage){
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
            continue;
        }
        flag = checkLRU(maxpage, nextpage, head);
        if (flag == 0){                                             // 페이지 Hit
            deleteLRU(head, nextpage, maxpage);                     // 해당 페이지 참조 되었으므로 위치이동
            pushLRU(&head, nextpage);
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
        }
        else if (flag == 1){                                        // page fault in empty page frame
            pushLRU(&head, nextpage);                               // push
            deleteLRU(head, -1, maxpage);                           // delete -1인 페이지
            printList(head);
            count++;                                                // pagefault counter 
        }
        else if (flag == 2){                                        // pagefault
            pushLRU(&head, nextpage);                               // page push
            deleteTail(head);                                       // 오래된 page 삭제
            printList(head);
            count++;                                                // pagefault counter
        }
        else {
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
        }
    }
    printf("\nLRU Algorithm Page Fault : %d\n\n", count);
    if(save == 2) fprintf(resultfp, "\nLRU Algorithm Page Fault : %d\n\n", count);

    return count;
}