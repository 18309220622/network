#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>

int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("Usage:[ip] [port]",argv[0]);
        exit(1);
    }
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(2);
    }

    struct sockaddr_in server;
    server.sin_family=AF_INET;
    server.sin_port=htons(atoi(argv[2]));
    server.sin_addr.s_addr=inet_addr(argv[1]);
    socklen_t len=sizeof(server);

    if(connect(sock,(struct sockaddr*)&server,len)<0)
    {
        perror("connect");
        exit(3);
    }
    
    char buf[1024];
    while(1)
    {
        printf("client#");
        fflush(stdout);
        ssize_t _s=read(0,buf,sizeof(buf)-1);
        if(_s>0)
        {
            buf[_s-1]=0;
            write(sock,buf,_s);
        }
        else
        {
            exit(4);
        }
    }
    close(sock);
    return 0;
}
