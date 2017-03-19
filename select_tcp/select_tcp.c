#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/time.h>
#include<unistd.h>
#include<assert.h>

#define _SIZE_ 1024

int gfds[_SIZE_];

int startup(int _port,char*_ip)
{
    assert(_ip);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
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
        printf("Usage:%s [local_ip][local_port]",argv[0]);
        return 4;
    }
    int listen_sock=startup(atoi(argv[2]),argv[1]);

    int i=0;
    for(;i<_SIZE_;i++)
    {
        gfds[i]=-1;    //set to invalid
    }
    gfds[0]=listen_sock;
    while(1)
    {
        int max_fd=-1;
        struct timeval timeout={5,0};
        fd_set rfds;
        FD_ZERO(&rfds);
        int j=0;
        for(;j<_SIZE_;j++)
        {
            if(gfds[j]>=0)
            {
                FD_SET(gfds[j],&rfds);
            }
            if(max_fd<gfds[j])
            {
               max_fd= gfds[j];
            }
        }

        switch(select(max_fd+1,&rfds,0,0,0))
        {
            case -1: //error
                    perror("select");
                    break;
            case 0: //timeout
                    printf("timeout...\n");
                    break;
            default: //ok
                    for(int k = 0;k<_SIZE_;k++)
                    {
                        if(gfds[k]<0)
                        continue;       

                        if(gfds[k]==listen_sock && FD_ISSET(gfds[k],&rfds))
                        {
                            //listen finish
                            struct sockaddr_in remote;
                            socklen_t len=sizeof(remote);

                            int sock=accept(listen_sock,(struct sockaddr*)&remote,&len);
                            if(sock<0)
                            {
                                perror("accept");
                                exit(5);
                            }

                            int n=0;
                            for(;n<_SIZE_;n++)
                            {
                                if(gfds[n]==-1)
                                {
                                    gfds[n]=sock;  //find min_fd,set sock
                                    printf("client ip:%s,port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
                                    break;
                                }
                            }
                            if(n==_SIZE_)
                            {
                                close(sock);
                            }
                        }
                        else if(FD_ISSET(gfds[k],&rfds))
                        {
                            char buf[1024];
                            while(1)
                            {
                                ssize_t _s=read(gfds[k],buf,sizeof(buf)-1);
                                if(_s>0)
                                {
                                    buf[_s-1]=0;
                                    printf("client#%s\n",buf);
                                }
                                else if(_s==0)
                                {
                                    close(gfds[k]);
                                    gfds[k]=-1;
                                    break; 
                                }
                                else
                                {
                                    perror("read");
                                    exit(6);
                                }
                            }  
                        } //
                    }

            break;
            
        }
    
    }
    close(listen_sock);
   return 0;
}

