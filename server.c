/*************************************************************************
> File Name: server.c
> Author: 
> Mail: 
> Created Time: 2018年09月30日 星期日 20时20分11秒
************************************************************************/

#include"command.c"
void get_file(int sock, int retcode, char *ip) {
    int size = 0;
    char data[MAX_SIZE] = {0};
    char logdir[MAX_SIZE] = {0};
    strcpy(logdir, "./log/");
    strcat(logdir, ip);
//开始锁
    mkdir(logdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    char logpath[MAX_SIZE] = {0};
    strcpy(logpath, logdir);
    get_log_path(retcode, logpath);
    FILE *fp = fopen(logpath, "a+");
    while((size = recv(sock, data, MAX_SIZE, 0)) > 0) {
        //printf("data : %s\n", data);
        fwrite(data, 1, size, fp);
        memset(data, 0, sizeof(data));

    }
    fclose(fp);
//结束锁
}

int main() {
    int port = 6906;
    int long_fd = create_server_fd(port);
    int short_fd = create_server_fd(port + 1);
    int pid, sockfd, count;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len;
        if ((sockfd = accept(long_fd, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("Accept");
            return -1;

        }
        char ip[MAX_SIZE] = {0};
            close(long_fd);
            memset(ip, 0, sizeof(ip));
            for (int i = 0; i < 3; i++) {
                int retcode, socket_data;
                recv(sockfd, &retcode, 4, 0);
                send(sockfd, &retcode, 4, 0);
                if ((socket_data = accept(short_fd, (struct sockaddr *)&client_addr, &len)) < 0) {
                    perror("Accept");
                    return -1;
                }
                strcpy(ip, inet_ntoa(client_addr.sin_addr));
                
                get_file(socket_data, retcode, ip);
                close(socket_data);
            }
            printf( "IP：%s已完成%d次发送", ip, count++);
            exit(1);//结束进程
    }
    close(long_fd);
    close(sockfd);
    return 0;

}

