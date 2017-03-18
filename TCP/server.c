/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: Tue 07 Mar 2017 05:45:33 PM PST
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int startup(int _port,const char* _ip)
{
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }

     int opt=1;
     if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0)
    {
        perror("setsockopt");
        exit(2);
    }


    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(_port);
    local.sin_addr.s_addr=inet_addr(_ip);

    socklen_t len=sizeof(local);

   if(bind(sock,(struct sockaddr*)&local,len)<0)
   {
     perror("bind");
     exit(2);
    }

    if(listen(sock,5)<0)
    {
        perror("listen");
        exit(3);
    }
   return sock;
}
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("Usage: [local_ip] [local_port]",argv[0]);
        return 3;
    }
    int listen_socket=startup(atoi(argv[2]),argv[1]);

    struct sockaddr_in remote;
    socklen_t len=sizeof(struct sockaddr_in);

    while(1)
    {
        int socket=accept(listen_socket,(struct sockaddr*)&remote,&len);
        if(socket<0)
        {
            perror("accept");
            continue;
        }

        pid_t pid=fork();
        if(pid==0)
        {
            if(fork()>0)
            {
                exit(1);
            }
            char buf[1024];
            while(1)
            {
               ssize_t _s=read(socket,buf,sizeof(buf)-1);
               if(_s>0)
               {
                 buf[_s]=0;
                 printf("client# %s\n",buf);   
               }
               else
               {
                 printf("client is quit!\n");
                 break;
               }
               printf("client,ip:%s,port:%d\n",inet_ntoa(remote.sin_addr)\
               ,ntohs(remote.sin_port)); 
            } 
        }
        else if(pid>0)
        {

           close(socket);
           waitpid(-1,NULL,WNOHANG);         
        }
        else
        {
            perror("fork");
            exit(2);
        }
    }    
    return 0;
}
