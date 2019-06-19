#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "defines.h"

#define MAX_CLIENTS 10

static int sockfd;
int clients[MAX_CLIENTS] = {0};

void handler(int signal)
{
	printf("Ctrl-C signal has been caught: server was stopped\n");
	close(sockfd);
	exit(0);
}

void manageConnection(unsigned int fd)
{
    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    int incom = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrSize);
    if (incom < 0)
    {
        perror("Error in accept(): ");
        exit(-1);
    }

    printf("\nNew connection: \nfd = %d \nip = %s:%d\n", incom, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == 0)
        {
            clients[i] = incom;
            printf("Managed as client #%d\n", i);
            break;
        }
    }
}

void manageClient(unsigned int fd, unsigned int client_id)
{
    char msg[MSG_LEN];
    memset(msg, 0, MSG_LEN);

    struct sockaddr_in client_addr;
    int addrSize = sizeof(client_addr);

    int recvSize = recv(fd, msg, MSG_LEN, 0);
    if (recvSize == 0)
    {
// имя машины, подключившейся к сокету
        getpeername(fd, (struct sockaddr *)&client_addr,(socklen_t *)&addrSize);
        printf("User %d disconnected %s:%d \n", client_id, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        close(fd);
        clients[client_id] = 0;
    }
    else
    {
        msg[recvSize] = '\0';
        printf("Message from client %d: '%s'\n", client_id, msg);
    }
}

int main(void)
{

    if(signal(SIGINT, handler) != 0)
    {
	printf("Can't set signal handler\n");
	return -1;
    } 

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Can't create socket: ");
        return sockfd;
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SOCK_PORT);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Can't bind socket to address:");
        return -1;
    }
    printf("Server is listening on the %d port\n", SOCK_PORT);

    if (listen(sockfd, 4) < 0)
    {
        perror("Error in listen(): ");
        return -1;
    }
    printf("Waiting for the connections\n");

    while(1)
    {
        fd_set readfds; 
        int max_fd;     
        int active_clients_count; 

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_fd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];

            if (fd > 0)
            {
                FD_SET(fd, &readfds);
            }

            max_fd = (fd > max_fd) ? (fd) : (max_fd);
        }

        active_clients_count = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (active_clients_count < 0 && (errno != EINTR))
        {
            perror("Error in select():");
            return active_clients_count;
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            manageConnection(sockfd);
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];
            if ((fd > 0) && FD_ISSET(fd, &readfds))
            {
                manageClient(fd, i);
            }
        }
    }
    return 0;
}
