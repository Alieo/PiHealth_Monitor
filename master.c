/*************************************************************************
> File Name: master.c
> Author: 
> Mail: 
> Created Time: Tue 13 Nov 2018 08:22:48 PM CST
************************************************************************/
#include"command.c"
/*
#include <pthread.h>
pthread_mutex_t m_mutex;
pthread_mutex_init(&m_Mutex, NULL);
 
int nRet = pthread_mutex_lock(&m_Mutex);
…… // do something
nRet = pthread_mutex_unlock(&m_Mutex);
 
pthread_mutex_destroy(&m_Mutex); // 销毁
 */
//全局变量: 互斥锁, M_PORT 
/*
typedef struct Node {
    char *data;
    struct sockaddr_in sock_addr;
    struct Node *next;
} Node, *Link;
*/
typedef struct LinkNode {
    char *data;
    struct sockaddr_in sock_addr;
    struct LinkNode *next;
} LinkNode;
typedef struct LinkList {
    LinkNode head;//虚拟节点
    //int length;
} LinkList;
typedef struct Arg {
    //heart_fd: 心跳端口  long_fd：标识码传输端口  short_fd: 数据端口　alarm_fd: 警报信息端口
    int long_fd, short_fd, heart_fd, alarm_fd;
    LinkNode *l;
} Arg;

Arg *arg;
char ip[MAX_SIZE] = {0};
LinkList *l ;


LinkList *init_link() {
    LinkList *p = (LinkList *)malloc(sizeof(LinkList));
    p->head.next = NULL;
    return p;
}
Arg *init_arg(LinkList *l) {
    Arg *p = (Arg *)malloc(sizeof(Arg));
    p->long_fd = 0;
    p->short_fd = 0;
    p->heart_fd = 0;
    p->l = &(l->head);
    return p;
}
LinkNode *getNewNode(char *val, struct sockaddr_in sock_addr) {
    LinkNode *n = (LinkNode *)malloc(sizeof(LinkNode));
    n->data = strdup(val); 
    n->sock_addr = sock_addr;
    n->next = NULL;
    return n;
}







//插入链表
void insert(LinkList *l, char *val, struct sockaddr_in sock_addr) {
    LinkNode *p = &(l->head);
    
    while (p->next) {
        if (p->data != NULL && strcmp(p->data, val) == 0) return ; 
        p = p->next;
    }
    LinkNode *new_node = getNewNode(val, sock_addr);
    //new_node->next = p->next;
    printf("---------------------------------------------\n");
    p->next = new_node;
    printf("insert %s 到链表里\n", new_node->data);
    //pthread_mutex_unlock(&mutex);
    return ;
}





//删除链表节点
void erase(LinkNode *delete) {
    if (delete == NULL) return ;
    printf("delete = %s\n", delete->data);
    free(delete->data);
    free(delete);
    return ;
}





void output(LinkList *l) {
    LinkNode *p = l->head.next;
    while (p != NULL) {
        printf("%s->", p->data);
        p = p->next;
    }
    printf("NULL\n");
    return ;
}





