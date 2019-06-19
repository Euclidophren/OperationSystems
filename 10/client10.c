#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "defines.h"

int main(void)
{
    srand(time(NULL));
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Error in sock(): ");
        return sock;
    }
/*
struct hostent {
        char    *h_name;        // официальное имя машины /
        char    **h_aliases;    // список псевдонимов /
        int     h_addrtype;     // тип адреса машины /
        int     h_length;       // длина адреса /
        char    **h_addr_list;  // список адресов /
}
*/
    struct hostent *host = gethostbyname(SOCK_ADDR); 
    if (!host)
    {
        perror("Error in gethostbyname(): ");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error in connect():");
        return -1;
    }

    char msg[MSG_LEN]; 
    memset(msg, 0, MSG_LEN);
    sprintf(msg, "Receive a message\n");

    if (send(sock, msg, strlen(msg), 0) < 0)
    { 
        perror("Error in send(): ");
        return -1;
    }

    printf("Sended message: '%s'\n", msg);

    printf("Client is disconnected\n");
    return 0;
}
