#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#define    MAXLINE        1024

int main() {
    int umap[9999]={0};

    FILE    *fp;
    int     recv_len;
    int     write_leng;
    char    buf[MAXLINE];
//自己先手动创建文件
    if ((fp = fopen("/home/h264.ps", "w")) == NULL) {
        perror("Open file failed\n");
        exit(0);
    }

    // 创建socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr;
    saddr.sin_port = htons(9999);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定
    bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    // 监听
    listen(lfd, 10);

    // 创建epoll
    int epfd = epoll_create(100);

    // epoll注册时间
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &epev);

    struct epoll_event epevs[1024];

    while(1) {

        int ret = epoll_wait(epfd, epevs, 1024, -1);
        if(ret == -1) {
            perror("epoll_wait");
            exit(-1);
        }

        //printf("ret = %d\n", ret);
        

        for(int i = 0; i < ret; i++) {

            int curfd = epevs[i].data.fd;
            //是监听
            if(curfd == lfd) {
                struct sockaddr_in cliaddr;
                unsigned int len = sizeof(cliaddr);
                int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);
                int curport=((struct sockaddr_in)cliaddr).sin_port;
                printf("port %d client connect...\n",curport);
                umap[cfd]=curport;
                epev.events = EPOLLIN;
                epev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &epev);
            } else {//是数据
                 if(epevs[i].events & EPOLLOUT) {
                    continue;
                }  
                if(fp==NULL){
                    if ((fp = fopen("/home/host.config", "a+")) == NULL) {
                        perror("Open file failed\n");
                        exit(0);
                    }
                }
                    
                char buf[1024] = {0};
                memset(buf,'\0', MAXLINE);
                while(recv_len = read(curfd, buf, sizeof(buf))){
                    printf("recv_len=%d",recv_len);
                    if(recv_len == -1) {
                        perror("read");
                        exit(-1);
                    } else if(recv_len == 0) {
                        printf("client closed...\n");
                        epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                        close(curfd);
                    } else if(recv_len > 0) {
                        //printf("port %d read buf = %s\n",umap[curfd], buf);
                        write_leng = fwrite(buf, sizeof(char), recv_len, fp);
                        printf("write_leng=%d\n",write_leng);
                    }
                    if (write_leng < recv_len) {
                        printf("Write file failed\n");
                        break;
                    }
                    memset(buf,'\0', MAXLINE);
                }

                fclose(fp);
                fp=NULL;

            }
        }
    }
    close(lfd);
    close(epfd);
    return 0;
}