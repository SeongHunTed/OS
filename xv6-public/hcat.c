#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];
int target;

void
hcat(int fd)
{
  int n;
  int count = 0;

  // 여기서 read와 print 동시에
  while((n = read(fd, buf, 1)) > 0) {

    if (buf[0] == '\n') {
      count++;
    }

    if (count == target) {
      printf(1, "\n");
      exit();
    }
    
    if (write(1, buf, n) != n) {
      printf(1, "hcat: write error\n");
      exit();
    }

    if(n < 0){
      printf(1, "hcat: read error\n");
      exit();
    }
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  target = atoi(argv[1]);
  
  if (target <= 0) {
    printf(1, "hcat : wrong option!\n");
    exit();
  }

  if(argc <= 1){
    hcat(0);
    exit();
  }

  for(i = 2; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "hcat: cannot open %s\n", argv[i]);
      exit();
    }
    hcat(fd);
    close(fd);
  }
  exit();

}
