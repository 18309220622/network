#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>

int main()
{
    int fd[2];
    int ret=socketpair(PF_LOCAL,SOCK_STREAM,0,fd);
    if(ret!=0)
    {
        perror("socketpair");
        return 1;
    }

    pid_t id=fork();
    if(id<0)
    {
        perror("fork");
        return id;
    }
    else if(id==0)    //child
    {
        const char* msg="hi,Peter";
        char buf[1024];
        close(fd[0]);     //close stdin
        while(1)
        {
            memset(buf,0,sizeof(buf));
            strcpy(buf,msg);
            write(fd[1],buf,sizeof(buf));
            ssize_t  _s=read(fd[1],buf,sizeof(buf));
            if(_s<0)
            {
                perror("read");
                return 2;
            }
            else
            {
                buf[_s]='\0';
                printf("client say:%s\n",buf);
            }
            sleep(2);
        }
        close(fd[1]);
    }
    else
    {
        //father
        const char* msg="hi,Jack!";
        close(fd[1]);   
        char buf[1024];
        while(1)
        {
            memset(buf,0,sizeof(buf));
            ssize_t _s=read(fd[0],buf,sizeof(buf));
            if(_s<0)
            {
                perror("read");
                return 3;
            }
            else
            {
                buf[_s]='\0';
                printf("father echo:%s\n",buf);
            }
            strcpy(buf,msg);
            write(fd[0],buf,sizeof(buf));
            sleep(3);
        }
        close(fd[0]);
    }
    int status;
    waitpid(id,&status,WNOHANG);
    return 0;
}
