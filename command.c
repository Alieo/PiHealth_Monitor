/*************************************************************************
> File Name: command.c
> Author: 
> Mail: 
> Created Time: 2018年09月28日 星期五 18时55分51秒
************************************************************************/
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#define IP "192.168.1.99"
#define MAX_SIZE 10000
#define M_PORT 6906
#define C_PORT 2153

/*
#define OPEN_DBG 1
#ifdef OPEN_DBG 
#define DBG(format,...) printf(""__FILE__" : %s : %05d==>"format"\n", __func__, __LINE__,##__VA_ARGS__)
#else
#define DBG(s) 1
#endif
*/
//互斥锁系统原型为: pthread_mutex_t, 在master端使用
pthread_mutex_t mutex;

//master: 创建connect用套接字
int create_connect_fd(int port, char *ip, struct sockaddr_in sock_addr) {
    int sock_fd;
    srand(time(0));
    //struct sockaddr_in sock_addr;
    printf("%d\n", rand() % 100);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);
    /*
    if (inet_pton(AF_INET, ip, &sock_addr.sin_addr) <= 0) {
        perror("connect_inet_pton error");
        return -1;
    }
    */
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("co_sock_fd");
        return -1;
    }
    if (connect(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("co_connect failed");
        return -1;
    }
    return sock_fd;
}

//client端: 单纯的只是做连接
int socket_connect(int port, char *host) {
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        perror("co_socket_create ");
        close(socket_fd);
        return -1;
    }
    struct sockaddr_in dest_addr ;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(host);
    
    if (connect(socket_fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("co_connect failed"); 
        return -1;
    }
    return socket_fd;
}

//master / client: 创建accept用套接字
int create_accept_fd(int port){
    int socket_fd;
    //创建套接字
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ac_create failed");
        return -1;
    }
    struct sockaddr_in sever_addr;
    //设置本地套接字地址
    sever_addr.sin_family = AF_INET;
    sever_addr.sin_port = htons(port);
    sever_addr.sin_addr.s_addr = INADDR_ANY;//任意网卡ip
    //绑定本地套接字到套接字
    if (bind(socket_fd, (struct sockaddr *)&sever_addr, sizeof(sever_addr)) < 0) {
        perror("ac_bind failed");
        return -1;
    }
    if (listen(socket_fd, 20) < 0) {
        perror("ac_listen failed");
        return -1;
    }
    /*
    struct sockaddr_in socket_addr;
    if ((socket_data = accept(socket_fd, (struct sockaddr *)&socket_addr, &len)) < 0) {
        perror("Accept");
        return -1;
    } 
    */
    return socket_fd;
}

//初始化思路: 提取配置文件信息, ip从40~49插入链表, connect, 如果连不上, erase.
//如果心跳收到connect,并且链表中没有, 则加入到链表中.
/*
void init(Arg *p) {
    //char value[20] = {0};
    char prename[20] = {0}; 
    get_conf_value("./PiHealth.conf", "prename", prename);
    char start[20] = {0}; 
    get_conf_value("./PiHealth.conf", "start", start);
    char finish[20] = {0}; 
    get_conf_value("./PiHealth.conf", "finish", finish);
    for (int i = atoi(start); i <= atoi(finish); i++) {
        char temp[20] = {0};
        strcpy(temp, prename);
        sprintf(temp + strlen(temp), ".%d", i);
        struct sockaddr_in t_addr;
    
        t_addr.sin_family = AF_INET;
        t_addr.sin_port = htons(C_PORT);
        t_addr.sin_addr.s_addr = inet_addr(temp);
        insert(l, temp, t_addr);
        printf("init_insert_unlock\n");
    }
    //output(p->l);
}
*/
//client获取shell路径字符串
void get_sh_path(int retcode, char *shpath) {
    strcpy(shpath, "./shell/");
    switch (retcode) {
        case 100: strcat(shpath, "Cpu.sh"); break;
        case 101: strcat(shpath, "Disk.sh"); break;
        case 102: strcat(shpath, "Mem.sh"); break;
        case 103: strcat(shpath, "Proc.sh"); break;
        case 104: strcat(shpath, "System.sh"); break;
        case 105: strcat(shpath, "Usrs.sh"); break;
    }
}

