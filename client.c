#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 9528

int main(int argc, char *argv[])
{
    if(2 != argc)
    {
        printf("please input the server's ip\n");
        return -1;
    }

    printf("Client start working\n");

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        printf("client create socket failed!\n");
        return -1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(-1 == ret)
    {
        printf("connect server failed!\n");
        close(socket_fd);
        return -1;
    }

    printf("Connect server success!\n");

    char wr_str[100] = "Sir!";
    write(socket_fd, wr_str, 4);

    char recv_buf[1024];
    int always_true = 1;
    while(always_true)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        read(socket_fd, recv_buf, sizeof(recv_buf));
        if(strlen(recv_buf) > 0)
        {
            printf("Recv server:%s\n", recv_buf);
        }
        else
        {
            break;
        }
    }

    close(socket_fd);
    return 0;
}

