#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char userID[16][32];
char pwdID[16][32];

void get_user_list() {

    int fd, i;

    if((fd = open("list.txt", O_RDONLY)) < 0){
      printf(1, "login : There is no file list.txt\n");
      exit();
    }

    for (i = 0; i < 15; i++) {
        // list.txt에서 Username, Password 정보를 userID, pwdID에 저장
        read(fd, userID[i], 5);
        userID[i][4] = '\0';
        read(fd, pwdID[i], 5);
        pwdID[i][4] = '\0';
    }

    close(fd);
}

int check_idpw() {
    // 입력받은 ID, PW 와 list.txt비교
    printf(1, "Username: ");
    char * username = gets("username", 32);
    username[4] = '\0';
    printf(1, "Password: ");
    char * password = gets("password", 32);
    password[4] = '\0';
    for (int i = 0; i < 15; i++) {
        if(strcmp(userID[i], username) == 0 && strcmp(pwdID[i], password) == 0) {
            return 1;
        }
    }

    return 0;
    
}

int main(int argc, char *argv[]) {

    get_user_list();
    while(1){
        if (check_idpw() == 1){
            exec("sh", argv);
        } else {
            printf(1, "Wrong ID or PW!!\n");
        }
    }

    return 0;
}
