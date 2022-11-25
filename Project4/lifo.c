#include "algorithm.h"

void printLIFO(int maxpage,int pageframe[])
{
            int j;
            for(j=0; j<maxpage; j++){
                printf("%d\t",pageframe[j]);
                if(save == 2) fprintf(resultfp, "%d\t",pageframe[j]);
            }
            printf("\n");
            if(save == 2) fprintf(resultfp, "\n");
}    


int checkLifo(int maxpage, int nextpage, int *pageframe)
{
    for (int i = 0; i < maxpage; i++)
    {
        // Hit
        if(pageframe[i] == nextpage){
            printf("No replacement\n");
            if(save == 2) fprintf(resultfp, "No replacement\n");
            return 0;
        }

        if(pageframe[i] == -1){
            pageframe[i] = nextpage;
            printLIFO(maxpage, pageframe);
            return 0;           // page fault -> pageframe이 비어있을 때
        }
    }
    return 1;                   // page fault
}

int lifo(int *page, int maxpage)
{
    int nextpage;                   // 다음 페이지
    int flag;                       // 히트, 폴트 플래그
    int count = maxpage;            // page fault counter
    int pageframe[maxpage];         // page frame 
    for(int i = 0; i < maxpage; i++)
    {
        pageframe[i] = -1;
    }

    printf("Page reference string\tFrames\n");
    if(save == 2) fprintf(resultfp, "Page reference string\tFrames\n");
    for (int i = 0; i < PAGENUM; i++)
    {
        nextpage = page[i];
        printf("%d\t\t\t", nextpage);
        if(save == 2) fprintf(resultfp, "%d\t\t\t", nextpage);
        flag = checkLifo(maxpage, nextpage, pageframe);

        if (flag > 0) {
            pageframe[maxpage-1] = nextpage;
            count++;
            printLIFO(maxpage, pageframe);
        }
    }

    printf("\nLIFO Algorithm Page Fault : %d \n\n", count);
    if(save == 2) fprintf(resultfp, "\nLIFO Algorithm Page Fault : %d \n\n", count);

    return count;
}