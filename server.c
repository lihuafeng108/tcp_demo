//服务器最多实现10路client的连接和收发
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 9528
#define CLIENT_MAX  10

struct client_fd_sets
{
    int index;
    int fd_set[CLIENT_MAX];
};

static int get_max_fd(int server_fd, struct client_fd_sets client_fds)
{
    int max = server_fd;

    for(int i=0; i<client_fds.index; i++)
    {
        if(max < client_fds.fd_set[i])
            max = client_fds.fd_set[i];
    }

    return max;
}

int main(int argc, char *argv[])
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == socket_fd)
    {
        printf("server create socket failed!\n");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(-1 == ret)
    {
        printf("server bind failed!\n");
        close(socket_fd);
        return -1;
    }

    ret = listen(socket_fd, CLIENT_MAX);
    if(-1 == ret)
    {
        printf("server listen failied!\n");
        close(socket_fd);
        return -1;
    }

    printf("Server start working! socket_fd:%d\n", socket_fd);

    struct sockaddr_in client_addr = {0};
    int len = sizeof(client_addr);
    int new_client_sockfd = -1;
    struct client_fd_sets client_fds;
    memset(&client_fds, 0, sizeof(client_fds));

    fd_set fds;
    struct timeval time;
    time.tv_sec = 3;
    time.tv_usec = 0;
    int always_true = 1;


    while(always_true)
    {
        FD_ZERO(&fds);
        FD_SET(0, &fds);  //将输入描述符也添加进来
        FD_SET(socket_fd, &fds);

        for(int i=0; i<client_fds.index; i++)
        {
            if(client_fds.fd_set[i] > 0)
            {
                FD_SET(client_fds.fd_set[i], &fds);
            }
        }

        int max_fd = get_max_fd(socket_fd, client_fds);
        int ret = select(max_fd+1, &fds, NULL, NULL, &time);
        if(ret < 0)
        {
            printf("<lihf> 多路复用检测失败，程序退出\n");
            close(socket_fd);
        }
        else if(0 == ret)
        {   //timeout doing nothing
        }
        else
        {
            if(FD_ISSET(socket_fd, &fds))
            {   //客户端有新的连接
                new_client_sockfd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
                if(-1 == new_client_sockfd)
                {
                    printf("client accept failed!\n");
                    continue;
                }
                else
                {
                    if(client_fds.index > CLIENT_MAX)
                    {
                        client_fds.index = CLIENT_MAX;
                        printf("client accept fds more than %d\n", CLIENT_MAX);
                        continue;
                    }

                    client_fds.fd_set[client_fds.index++] = new_client_sockfd;
                    //printf("a new connection from [%s:%d]", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); //不知道为什么有个中括号在这里打印不出来
                    printf("a new connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                }
            }

            if(FD_ISSET(0, &fds))
            {
                char input_buf[1024] = {0};
                scanf("%s", input_buf);

                for(int i=0; i<client_fds.index; i++)
                {
                    write(client_fds.fd_set[i], input_buf, strlen(input_buf));
                }
            }

            //判断是否有客户端数据来到
            for(int i=0; i<client_fds.index; i++)
            {
                if(FD_ISSET(client_fds.fd_set[i], &fds))
                {
                    char recv_buf[1024]={0};
                    char send_str[512] = "OK!";
                    read(client_fds.fd_set[i], recv_buf, sizeof(recv_buf));

                    if(strlen(recv_buf) > 0)
                    {
                        printf("client:%s\n", recv_buf);  //这一行少了个换行符也打不出来，让调试有点莫名其妙....
                        write(client_fds.fd_set[i], send_str, 3);
                    }
                }
            }
        }
    }

    close(socket_fd);
    for(int i=0; i<client_fds.index; i++)
    {
        close(client_fds.fd_set[i]);
    }
    return 0;
}

