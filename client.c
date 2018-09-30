/*************************************************************************
  > File Name: client.c
  > Author: 
  > Mail: 
  > Created Time: 2018年09月23日 星期日 16时18分26秒
 ************************************************************************/
 
#include "command.c"

int send_file(int recode, int sockfd) {
    FILE *fp;
    int size;
    char buffer[MAX_SIZE] = {0};
    char shpath[MAX_SIZE] = {"bash "}, logpath[MAX_SIZE] = {0};
    get_sh_path(recode, shpath);
    get_log_path(recode, logpath);
    //printf("shpath : %s\n", shpath);
    //DBG("logpath : %s\n", logpath);
    system(shpath);
    fp = fopen(logpath, "a+");
    while(fgets(buffer, MAX_SIZE - 1, fp)) {
        printf("send : %s\n", buffer);
        size = send(sockfd, buffer, strlen(buffer), 0);
    }
    fclose(fp);
    return size;
}

int main(int argc, char **argv) {
    int port = 6906;
    while (1) {
        int sock_fd = create_client_fd(port), size, count = 0;
        printf("connect_client ok\n");
        for (int recode = 100; recode < 103; recode++) {
            int temp_code;
            send(sock_fd, &recode, 4, 0);
            recv(sock_fd, &temp_code, 4, 0);
            int sock_data = create_client_fd(port + 1);
            send_file(recode, sock_data);
            close(sock_data);
        }
        close(sock_fd);
        usleep(5000000);//睡5s
    }
    return 0;
}

