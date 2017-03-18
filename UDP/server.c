#include<stdio.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>

int main(int argc,const char* argv[])
{   
    if(argc!=3)
    {
        printf("Usage:[local_ip] [local_port]",argv[0]);
        exit(3);
    }
    int sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(atoi(argv[2]));
    local.sin_addr.s_addr=inet_addr(argv[1]);

    socklen_t len=sizeof(local);

    if(bind(sock,(struct sockaddr*)&local,len)<0)
    {
        perror("bind");
        exit(2);
    }

    char buf[1024];
    while(1)
    {
        struct sockaddr_in peer;
        socklen_t len=sizeof(peer);
        memset(&peer,0,len);

        ssize_t size=recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr*)&peer,&len);
        if(size>0)
        {
            printf("get a msg pack:%s,%d\n",inet_ntoa(peer.sin_addr)\
                ,ntohs(peer.sin_port));
           
            buf[size]='\0';
            printf("client say# %s\n",buf);
        }
        else if(size==0)
        {
            printf("send quit"); 
            break;
        }
        else
        {
            perror("recvfrom");
            exit(3);
        }
    }
    close(sock);
    return 0;
}
