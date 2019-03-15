/*************************************************************************
	> File Name: slave_1.c
	> Author: 
	> Mail: 
	> Created Time: Wed 19 Dec 2018 08:43:15 PM CST
 ************************************************************************/
#include"command.c"
typedef struct Arg {
    int long_fd, short_fd, heart_fd, alarm_fd;
} Arg;

Arg *arg;

Arg *init_arg() {
    Arg *p = (Arg *)malloc(sizeof(Arg));
    p->long_fd = 0;
    p->short_fd = 0;
    p->heart_fd = 0;
    p->alarm_fd = 0;
    return p;
}





//心跳
void *heart_func() {
    while (1) {

        arg->heart_fd= socket_connect(M_PORT, (char *)IP);
        if (arg->heart_fd > 0) close(arg->heart_fd);
        else {
            close(arg->heart_fd);
            sleep(2);
            continue;
        }
        sleep(2);
    }
}


//警报
/*如果存在warning，return 1, else return 0*/
void *warn_func() {
    while (1) {
        if (exist_Warning()) {
            arg->alarm_fd = socket_connect(M_PORT, (char *)IP);
            if (arg->alarm_fd > 0) {
                send_file(103, arg->alarm_fd);
                close(arg->alarm_fd);
            }
            else continue;
        }
        sleep(2);
    }
}


//识别码，数据发送
void *send_func() {
    //长连接发送识别码
    struct sockaddr_in m_addr;
    socklen_t len = sizeof(m_addr);
    int temp_fd, send_retcode, recv_retcode;
    char logpath[MAX_SIZE] = {0};
    strcpy(logpath, "./log");

    while (1) {
        
        if((temp_fd = accept(arg->long_fd, (struct sockaddr *)&m_addr, &len)) < 0) {
            perror("accept failed\n");
            sleep(2);
            continue;
        }
        for (int retcode = 100; retcode < 106; retcode++) {
            int temp_code;
            if (send(temp_fd, &retcode, 4, 0) <= 0) {
                perror("send");
                break;
            }
            if (recv(temp_fd, &temp_code, 4, 0) <= 0) {
                perror("recv");
                break;
            }
            //短连接发送数据
            arg->short_fd = socket_connect(M_PORT + 1, (char *)IP);
            printf("retcode = %d\n", retcode);
            if (send_file(retcode, arg->short_fd) >= 0) {
               printf("send success!!!\n");
               /*
                strcpy(logpath, get_log_path_client(retcode, logpath));
                if (retcode == 103) strcpy(logpath, "./log/Proc.log");
                if (retcode == 104) strcpy(logpath, "./log/System.log");
                if (remove(logpath) == 0) {
                    printf("rm %d.log success!!\n", retcode);
                } else printf("rm %d.log failed!!!\n", retcode);
            */
            }
            else {
                printf("----------send failed------------\n");
                close(arg->short_fd);
                break;
            }
            close(arg->short_fd);
            //sleep(2);
        }
	    close(temp_fd);
        sleep(2);
    }
}

void *run_func() {
    while (1) {
        
        for (int retcode = 100; retcode < 106; retcode++) {
            run_sh(retcode);
            sleep(2);
        }
        if (exist_Warning()) {
            system("cat ./log/tSystem.log > ./log/Proc.log");
            system("cat ./log/tProc.log >> ./log/Proc.log");
        }
        else {
            system("cat ./log/tSystem.log >> ./log/System.log");
            system("cat ./log/tProc.log >> ./log/Proc.log");
        }
        //sleep(2);
    }
}


int main() {

    arg = init_arg();
    pthread_t heart, alarm, runsh, sendlog;
    arg->long_fd = create_accept_fd(C_PORT + 1);
     
    //初始化本地log的路径
    init_get_path();
    
    if (pthread_create(&heart, NULL, heart_func, (void *)arg) < 0) {
        perror("create heart failed");
        exit(1);
    }
    if (pthread_create(&runsh, NULL, run_func, (void *)arg) < 0) {
        perror("Create_failed!\n");
        return -1;
    }
    if(pthread_mutex_init(&mutex, NULL) != 0) {  
        printf("Init metux error.\n");  
        exit(1);  
    }
    if (pthread_create(&sendlog, NULL, send_func, (void *)arg) < 0) {
        perror("Create_failed!\n");
        return -1;
    }
    if (pthread_create(&alarm, NULL, warn_func, (void *)arg) < 0) {
        perror("create alarm failed");
        exit(1);
    }

    pthread_join(heart, NULL);
    pthread_join(alarm, NULL);
    pthread_join(runsh, NULL);
    pthread_join(sendlog, NULL);

    return 0;
}

