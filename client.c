#include "client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (input_check(argc, argv) == 0)
    {
        printf("input error.\n");
        return 0;
    }

    while (1)
    {
        char message[MSG_MAX_SIZE];

        if (fgets(message, MSG_MAX_SIZE, stdin) == NULL)
            continue;

        if (strlen(message) >= MSG_MAX_SIZE - 1)
        {
            printf("input too long\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        message[strcspn(message, "\n")] = 0;
        // user press only enter key or input is not allowed
        if(strlen(message) == 0 || precheck(message) == 0)
        {
            printf("The strings format is not correct.\n");
            continue;
        }


        pthread_t tid;
        int err = pthread_create(&tid, NULL, &query_handler, message);
        if (err != 0)
        {
            printf("Failed to create a thread.\n");
            continue;
        }
    }
    // close(sockfd);
    return 0;
}

void *query_handler(void *message)
{
    char receive_message[MSG_MAX_SIZE];
    // create a socket
    int sockfd  = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Fail to create a socket.");
    }
    // socket的連線
    struct sockaddr_in info;
    // bzero(&info, sizeof(info));
    memset(&info, 0, sizeof(info));
    info.sin_family = PF_INET;

    // localhost test
    info.sin_addr.s_addr = ip_addr;
    info.sin_port = htons(port);

    int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
    if (err == -1)
    {
        printf("Connection error\n");
        return 0;
    }
    send(sockfd, message, MSG_MAX_SIZE, 0);
    recv(sockfd, receive_message, MSG_MAX_SIZE, 0);
    printf("%s", receive_message);

    close(sockfd);
    pthread_exit(0);
    return NULL;
}

char precheck(char *message)
{
    if (strncmp(message, "Query ", 6) != 0)
        return 0;

    const char *index = &message[6];
    int query_list_size = 0;
    while (1)
    {
        char *start_str = strchr(index, '\"');
        if (start_str == NULL)
            break;
        char *end_str = strchr(start_str + 1, '\"');
        if (end_str == NULL)
            return 0;

        index = end_str + 1;
        query_list_size++;
    }
    if(query_list_size != 0)
    {
        return 1;
    }
    return 0;
}

char input_check(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("argument incompleted.\n");
        return 0;
    }

    if ((strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "-p") != 0) ||
            (strcmp(argv[3], "-h") != 0 && strcmp(argv[3], "-p") != 0))
    {
        printf("input error.\n");
        return 0;
    }

    ip_addr = inet_addr(strcmp(argv[1], "-h") == 0 ? argv[2] : argv[4]);
    port = atoi(strcmp(argv[1], "-p") == 0 ? argv[2] : argv[4]);

    return 1;
}
