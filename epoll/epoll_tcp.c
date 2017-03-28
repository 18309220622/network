#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<assert.h>

#define _SIZE_ 1024

typedef struct epbuf{
    int fd;
    char buf[_SIZE_];
}epbuf_t,*epbuf_p,**epbuf_pp;

static int startup(const char* _ip,int _port)
{
    assert(_ip);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(2);
    }

    int opt=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(_port);
    local.sin_addr.s_addr=inet_addr(_ip);
    socklen_t len=sizeof(local);

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

static epbuf_p alloc_epbuf(int fd)
{
    epbuf_p ptr=(epbuf_p)malloc(sizeof(epbuf_t));
    if(ptr==NULL)
    {
        perror("malloc");
        exit(7);
    }
    ptr->fd=fd;
    return ptr;         
}

static epbuf_p del_epbuf(epbuf_p ptr)
{
    if(ptr!=NULL)
    {
        free(ptr);
        ptr=NULL;
    }
    return NULL;
}
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("Usage:%s [local_ip] [local_port]\n",argv[0]);
        exit(1);
    }

    int listen_sock= startup(argv[1],atoi(argv[2]));
    int epfd=epoll_create(256);
    if(epfd<0)
    {
        perror("epoll_create");
        exit(5);
    }
    
    struct epoll_event _ev;
    _ev.events=EPOLLIN | EPOLLET;
    _ev.data.ptr=alloc_epbuf(listen_sock);
    
    if((epoll_ctl(epfd,EPOLL_CTL_ADD,listen_sock,&_ev))<0)
    {
        perror("epoll_ctl");
        exit(6);      
    }
    //epoll_wait

    struct epoll_event events[32];
    int max_evs=32;
    int timeout=3000;
     
    while(1)
    {
        int nums=0;
        int i=0;
        switch(nums=epoll_wait(epfd,events,max_evs,timeout))
        {
            case -1:   //failed
                    perror("epoll_wait");
                    break;

            case 0:    //timeout
                    printf("timeout\n");
                    break;

            default:   // success
                    for(;i<nums;i++)
                    {
                       int fd=((epbuf_p)(events[i].data.ptr))->fd;
                       if(fd==listen_sock && (EPOLLIN & events[i].events))
                        {
                            struct sockaddr_in peer;
                            socklen_t len=sizeof(peer);
                            int acc_sock=accept(fd,(struct sockaddr*)&peer,&len);
                            if(acc_sock<0)
                            {
                                perror("accept");
                                continue;
                            }
                            _ev.events=EPOLLIN|EPOLLET;
                            _ev.data.ptr=alloc_epbuf(acc_sock);
                            epoll_ctl(epfd,EPOLL_CTL_ADD,acc_sock,&_ev);
                        }//if
                        else if(fd!=listen_sock &&(events[i].events & EPOLLIN))
                        {                       
                            char* buf=((epbuf_p)(events[i].data.ptr))->buf;
                            ssize_t _s=read(fd,buf,_SIZE_);
                            if(_s>0)
                            {
                                buf[_s]=0;
                                printf("%s\n",buf);
                                _ev.events=EPOLLOUT;
                                _ev.data.ptr=(epbuf_p)events[i].data.ptr;
                                epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&_ev);
                                   
                            }
                            else if(_s==0)
                            {
                                del_epbuf((epbuf_p)(events[i].data.ptr));
                                events[i].data.ptr=NULL;
                                epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
                                close(fd);
                                printf("client quit!");
                            }
                            else
                            {
                                perror("read");
                            }
                        }  //else if
                        else if(fd!=listen_sock && (EPOLLOUT & events[i].events ))
                        {
                            char* buf=((epbuf_p)(events[i].data.ptr))->buf;
                            const char* msg="HTTP/ 1.0200 OK\r\n\r\n<html><h1>HELLO WORLD!\
                            </h1></html>\n";
                            write(fd,msg,strlen(msg));
                        }  //else if
                        else
                        {
                            //do nothing!
                        }
                    } //for
                    break;
        }
    }
    return  0;
}