//server获取.log文件路径字符串
char *get_log_path(int retcode, char *logpath) {
    switch (retcode) {
        case 100: strcat(logpath, "/Cpu.log"); break;
        case 101: strcat(logpath, "/Disk.log"); break;
        case 102: strcat(logpath, "/Mem.log"); break;
        case 103: strcat(logpath, "/Proc.log"); break;
        case 104: strcat(logpath, "/System.log"); break;
        case 105: strcat(logpath, "/Usrs.log"); break;
    }
    return logpath;
}
//client获取.log文件路径字符串
char *get_log_path_client(int retcode, char *logpath) {
    switch (retcode) {
        case 100: strcat(logpath, "/Cpu.log"); break;
        case 101: strcat(logpath, "/Disk.log"); break;
        case 102: strcat(logpath, "/Mem.log"); break;
        case 103: strcat(logpath, "/tProc.log"); break;
        case 104: strcat(logpath, "/tSystem.log"); break;
        case 105: strcat(logpath, "/Usrs.log"); break;
    }
    return logpath;
}
//读取配置文件
/*prename, start, finish*/
void get_conf_value(char *pathname, char *key_name, char *value) {
    FILE *fp;
    char *p;
    char buffer[MAX_SIZE] = {0};
    fp = fopen(pathname, "r");
    while (fscanf(fp, "%s", buffer) != EOF){
        if (strstr(buffer, key_name) != NULL) {
            p = strchr(buffer, '=');
            strcat(value, p + 1);
            break;
            //等号右边第一个字符串是p+1的首地址, p+1指向首地址为它的一串连续字符串   
        }
    }
    fclose(fp);
}


