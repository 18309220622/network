#include<stdio.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<stdlib.h>

int startup(int _port,char* _ip)
{
    assert(_ip);

    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(2);
    }

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(_port);
    local.sin_addr.s_addr=inet_addr(_ip);
    socklen_t len=sizeof(local);
      
    int opt = 1;
    int stat= setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    if(bind(sock,(struct sockaddr*)&local,len)<0)
    {
        perror("bind");
        exit(3);
    }
    if(listen(sock,5)<0)
    {
        perror("listen");
        exit(4);
    }
    return sock;
}

void* handler_fd(void* arg)
{
    int sock=*((int*)arg);
    pthread_detach(pthread_self());
    char buf[1024];
    while(1)
    {
        ssize_t _s=read(sock,buf,sizeof(buf)-1);
        if(_s>0)
        {
          buf[_s-1]=0;
          printf("Client# %s\n",buf);
        }
        else 
        {
           printf("Client quit!");
           close(sock);
           break;
        }  
}

int main(int argc,char*argv[])
{
    if(argc!=3)
    {
        printf("Usage:%s,[local_ip] [local_port]\n",argv[0]);
        exit(1);
    }
    int listen_sock=startup(atoi(argv[2]),argv[1]);

    struct sockaddr_in peer;
    socklen_t len=sizeof(peer);

    while(1)
    {
        int sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
        if(sock<0)
        {
            perror("accept");
            continue;
        }

        pthread_t tid;
        int ret=pthread_create(&tid,NULL,handler_fd,&sock);
        if(ret<0)
        {
            printf("pthread create failed!\n");
            exit(5);
        }
        pthread_detach(tid);
    }
    return 0;
}
