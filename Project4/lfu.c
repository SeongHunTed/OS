#include "algorithm.h"

#define MAXPNUM 30

void printLFU(int maxpage,int pageframe[])
{
            int j;
            for(j=0; j<maxpage; j++){
                printf("%d\t",pageframe[j]);
                if(save == 2) fprintf(resultfp, "%d\t",pageframe[j]);
            }
            printf("\n");
            if(save == 2) fprintf(resultfp, "\n");
}    

// Hit면 0
// Fault면 1 반환
int checkLFU(int maxpage, int nextpage, int *pageframe)
{
    for (int i = 0; i < maxpage; i++)
    {
        if(pageframe[i] == nextpage){
            return 0;
        }
    }
    return 1;
}

int lfu(int *page, int maxpage)
{   
    int count = 0;                              // pagefault counter
    int nextpage, replaceIndex, minRefence;     // 다음 페이지, 교체될 페이지 프레임의 인덱스, 가장 적게 참조된 페이지
    int flag;               
    int pageframe[MAXPNUM];                     // 1~30 페이지 이므로 카운터도 30개 크기의 배열
    // page frame 초기화
    for(int i = 0; i < maxpage; i++)
        pageframe[i] = -1;
    
    // page counter 초기화
    int pageCounter[MAXPNUM];
    for (int i = 0; i < MAXPNUM; i++)
        pageCounter[i] = 0;

    printf("Page reference string\tFrames\n");
    if(save == 2) fprintf(resultfp, "Page reference string\tFrames\n");
    for(int i = 0; i < PAGENUM; i++)
    {
        flag = 0;
        nextpage = page[i];
        printf("%d\t\t\t", nextpage);
        if(save == 2) fprintf(resultfp, "%d\t\t\t", nextpage);
        flag = checkLFU(maxpage, nextpage, pageframe);

        // page hit
        if (flag == 0) {
            // 해당 페이지 카운터 증가
            pageCounter[nextpage-1]++;
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
        }

        // pagefault 처음에 비어있을 때
        if (flag == 1 && count < maxpage) {
            // pageframe이 아직 다 차지 않았는데 HIT가 발생했을 경우
            for(int j = 0; j < maxpage; j++){
                if(pageframe[j] == -1){
                    pageframe[j] = nextpage;
                    break;
                }
            }
            pageCounter[nextpage-1] = 1;
            count++;
            printLFU(maxpage, pageframe);
        } 
        else if (flag == 1)
        {
            replaceIndex = 0;                                    // 교체할 프레임 인덱스 초기화
            minRefence = pageCounter[pageframe[0]-1];            // 가장 적게 교체된 프레임 0번 인덱스의 counter 값으로
            for (int j = 1; j < maxpage; j++) {                  // 프레임내를 돌면서 가장 적게 참조된 페이지 검색
                if (pageCounter[pageframe[j]-1] < minRefence)
                {
                    replaceIndex = j;                            // 교체대상 선택
                    minRefence = pageCounter[pageframe[j]-1];    // minRefence update
                }
            }
            pageframe[replaceIndex] = nextpage;                  // frame의 교체대상 교체
            pageCounter[nextpage-1]++;                           // 교체한 페이지 카운터값 증가
            count++;                                             // page fault 증가
            printLFU(maxpage, pageframe);
        }

    }
    printf("\nLFU Alogrithm Page Fault : %d \n\n", count);
    if(save == 2) fprintf(resultfp, "\nLFU Alogrithm Page Fault : %d \n\n", count);
    return count;
}