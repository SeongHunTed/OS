#include "types.h"
#include "stat.h"
#include "user.h"

#define PNUM 5                      // process개수
#define PRINT_CYCLE 10000000        // process가 정보를 출력하는 주기
#define TOTAL_COUNTER 500000000     // process가 종료할 때 counter값

void sdebug_func(void)
{
    int n, pid;                     // 반복문 인자, 자식 프로세스 process id

    printf(1, "start sdebug command\n");

    // 명세에서 요구한대로 생성할 프로세스 개수룰 가준으로 반복문 수행
    // 반복문을 수행할 때 마다 fork()가 이루어진다
    // fork를 통해 생성된 자식 프로세스는 명세에서 요구한 두 조건에 따라 동작한다
    // 1. counter가 PRINT_CYCLE이면 프로세스 정보를 출력하고
    // 2. counter가 TOTAL_COUNTER값이 되면 프로세스를 종료한다
    // 자식프로세스가 종료될 때까지 부모 프로세스는 wait()
    for(n = 0; n < PNUM; n++) {

        pid = fork();                   // 자식 프로세스 생성
        
        if(pid < 0){                    // fork 실패
            printf(1, "fork failed!\n");
            break;
        }
        // 자식 프로세스일 경우
        if(pid == 0)
        {
            int starttime = uptime();   // 자식 프로세스 생성 후 tick 저장
            int endtime = 0;            // print 상태에서의 tick 값

            int weight = n+1;           // 가중치는 프로세스 생성된 순서대로 증가한다
            weightset(weight);          // system call weightset()으로 가중치를 프로세스에 반영

            int counter = 0;            // 프로세스 종료 조건의 counter값
            int print_counter = 0;      // 프로세스 정보 출력 조건의 counter 값

            int flag = 1;               // 한 번만 출력하기 위한 flag

            while(1)
            {
                counter++;              // 종료 조건 카운터
                print_counter++;        // 출력 조건 카운터

                if(counter == TOTAL_COUNTER)                        // 프로세스 종료 조건
                {
                    printf(1, "PID : %d, terminated\n", getpid());  // 프로세스 종료와 함께 출력
                    exit();
                }
                if(print_counter == PRINT_CYCLE)                    // 프로세스 정보 출력 조건
                {
                    if(flag)
                    {
                        endtime = uptime();                         // 출력 시간 측정
                        printf(1, "PID : %d, WEIGHT : %d, TIMES : %dms\n", getpid(), weight, 10*(endtime-starttime));
                        flag = 0;                                   // flag = 0으로 재출력 방지
                    }
                    print_counter = 0;
                }

            }
        }
    }

    // 부모 프로세스는 모든 자식 프로세스가 종료될 때 까지 wait
    for(; n > 0; n--){
        if(wait() < 0){     // wait()이 너무 빨리 끝났을 때
            printf(1, "wait stopped early\n");
            exit();
        }
    }
    printf(1, "end of sdebug command\n");
}

int main(void)
{
    sdebug_func();
    exit();
}
