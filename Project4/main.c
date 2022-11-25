#include "makePage.h"
#include "algorithm.h"

int main()
{
    while(1)
    {   
        int exitFlag = user();             // -1 : Program Exit 0 : Wrong Input, 2 : All Algorithm
        if(exitFlag == -1){
            printf("프로그램 종료 !\n");
            return 0;
        }
    }
    return 0;
}