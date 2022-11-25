#include "makePage.h"
#include "algorithm.h"

FILE *resultfp;     // File 저장
int save = 1;       // 표준출력

int user()
{   
    // A. 알고리즘 선택 최대 3개, 
    int flag = 0;
    char inputBuffer[20];
    int simulator[4] = {-1, -1, -1, -1};
    printf("=============================================================================================\n");
    printf("A. Page Replacement 알고리즘 시뮬레이터를 선택하시오 (최대 3개)\n\n");
    printf("(1) Optimal  (2) FIFO  (3) LIFO  (4) LRU  (5) LFU  (6) SC  (7) ESC  (8) ALL  (9) Program Exit\n\n");

    printf("User Input : ");
    rewind(stdin);
    fgets(inputBuffer, sizeof(inputBuffer), stdin);
    if(inputBuffer[strlen(inputBuffer)-1] == '\n')
        inputBuffer[strlen(inputBuffer)-1] = '\0';
    int count =sscanf(inputBuffer, "%d %d %d %d", &simulator[0], &simulator[1], &simulator[2], &simulator[3]);
    if (count > 3 || count < 1)
    {
        printf("Error : Usage - <최소 1개 - 최대 3개 정수 입력>\n");
        return 0;
    }

    for (int i = 0; i < count; i++){
        if(simulator[i] == 9) return -1;

        if(simulator[i] > 9 || simulator[i] < 1)
        {
            printf("Error : Usage - <1~9 사이의 정수 입력>\n");
            return 0;
        }

        if(simulator[i] == 8){
            flag = 1;
        }
    }

    // B. C 프레임 개수와 데이터 입력 방식 설정
    int maxpage;                        // frame 개수
    printf("=============================================================================================\n");
    printf("B. 페이지 프레임의 개수를 입력하시오. (3~10)\n\nUser Input : ");
    while(1){
        scanf("%d", &maxpage);
        if(maxpage > 10 || maxpage < 3){
            printf("Error : Usage - <3~10 사이의 정수 입력>\nUser Input : ");
        } else break;
    }

    int inputMethod;                    // 데이터 입력 방식  1 : 랜덤 생성 , 2 : 사용자 파일 오픈
    printf("=============================================================================================\n");
    printf("C. 데이터 입력 방식을 선택하시오.\n [1] : 랜덤 생성\n [2] : 사용자 생성 파일 오픈\n\nUser Input : ");
    while(1){
        scanf("%d", &inputMethod);
        if(inputMethod != 1 && inputMethod != 2){
            printf("Error : Usage - <1 or 2 정수 입력>\nUser Input : ");
        } else break;
    }
    printf("=============================================================================================\n");

    FILE *fp;
    char pageName[30];
    int *pagestream;

    // 랜덤 생성
    if (inputMethod == 1)
    {
        pagestream = randomPage();
        printf("Random Page Stream\n");
        printf("\n=============================================================================================\n");
        printf("\n\n");
    } 
    else if (inputMethod == 2)
    {
        while(1)
        {
            memset(pageName, (char)0xff, sizeof(pageName));
            printf("사용자 생성 파일을 입력 하세요.\n파일이름 : ");
            scanf("%s", pageName);                 // Ubuntu에서 어떻게 동작 하는 지 확인
            if(access(pageName, F_OK) < 0) {
                printf("존재하지 않는 파일입니다. Usage : <파일이름.확장자>\n\n");
            } else {
                printf("%s 를 엽니다 \n", pageName);
                pagestream = readPage(fp, pageName);
                break;
            }
        }
    }

    FILE *escfp;
    int escflag = 0;
    char escFileName[30];
    int *escpageStream;

    for(int i = 0; i < count; i++)
    {
        if(simulator[i] == 7 || simulator[i] == 8)
        {
            printf("ESC 시뮬레이터를 위한 파일이 있습니까?\n [1] : No -> Random Stream\n [2] : Yes\n\nUser Input : ");
            scanf("%d", &escflag);
            if(escflag == 1)
            {
                int *wrBit = escPage();
                int stream[PAGENUM*2];
                escpageStream = makePageESC(stream, pagestream, wrBit);
                break;
            } else if(escflag == 2){
                memset(escFileName, (char)0xff, sizeof(escFileName));
                printf("사용자 생성 ESC 파일을 입력 하세요.\n파일이름 : ");
                scanf("%s", escFileName);                 // Ubuntu에서 어떻게 동작 하는 지 확인
                if(access(escFileName, F_OK) < 0) {
                    printf("존재하지 않는 파일입니다. Usage : <파일이름.확장자>\n\n");
                } else {
                    printf("%s 를 엽니다 \n", escFileName);
                    int *wrBit = readEscPage(escfp, escFileName);
                    int stream[PAGENUM*2];
                    escpageStream = makePageESC(stream, pagestream, wrBit);
                    break;
                }
            } else {
                printf("Error : Usage - <1 or 2 정수 입력>\nUser Input : ");
            }
        }
    }
    printf("=============================================================================================\n");

    char *fileName = "result.txt";
    printf("D. 결과를 저장하시겠습니까? 현재 경로에 result.txt로 저장됩니다.\n [1] : 단순 출력\n [2] : 표준 출력 및 저장\n\nUser Input : ");
    while(1){
        scanf("%d", &save);
        if(save != 1 && save != 2)
            printf("Error Usage - <1 or 2 정수 입력>\nUser Input : ");
        else break;
    }
    if(save == 2)
        resultfp = fopen(fileName, "w");
    printf("=============================================================================================\n");
    
    int optimalCount, fifoCount, lifoCount, lruCount, lfuCount, scCount, escCount;

    if (flag == 1)
    {   
        if(save == 2) fprintf(resultfp, "Page Stream\n\n");
        for (int i = 0; i < PAGENUM; i++){
            printf("%d ", pagestream[i]);
            if(((i+1)%40) == 0) printf("\n");
            if (save == 2) {
                fprintf(resultfp, "%d ", pagestream[i]);
                if(((i+1)%20) == 0) fprintf(resultfp, "\n");
            }
        } 
        if(save == 2) fprintf(resultfp, "---------------------------------------------------------------------\n");
        printf("\n=============================================================================================\n");
        optimalCount = optimal(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        fifoCount = fifo(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        lifoCount = lifo(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        lruCount = lru(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        lfuCount = lfu(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        scCount = sc(pagestream, maxpage);
        printf("\n=============================================================================================\n");
        printf("Esc Stream\n");
        if(save == 2) fprintf(resultfp, "ESC Stream\n");
        
        for(int i = 0; i < PAGENUM; i++)
        {
            if(escpageStream[i*2+1] == 0) printf("[%d R] ", escpageStream[i*2]);
            else printf("[%d W] ", escpageStream[i*2]);
            if(((i+1)%20) == 0) printf("\n");
            if(save == 2) {
                if(escpageStream[i*2+1] == 0) fprintf(resultfp, "[%d R] ", escpageStream[i*2]);
                else fprintf(resultfp, "[%d W] ", escpageStream[i*2]);
                if(((i+1)%10) == 0) fprintf(resultfp, "\n");
            }
        }
        printf("\n=============================================================================================\n");
        escCount = esc(escpageStream, maxpage);
        printf("\n\n=============================================================================================\n");
        printf("Algorithm\tPage Fault\n");
        printf("Optimal\t\t%d\n", optimalCount);
        printf("FIFO\t\t%d\n", fifoCount);
        printf("LIFO\t\t%d\n", lifoCount);
        printf("LRU\t\t%d\n", lruCount);
        printf("LFU\t\t%d\n", lfuCount);
        printf("SC\t\t%d\n", scCount);
        printf("ESC\t\t%d\n", escCount);
        if(save == 2)
        {
            fprintf(resultfp, "Algorithm\tPage Fault\n");
            fprintf(resultfp, "Optimal\t\t%d\n", optimalCount);
            fprintf(resultfp, "FIFO\t\t%d\n", fifoCount);
            fprintf(resultfp, "LIFO\t\t%d\n", lifoCount);
            fprintf(resultfp, "LRU\t\t%d\n", lruCount);
            fprintf(resultfp, "LFU\t\t%d\n", lfuCount);
            fprintf(resultfp, "SC\t\t%d\n", scCount);
            fprintf(resultfp, "ESC\t\t%d\n\n", escCount);
        }
    }
    else
    {
        if(save == 2) fprintf(resultfp, "Page Stream\n\n");
        for (int i = 0; i < PAGENUM; i++){
            printf("%d ", pagestream[i]);
            if(((i+1)%40) == 0) printf("\n");
            if (save == 2) {
                fprintf(resultfp, "%d ", pagestream[i]);
                if(((i+1)%20) == 0) fprintf(resultfp, "\n");
            }
        }
        if(save == 2) fprintf(resultfp, "---------------------------------------------------------------------\n");
        for(int i = 0 ; i < 3; i++)
        {
            int target = simulator[i];
            printf("\n=============================================================================================\n");
            if(target == 1) optimalCount = optimal(pagestream, maxpage);
            if(target == 2) fifoCount = fifo(pagestream, maxpage);
            if(target == 3) lifoCount = lifo(pagestream, maxpage);
            if(target == 4) lruCount = lru(pagestream, maxpage);
            if(target == 5) lfuCount = lfu(pagestream, maxpage);
            if(target == 6) scCount = sc(pagestream, maxpage);
            if(target == 7){
                printf("Esc Stream\n");
                if(save == 2) fprintf(resultfp, "ESC Stream\n");
                for(int i = 0; i < PAGENUM; i++)
                {
                    if(escpageStream[i*2+1] == 0) printf("[%d R] ", escpageStream[i*2]);
                    else printf("[%d W] ", escpageStream[i*2]);
                    if(((i+1)%20) == 0) printf("\n");
                    if(save == 2) {
                        if(escpageStream[i*2+1] == 0) fprintf(resultfp, "[%d R] ", escpageStream[i*2]);
                        else fprintf(resultfp, "[%d W] ", escpageStream[i*2]);
                        if(((i+1)%10) == 0) fprintf(resultfp, "\n");
                    }
                }
                printf("\n");
                printf("=============================================================================================\n");
                escCount = esc(escpageStream, maxpage);
            }
        }
        printf("\n=============================================================================================\n");
        printf("Algorithm\tPage Fault\n");
        for(int i = 0; i < 3; i++)
        {
            int target = simulator[i];
            if(target == 1) {
                printf("Optimal\t\t%d\n", optimalCount);
                if(save == 2) fprintf(resultfp, "Optimal\t\t%d\n", optimalCount);
            }
            if(target == 2) {
                printf("FIFO\t\t%d\n", fifoCount);
                if(save == 2) fprintf(resultfp, "FIFO\t\t%d\n", fifoCount);
            }
            if(target == 3) {
                printf("LIFO\t\t%d\n", lifoCount);
                if(save == 2) fprintf(resultfp, "LIFO\t\t%d\n", lifoCount);
            }
            if(target == 4) {
                printf("LRU\t\t%d\n", lruCount);
                if(save == 2) fprintf(resultfp, "LRU\t\t%d\n", lruCount);
            }
            if(target == 5) {
                printf("LFU\t\t%d\n", lfuCount);
                if(save == 2) fprintf(resultfp, "LFU\t\t%d\n", lfuCount);
            }
            if(target == 6) {
                printf("SC\t\t%d\n", scCount);
                if(save == 2) fprintf(resultfp, "SC\t\t%d\n", scCount);
            }
            if(target == 7) {
                printf("ESC\t\t%d\n", escCount);
                if(save == 2) fprintf(resultfp, "ESC\t\t%d\n\n", escCount);
            }
        }
    }
    printf("\n\n");
    fclose(resultfp);
    return 1;
}