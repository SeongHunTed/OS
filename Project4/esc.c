#include "algorithm.h"

typedef struct escNode {
    int page;           // page number
    int readBit;        // reference bit
    int writeBit;       // write bit (dirty bit)
} eNode;

typedef struct CircularQueue{
    eNode* nodes;       // 노드 배열 가르킬 포인터
    int capacity;       // 큐 크기
    int front;          // 첫번째 위치
    int rear;           // 마지막 위치
} eCircularQueue;

int *makePageESC(int *escStream, int *page, int *wrBit)
{   
    for(int i = 0; i < PAGENUM; i++)
    {
        escStream[i*2] = page[i];
        escStream[i*2 + 1] = wrBit[i];
    }
    return escStream;
}

// 환형 큐 생성
void createCircularQueueEsc(eCircularQueue **queue, int capacity) 
{
    // Heap 영역에 큐의 공간을 할당
    (*queue) = (eCircularQueue*)malloc(sizeof(eCircularQueue));
    // 노드 배열을 생성
    (*queue)->nodes = (eNode*)malloc(sizeof(eNode)* (capacity+1));
    // 큐의 크기 정함
    (*queue)->capacity = capacity;
    // 큐의 front와 rear 0으로 초기화
    (*queue)->front = 0;
    (*queue)->rear = 0;
}

// queue의 상태 확인
int isEmptyEsc(eCircularQueue *queue)
{
    return (queue->front == queue->rear);
}

// queue의 상태 확인
int isFullEsc(eCircularQueue *queue)
{
    if(((queue->rear+1) % queue->capacity) == queue->front)
        return 1;
    else
        return 0;
}

// queue 의 front 삭제
void dequeueEsc(eCircularQueue *queue)
{
    queue -> front = (queue -> front + 1) % queue -> capacity;      // 모듈로 연산
}

// queue 의 rear 삽입
void enqueueEsc(eCircularQueue *queue, int page, int readBit, int writeBit)
{
    if(isFullEsc(queue)){
        printf("Queue is Full\n");
    } else {
        queue->rear = (queue -> rear + 1)  % queue->capacity;
        queue->nodes[queue->rear].page = page;
        queue->nodes[queue->rear].readBit = readBit;
        queue->nodes[queue->rear].writeBit = writeBit;
    }
}

// queue 초기화
void freeQueueEsc(eCircularQueue *queue)
{
    free(queue->nodes);
    free(queue);
}

// 큐 전체 print
void printEsc(eCircularQueue *queue)
{
    int i = queue->front;
    do
    {
        i = (i + 1) % queue->capacity;
        printf("\t\t%d\t%d(R)\t%d(W)\t큐인덱스 : %d\n\t\t", queue->nodes[i].page, queue->nodes[i].readBit,
        queue->nodes[i].writeBit,i);
        if(save == 2) fprintf(resultfp, "\t\t%d\t%d(R)\t%d(W)\t큐인덱스 : %d\n\t\t", queue->nodes[i].page, 
        queue->nodes[i].readBit, queue->nodes[i].writeBit,i);
        if(i == queue->rear)
            break;
    } while(i != queue->front);
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b---------------------------------------------------------------------\n");
    if(save == 2) fprintf(resultfp, "\n---------------------------------------------------------------------\n");
}   