//向文件写日志
int write_Pi_log(char *PiHealthLog, const char *format, ...) {
    FILE *fp;
    fp = fopen(PiHealthLog, "a+");
    char buffer[MAX_SIZE] = {0};
    time_t t;
    struct tm * lt;
    time(&t);//获取Unix时间戳。
    lt = localtime(&t);//转为时间结构。
    sprintf(buffer, "%d/%d/%d %d:%d:%d\t",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    fprintf(fp, "%s: ", buffer);
    int cnt = 0;
    va_list arg;
    va_start(arg, format);
    for (int i = 0; format[i]; i++) {
        switch (format[i]) {
            case '%' : {
                i++;
                switch (format[i]) {
                    case 's' : {
                        char *ret = va_arg(arg, char *);
                        fputs(ret, fp);
                        cnt = cnt + strlen(ret);
                    } break;
                    default :
                    fprintf(stderr, "error : unknow %%%c\n", format[i]);
                    exit(1);
                }
            } break;
            default :
            fputc(format[i], fp);
            cnt++;
        }
    }
    return cnt;
}

/*
//获取文件大小

void file_size(char *filename, long int size, char *dir) {
    FILE *fp;
    int flag;
    size = size * 1024 *1024
    long int f_size;
    fp=fopen( "filename", "r");
    fseek(fp, 0L, SEEK_END );
    f_size = ftell(fp); 
    if (f_size >= size) {
        //打包压缩文件, 放到dir中
        flag = 0;
    } else return 0;
    
    char cmd_1[50] = {0};
    char cmd_2[100] = {0};
    sprintf(cmd_1, "basename %s", filename);
}
*/
////判断是否存在warning
int exist_Warning() {
    int ans = 0;
    char logpath[MAX_SIZE] = {0};
    strcpy(logpath, "./log/tSystem.log");
    //如果logpath不存在则返回0
    //access()函数判断文件是否存在
    if (access(logpath, 0) < 0) ans = 0;
    /*如果存在warning，return 1, else return 0*/
    else {
        char Disk[20] = {0}; 
        get_conf_value(logpath, "Disk", Disk);
        char Mem[20] = {0}; 
        get_conf_value(logpath, "Mem", Mem);
        char Temp[20] = {0}; 
        get_conf_value(logpath, "Temp", Temp);
        if (strcmp(Disk, "warning") == 0 || strcmp(Mem, "warning") == 0 || strcmp(Temp, "warning") == 0 ) ans = 1;
        else ans = 0;
    }
    return ans;
}
//本地log路径初始化
void init_get_path() {
    char logpath[MAX_SIZE] = {0};
    for (int i = 100; i < 106; i++) {
        strcpy(logpath, "./log");
        strcpy(logpath, get_log_path_client(i, logpath));
        FILE *fp = fopen(logpath, "a+");
        fclose(fp);
        if (i == 103) {
            strcpy(logpath, "./log/Proc.log");
            FILE *fp1 = fopen(logpath, "a+");
            fclose(fp1);
        }
        if (i == 104) {
            strcpy(logpath, "./log/System.log");
            FILE *fp2 = fopen(logpath, "a+");
            fclose(fp2);
        }
        sleep(1);
    }
    return ;
}

//运行脚本
void run_sh(int retcode) {
    char shpath[MAX_SIZE] = {"bash "};
    //获取所要运行脚本的路径
    get_sh_path(retcode, shpath);
    //printf("shpath : %s\n", shpath);
    //printf("logpath : %s\n", logpath);
    system(shpath);
    //如果他是warning，将System和Proc的运行结果从临时文件放到Proc.log
    /*
    if (retcode == 103 || retcode == 104) {
        if (exist_Warning()) {
            system("cat ./log/tSystem.log > ./log/Proc.log");
            system("cat ./log/tProc.log >> ./log/Proc.log");
        }
        else {
            system("cat ./log/tSystem.log >> ./log/System.log");
            system("cat ./log/tProc.log >> ./log/Proc.log"); 
        }
    }
    */
    return ;
}
//发送
//打开本地log日志，将log日志中的内容读取出来发送给master端
int send_file(int retcode, int sockfd) {
    FILE *fp;
    int size;
    char buffer[MAX_SIZE] = {0};
    char logpath[MAX_SIZE] = {0};
    strcpy(logpath, "./log");
    strcpy(logpath, get_log_path_client(retcode, logpath));
    /*
    if (retcode == 104) {
        fp = fopen(logpath, "r");
        while (fgets(buffer, MAX_SIZE - 1, fp)) {
            size = send(sockfd, buffer, strlen(buffer), 0);
            
        }
        fclose(fp);
    }
    */
   // else {
        if (retcode == 103) strcpy(logpath, "./log/Proc.log");
        if (retcode == 104) strcpy(logpath, "./log/System.log");
        int mut = pthread_mutex_lock(&mutex);
        if (mut != 0) {
            printf("lock error\n");
            fflush(stdout);
        }
        fp = fopen(logpath, "r");
        while (fgets(buffer, MAX_SIZE - 1, fp)) {
            size = send(sockfd, buffer, strlen(buffer), 0); 
            printf("send : %s\n", buffer);
        } 
        fclose(fp);    
        pthread_mutex_unlock(&mutex); 
        if (remove(logpath) == 0) {
            printf("rm %d.log success!!\n", retcode);
        } else printf("rm %d.log failed!!!\n", retcode);  
        init_get_path();
    // }
     
    return size;
}

//master端收到标识码后的准备
char *create_dir(int retcode, char *ip) {
    char logdir[MAX_SIZE] = {0};
    //将master存放log日志的路径设为：./log/对端ip
    strcpy(logdir, "./log/");

    strcat(logdir, ip);
    /*
    int mut = pthread_mutex_lock(&mutex);
    if (mut != 0) {
        printf("lock error\n");
        fflush(stdout);
    }
    */
    //将./log/对端ip设置为目录
    mkdir(logdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    char *logpath = (char *)malloc(sizeof(char) * MAX_SIZE);
    strcpy(logpath, logdir);
    //在所得到的新目录下创建各个脚本相应的.log日志
    strcpy(logpath, get_log_path(retcode, logpath));
    return logpath;
}

//接收
int get_file(char *logpath, int sock) {
    int size = 0, ans = 0;
    char data[MAX_SIZE] = {0};
    
    /*
    int mut = pthread_mutex_lock(&mutex);
    if (mut != 0) {
        printf("lock error\n");
        fflush(stdout);
    } 
    */
    FILE *fp = fopen(logpath, "a+");
    while ((size = recv(sock, data, MAX_SIZE, 0)) > 0) {
        printf("data : %s\n", data);
        fwrite(data, 1, size, fp);
        memset(data, 0, sizeof(data));
        ans = 1;
    }
    fclose(fp);
    //pthread_mutex_unlock(&mutex);
    return ans;
}
/*
//将ip写进log日志中
void write_ip_to_log(struct sockaddr_in client_addr, char *logpath) {
    char client_ip[MAX_SIZE] = {0};
    if (inet_ntop(AF_INET, (void *)&(client_addr.sin_addr), client_ip, 16) == NULL) {
        perror("parsing ip error");
    }
    FILE *fp = fopen(logpath, "a+");
    fprintf(fp, "%s", client_ip);
    fclose(fp);
}
*/
char *create_warn_dir(char *ip) {
    char logdir[MAX_SIZE] = {0};
    //将master存放log日志的路径设为：./log/对端ip
    strcpy(logdir, "./log/");
    strcat(logdir, ip);
    /*
    int mut = pthread_mutex_lock(&mutex);
    if (mut != 0) {
        printf("lock error\n");
        fflush(stdout);
    }
    */
    //将./log/对端ip设置为目录
    mkdir(logdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    char *logpath = (char *)malloc(sizeof(char) * MAX_SIZE);
    strcpy(logpath, logdir);
    //在所得到的新目录下创建各个脚本相应的.log日志
    strcpy(logpath, "/Warn.log");
    return logpath;
}

