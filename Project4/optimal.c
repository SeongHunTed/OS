#include "algorithm.h"

void printOpt(int maxpage,int pageframe[])
{
            int j;
            for(j=0; j<maxpage; j++){
                printf("%d\t",pageframe[j]);
                if (save == 2) fprintf(resultfp, "%d\t",pageframe[j]);
            }
            printf("\n");
            if (save == 2) fprintf(resultfp, "\n");
}    

// pageframe 안에 nextpage가 존재하는지 체크
// 존재하거나 pageframe에 처음들어가는 경우는 return 0
// 존재하지않으면 page fault 알리고 return 1
int checkOptimal(int maxpage, int nextpage, int *pageframe)
{   
    // 페이지 프레임 순회
    for (int i = 0; i < maxpage; i++)
    {   
        // Page Hit
        if(pageframe[i] == nextpage){
            printf("No Replacement\n");
            if (save == 2) fprintf(resultfp, "No Replacement\n");
            return 0;
        }

        // Empty Page Frame
        if (pageframe[i] == -1)
        {
            pageframe[i] = nextpage;
            printOpt(maxpage, pageframe);
            return 0;
        }
    }
    return 1;
}


// 쫒아내야될 아이를 찾는 과정
// return 값이 쫓아 내야할 page의 인덱스
int predict(int *page, int maxpage, int i, int *pageframe)
{   
    int distance[maxpage];  // 현재 프레임안에 있는 페이지 중에서 다음 입력의 거리를 배열에 저장
    int max = -1;           // 가장 먼 거리
    int replaceIndex;       // 교체대상 인데스

    // 거리 계산
    for(int k = 0; k < maxpage; k++)
    {   
        distance[k] = 0;
        // 이미 들어온 page 그 이후부터 수행
        for(int j = i+1; j < PAGENUM; j++)
        {
            if(pageframe[k] == page[j]){
                break;
            }
            distance[k]++;
        }
    }

    // distnace 배열 순회하며 가장 먼 page의 인덱스 검색
    for(int k = 0; k < maxpage; k++)
    {
        if(distance[k] > max){
            max = distance[k];
            replaceIndex = k;
        }
    }

    return replaceIndex;
}

int optimal(int *page, int maxpage)
{   
    int nextpage;           // 다음 페이지
    int index;              // replace 대상 인덱스
    int count = maxpage;    // 비어있을 초기에 pagefault
    int pageframe[maxpage]; // pageframe
    
    // pageframe initialize
    for(int i = 0; i < maxpage; i++)
    {
        pageframe[i] = -1;
    }

    printf("Page reference string\tFrames\n");
    if (save == 2) fprintf(resultfp, "Page reference string\tFrames\n");
    // 입력 스트일 개수에 따라 (해당과제에서 500)
    for (int i = 0; i < PAGENUM; i++)
    {   
        nextpage = page[i];
        printf("%d\t\t\t", nextpage);
        if (save == 2) fprintf(resultfp, "%d\t\t\t", nextpage);         // 출력결과 파일에 저장

        // page fault 발생한 경우 checkOptimal == 1
        if (checkOptimal(maxpage, nextpage, pageframe) > 0)
        {
            index = predict(page, maxpage, i, pageframe);               // 교체될 인덱스 검색
            pageframe[index] = nextpage;                                // 해당 페이지 교체
            count++;                                                    // page fault 증가
            printOpt(maxpage, pageframe);
        }
    }
    printf("\nOptimal Algorithm Page Fault Total : %d\n\n", count);
    if (save == 2) fprintf(resultfp, "\nOptimal Algorithm Page Fault Total : %d\n\n", count);

    return count;
}