#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "defines.h"

int main(int argc, char** argv)
{
    char* server_id;
    if (argc != 2)
    {
        srand(time(NULL));
        int temp = rand();
        sprintf(server_id,"Client %d", temp);
    }
    else
    {
        server_id = argv[1];
    }


    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0); 
    if (sockfd < 0)
    {
        printf("Error in socket():\n");
        return sockfd;
    }

    struct sockaddr server_addr;
    server_addr.sa_family = AF_UNIX;
    strcpy(server_addr.sa_data, SOCKET_NAME);

    char msg[MSG_LEN];
    
    memset(msg, 0, MSG_LEN);
    sprintf(msg, "%s sent message",server_id);
    if (sendto(sockfd, msg, strlen(msg), 0, &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error in send():");
        return -1;
    }

    printf("Sended message: '%s'\n", msg);

    int wait_time = 1 + rand() % 3;
    sleep(wait_time);
    
    printf("Client is disconnected\n");
    close(sockfd);
    return 0;
}
