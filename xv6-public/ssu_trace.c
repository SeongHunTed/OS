#include "types.h"
#include "user.h"
#include "stat.h"

int
main(int argc, char *argv[])
{
    printf(1, "ssu_trace working\n");
    
    int a = trace();

    printf(1, "trace : %d\n", a);
    
    exit();
}