int checkEsc(int maxpage, int nextpage, int rwBit, eCircularQueue *queue)
{
    int target = (queue->front+1) % queue->capacity;

    for(int i = 0; i < maxpage; i++)
    {
        // Hit
        if(queue->nodes[target].page == nextpage)
        {   
            printf("\t\tNo replacement\n\t\t");
            if(save == 2) {
                fprintf(resultfp, "\t\tNo replacement\n\t\t");
            }
            queue->nodes[target].readBit = 1;
            // Hit -> if write bit = 1 -> chagne to 1
            if(rwBit == 1)
                queue->nodes[target].writeBit = 1;
            printEsc(queue);
            return 0;
        }
        target = (target+1) % queue->capacity;
    }

    // 초기 pagefault
    target = (queue->front+1) % queue->capacity;
    if(queue->nodes[target].page < 0)
    {
        dequeueEsc(queue);
        if(rwBit == 0) enqueueEsc(queue, nextpage, 1, 0);
        else enqueueEsc(queue, nextpage, 1, 1);
        printEsc(queue);
        return 1;
    }

    target = (queue->front+1) % queue->capacity;
    int firstMeet = 0;
    int count = 0;
    for(int i = 0; i < 2*(maxpage+1); i++)
    {
        // Queue내에 0, 0 있는지 체크 후 추방
        if(queue->nodes[target].readBit == 0 && queue->nodes[target].writeBit == 0)
        {   
            dequeueEsc(queue);
            if(rwBit == 0) enqueueEsc(queue, nextpage, 1, 0);
            else enqueueEsc(queue, nextpage, 1, 1);
            printEsc(queue);
            return 1;
        }
        
        // front에 (0, 1)
        // 맨 처음 0, 1이 되는 page.number를 기억..
        // 큐를 막 돌리다가 얘가 또 오면 얘  deque
        if(queue->nodes[target].readBit == 0 && queue->nodes[target].writeBit == 1)
        {
            int enqPage, enqWriteBit;
            enqPage = queue->nodes[target].page;
            enqWriteBit = queue->nodes[target].writeBit;
            if(count == 1 && queue->nodes[target].page == firstMeet 
            && queue->nodes[(target+1)%queue->capacity].readBit != 1 
            && queue->nodes[(target+2)%queue->capacity].readBit != 1 
            && queue->nodes[(target+1)%queue->capacity].writeBit != 0 
            && queue->nodes[(target+2)%queue->capacity].writeBit != 0)
            {
                dequeueEsc(queue);
                if(rwBit == 0) enqueueEsc(queue, nextpage, 1, 0);
                else enqueueEsc(queue, nextpage, 1, 1);
                printEsc(queue);
                return 1;
            } else {
                dequeueEsc(queue);
                enqueueEsc(queue, enqPage, 0, enqWriteBit);
                // printEsc(queue);
                // printf("\t\t");
            }
        }

        // page frame 내에서 3번까지는 FIFO로 구현할 수 있는 장치
        if(queue->nodes[target].readBit == 0 && queue->nodes[target].writeBit == 1 && count == 0) {
            firstMeet = queue->nodes[target].page;
            count++;
        }

        // front에 (1, 0)
        if(queue->nodes[target].readBit == 1 && queue->nodes[target].writeBit == 0)
        {
            int enqPage, enqWriteBit;
            enqPage = queue->nodes[target].page;
            enqWriteBit = queue->nodes[target].writeBit;
            dequeueEsc(queue);
            enqueueEsc(queue, enqPage, 0, enqWriteBit);     // 기회 한번 더
        }
        

        // front에 (1, 1)
        if(queue->nodes[target].readBit == 1 && queue->nodes[target].writeBit == 1)
        {
            int enqPage, enqWriteBit;
            enqPage = queue->nodes[target].page;
            enqWriteBit = queue->nodes[target].writeBit;
            dequeueEsc(queue);
            enqueueEsc(queue, enqPage, 0, enqWriteBit);
        }
        target = (target+1) % queue->capacity;
    }

    return 1;
}

int esc(int *page, int maxpage)
{
    eCircularQueue *queue;              // 환형 큐
    int queueSize = maxpage + 1;        // 큐 사이즈
    int nextpage;                       // 다음 페이지
    int rwBit;                          // 입력 구분 R, W
    int count = 0;                      // Pagefault

    // 큐 생성 및 초기화
    createCircularQueueEsc(&queue, queueSize);
    for(int i = 0; i < maxpage; i++)
    {
        enqueueEsc(queue, -1, -1, -1);
    }

    printf("Page\tR or W Bit\t\tFrames\tR-bit\tW-bit\n");
    if(save == 2) {
        fprintf(resultfp, "\n\n---------------------------------------------------------------------\n");
        fprintf(resultfp, "Page\tR or W Bit\t\tFrames\tR-bit\tW-bit\n");
    }

    for(int i = 0; i < PAGENUM; i++)
    {
        int flag = 0;
        nextpage = page[2*i];
        rwBit = page[2*i + 1];
        // readBit == 0 -> R인 경우로 가정
        if(rwBit == 0){
            printf("%d\tR\t", nextpage);
            if(save == 2) fprintf(resultfp, "%d\tR\t", nextpage);
        } else if(rwBit == 1) { // writeBit == 0 -> W인 경우로 가정
            printf("%d\tW\t", nextpage);
            if(save == 2) fprintf(resultfp, "%d\tW\t", nextpage);
        }
        flag = checkEsc(maxpage, nextpage, rwBit, queue);
        if(flag > 0) count++;
    }

    freeQueueEsc(queue);

    printf("\nEnhanced Second Change Algorithm Page Fault : %d\n", count);
    if(save == 2) fprintf(resultfp, "\nEnhanced Second Change Algorithm Page Fault : %d\n", count);

    return count;
}
