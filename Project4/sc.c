#include "algorithm.h"

typedef struct scNode {
    int page;           // page number
    int referenceBit;   // reference bit
} Node;

typedef struct CircularQueue{
    Node* nodes;    // 노드 배열 가르킬 포인터
    int capacity;   // 큐 크기
    int front;      // 첫번째 위치
    int rear;       // 마지막 위치
} CircularQueue;

// 환형 큐 생성
void createCircularQueue(CircularQueue **queue, int capacity) 
{
    // Heap 영역에 큐의 공간을 할당
    (*queue) = (CircularQueue*)malloc(sizeof(CircularQueue));
    // 노드 배열을 생성
    (*queue)->nodes = (Node*)malloc(sizeof(Node)* (capacity+1));
    // 큐의 크기 정함
    (*queue)->capacity = capacity;
    // 큐의 front와 rear 0으로 초기화
    (*queue)->front = 0;
    (*queue)->rear = 0;
}

// queue의 상태 확인
int isEmpty(CircularQueue *queue)
{
    return (queue->front == queue->rear);
}

// queue의 상태 확인
int isFull(CircularQueue *queue)
{
    if(((queue->rear+1) % queue->capacity) == queue->front)
        return 1;
    else
        return 0;
}

// queue 의 front 삭제
void dequeue(CircularQueue *queue)
{
    queue -> front = (queue -> front + 1) % queue -> capacity;      // 모듈로 연산
}

// queue 의 rear 삽입
void enqueue(CircularQueue *queue, int page, int referenceBit)
{
    if(isFull(queue)){
        printf("Queue is Full\n");
    } else {
        queue->rear = (queue -> rear + 1)  % queue->capacity;
        queue->nodes[queue->rear].page = page;
        queue->nodes[queue->rear].referenceBit = referenceBit;
    }
}

// queue 초기화
void freeQueue(CircularQueue *queue)
{
    free(queue->nodes);
    free(queue);
}

// 큐 전체 print
void printSC(CircularQueue *queue)
{
    int i = queue->front;
    do
    {
        i = (i + 1) % queue->capacity;
        printf("\t\t\tFrame : %d\tBit : %d \t큐인덱스 : %d\n", queue->nodes[i].page, queue->nodes[i].referenceBit, i);
        if(save == 2) fprintf(resultfp, "\t\t\tFrame : %d\tBit : %d \t큐인덱스 : %d\n", queue->nodes[i].page, 
        queue->nodes[i].referenceBit, i);
        if(i == queue->rear)
            break;
    } while(i != queue->front);
    printf("---------------------------------------------------------------------\n");
    if(save == 2) fprintf(resultfp, "-------------------------------------------------------------------\n");
}

// 1. Pageframe 내에 해당 페이지가 있는지 검사한다.
//      - 존재한다면 reference bit = 1 할당
// 2. Pageframe 내에 해당 페이지가 없다면
//      2-1. reference bit == 1
//          dequeue(queue) -> 맨앞이 가르키는애 쫓아 내고 
//          page, reference bit = 0 -> enqueue()
//          다시 처음으로 돌아가서 확인
//      2-2. reference bit == 0
//          dequeue(queue) -> 바로 쫓아냄
//          page, reference bit = 1 -> enqueue()
int checkSC(int maxpage, int nextpage, CircularQueue *queue)
{
    int target = (queue->front + 1) % queue->capacity;

    for(int i = 0; i < maxpage+1; i++)
    {
        // page Hit 인 경우
        if(queue->nodes[target].page == nextpage)
        {
            printf("\t\t\tNo replacement\n");
            printf("---------------------------------------------------------------------\n");
            if(save == 2) {
                fprintf(resultfp, "\t\t\tNo replacement\n");
                fprintf(resultfp, "---------------------------------------------------------------------\n");
            }
            // Hit 이므로 r bit = 1로 할당
            queue->nodes[target].referenceBit = 1;
            return 0;
        }
        target = (target + 1) % queue->capacity;
    }

    for(int i = 0; i < maxpage+1; i++)
    {
        target = (queue->front + 1) % queue->capacity;

        // 초기 pagefault
        if(queue->nodes[target].page < 0)
        {
            dequeue(queue);                 
            enqueue(queue, nextpage, 1);    // rbit = 1 setting
            printSC(queue);
            return 1;
        }
        // reference bit == 0인 경우 바로 내쫓음
        if(queue->nodes[target].referenceBit == 0)
        {
            dequeue(queue);
            enqueue(queue, nextpage, 1);    // rbit = 1 setting
            printSC(queue);
            return 1;
        }
        // reference bit == 1인 경우 큐에서 삭제 후 reference bit = 0 변환 후 삽입
        if (queue->nodes[target].referenceBit == 1)
        {
            int enqPage;        // 다시 큐에 넣어줄 page
            enqPage = queue->nodes[target].page;
            dequeue(queue);
            enqueue(queue, enqPage, 0);
            target++;
        }
    }
    return 1;
}

int sc(int *page, int maxpage)
{
    CircularQueue *queue;           // Queue
    int queueSize = maxpage + 1;    // queue size 정의
    int nextpage;                   // nest page stream
    int count = 0;                  // page fault

    // 큐 생성 및 초기화
    createCircularQueue(&queue, queueSize);
    for(int i = 0; i < maxpage; i++)
    {
        enqueue(queue, -1, 0);
    }

    printf("Page reference string\tFrames\t\tR-Bit\n");
    if(save == 2) fprintf(resultfp, "Page reference string\tFrames\t\tR-Bit\n");
    for(int i = 0; i < PAGENUM; i++)
    {
        int flag = 0;
        nextpage = page[i];
        printf("%d", nextpage);
        if(save == 2) fprintf(resultfp, "%d", nextpage);
        flag = checkSC(maxpage, nextpage, queue);
        if(flag > 0) count++;
    }

    freeQueue(queue);

    printf("\nSecond Change Algorithm Page Fault : %d\n", count);
    if(save == 2) fprintf(resultfp, "\nSecond Change Algorithm Page Fault : %d\n", count);

    return count;
}