//处理任务
void deal_task(Arg *p, char *client_ip) {
    char path[MAX_SIZE] = {0};
    for (int i = 0; i < 6; i++) {
        int _short_fd, recv_retcode;
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        if (recv(p->long_fd, &recv_retcode, sizeof(int), 0) <= 0) {
            perror("recv");
            break;
        }
        printf("recv = %d\n", recv_retcode);
        if (send(p->long_fd, &recv_retcode, sizeof(int), 0) <= 0) {
            perror("send");
            break;
        }
        //短连接接收数据
        if ((_short_fd = accept(p->short_fd, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("short__Accept failed");
            close(_short_fd);
            break; 
        }
        //处理数据
        printf("正在处理数据\n");
        strcpy(path, create_dir(recv_retcode, client_ip));
        get_file(path, _short_fd);
        printf("数据处理完毕\n");
        close(_short_fd);
    }
    close(p->long_fd);
}





//从任务链表中获取任务, 并处理任务
void *get_task(void *arg) {
    Arg *p = arg;
    LinkNode *delete;
    struct sockaddr_in sock_addr;
    int _short_fd; 
    /*如果connect失败,则删除这个节点,如果成功,删除节点的同时处理数据*/
    while (1) {
        //output(p->l);
        
        while (p->l->next == NULL) {
            sleep(1);
        }
        int mut = pthread_mutex_lock(&mutex);
        if (mut != 0) {
            printf("lock error\n");
            fflush(stdout);
        }    
        
        if (p->l->next) {
            //将delete锁住
            delete = p->l->next;
            p->l->next = delete->next;
            //printf("delete = %s\n", delete->data);

            //长连接访问pi,问你还在线么。
            p->long_fd = create_connect_fd(C_PORT + 1, delete->data, p->l->sock_addr);
            
            if (p->long_fd < 0) { 
                erase(delete); 
                //close(p->long_fd);
            }
            //链接成功:输出当前正在连接的ip, 处理pi数据, 删除节点.
            else {
                //处理pi数据
                deal_task(p, delete->data);
                erase(delete);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    printf("线程结束\n");
}







//将ip放入链表中
void get_ip_to_link(Arg *p, struct sockaddr_in client_addr) {
        if (inet_ntop(AF_INET, (void *)&(client_addr.sin_addr), ip, 16) == NULL) {
            perror("get_ip error");
        }
      // printf("%s\n", ip);
        int mut = pthread_mutex_lock(&mutex);
        if (mut != 0) {
            printf("lock error\n");
            fflush(stdout);
        }
        insert(l, ip, client_addr);
        pthread_mutex_unlock(&mutex);
        //output(l);
}





//获取pi
void *get_pi(void *arg) {
    Arg *p = arg;
    //初始化获取所有pi
    //init(p);
    //如果心跳收到connect,并且链表中没有, 则加入到链表中.
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        if (accept(p->heart_fd, (struct sockaddr *)&client_addr, &len) < 0) {
            perror("heart__Accept failed");
            exit(1);
        }
        get_ip_to_link(p, client_addr);
        //close(p->heart_fd);
        sleep(2);
    }
}

void *warn_func(void *arg) {
    Arg *p = arg;
    char logpath[MAX_SIZE] = {0};
    strcpy(logpath, "./log/Warn.log");
    char client_ip[MAX_SIZE] = {0};
   
    while (1) {

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int temp_fd;   
        //如果没有pi来连master，一直在阻塞。
        if ((temp_fd = accept(p->alarm_fd, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("warn__Accept failed");
            close(temp_fd);
            sleep(3);
            continue;
        }  
        //若链接成功，则处理警报信息
        else {
            if (inet_ntop(AF_INET, (void *)&(client_addr.sin_addr), client_ip, 16) == NULL) {
                perror("parsing ip error");
            }
            strcpy(logpath, create_warn_dir(client_ip));
            get_file(logpath, temp_fd);
            close(temp_fd);
        }

    }
}


void main_fork(){
    pid_t pid;
    int i;
    pid = fork();
    if(pid == -1) {
        exit(-1);
    }
    else if(pid != 0){
        exit(EXIT_SUCCESS);
    } 
}


int main() {
    main_fork();
    l = init_link();
    arg = init_arg(l);
    pthread_t heart, alarm;

    //心跳套接字
    arg->heart_fd = create_accept_fd(M_PORT);
    //数据套接字
    arg->short_fd = create_accept_fd(M_PORT + 1);
    //警报套接字
    arg->alarm_fd = create_accept_fd(M_PORT + 2);
    if (pthread_create(&heart, NULL, get_pi, (void *)arg) < 0) {
        perror("create heart failed");
        exit(1);
    }
    if (pthread_create(&alarm, NULL, warn_func, (void *)arg) < 0) {
        perror("create alarm failed");
        exit(1);
    }
    pthread_t t[6];
    if(pthread_mutex_init(&mutex, NULL) != 0) {  
        printf("Init metux error.\n");  
        exit(1);  
    }
    for (int i = 0; i < 5; i++) {
        t[i] = i + 1;
        if (pthread_create(&t[i], NULL, get_task, (void *)arg) < 0) {
            perror("Create_failed!\n");
            return -1;
        }
    }
    pthread_join(heart, NULL);
    for (int i = 0; i < 5; i++) {
        pthread_join(t[i], NULL);
    }
    return 0;
}
