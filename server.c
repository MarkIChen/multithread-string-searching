#include "server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char *argv[])
{

    if (input_check(argc, argv) == 0)
        return 0;

    if (start_server() == 0)
        return 0;

    if (pthread_mutex_init(&req_que_look, NULL) != 0)
    {
        printf("req_que_look init failed.\n");
        return 0;
    }

    if (pthread_mutex_init(&file_table_look, NULL) != 0)
    {
        printf("file_table_look init failed.\n");
        return 0;
    }

    // load directory
    file_table_size = 0;
    if (get_file_table(root, file_table, &file_table_size) == 0)
    {
        printf("file open error.\n");
        return 0;
    }

    pthread_t tid[t_pool_number];
    req_que_head = NULL;
    req_que_tail = NULL;

    // create muti threads
    for (int i = 0; i < t_pool_number; i++)
    {
        // count ++;
        int err = pthread_create(&(tid[i]), NULL, &worker_handler, NULL);
        if (err != 0)
            printf("Failed to create a thread.\n");
    }

    pthread_t main_tid;
    // create main thread
    int err = pthread_create(&main_tid, NULL, &main_handler,
                             NULL);
    if (err != 0)
    {
        printf("Failed to create a thread.\n");
        return 0;
    }

    pthread_join(main_tid, NULL);
    for (int i = 0; i < t_pool_number; i++)
    {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&req_que_look);
    pthread_mutex_destroy(&file_table_look);
    return 0;
}

void *main_handler(void *para)
{
    // int forClientSockfd = *(int *)para;
    while (1)
    {
        int forClientSockfd = accept(sockfd, (struct sockaddr *)&clientInfo, &addrlen);
        if (forClientSockfd == -1)
            break;

        char inputBuffer[MSG_MAX_SIZE];
        int err = recv(forClientSockfd, inputBuffer, sizeof(inputBuffer), 0);
        if (err < 1)
            break;

        printf("%s\n", inputBuffer);

        struct request *new_query = getRequest(inputBuffer, forClientSockfd);
        if (new_query == NULL)
        {
            printf("input error. try again.\n");
            continue;
        }
        pthread_mutex_lock(&req_que_look);
        insert_request(new_query);
        pthread_mutex_unlock(&req_que_look);

    }

    pthread_exit(0);
    return NULL;
}

void *worker_handler(void *para)
{
    while (1)
    {
        struct request *work = NULL;
        pthread_mutex_lock(&req_que_look);
        if (req_que_head != NULL)
        {
            // get a work
            work = req_que_head;
            // dequeue
            if (req_que_head->next != NULL)
            {
                req_que_head = req_que_head->next;
            }
            else if (req_que_head->next == NULL)
            {
                req_que_head = NULL;
                req_que_tail = NULL;
            }
        }
        pthread_mutex_unlock(&req_que_look);
        if (work == NULL)
            continue;
        // update the file table
        pthread_mutex_lock(&file_table_look);
        file_table_size =0 ;
        get_file_table(root, file_table, &file_table_size);
        // search File
        char *result = get_search_result(file_table, file_table_size, work->query_list, work->list_size);
        pthread_mutex_unlock(&file_table_look);

        send(work->forClientSockfd, result, strlen(result), 0);
        free(result);
    }
    pthread_exit(0);
    return NULL;
}

char input_check(int argc, char **argv)
{
    if (argc != 7)
    {
        printf("argument incompleted.\n");
        return 0;
    }

    for (int i = 1; i < 7; i += 2)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            root = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            server_port = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            t_pool_number = atoi(argv[i + 1]);
        }
        else
        {
            printf("input error.\n");
            return 0;
        }
    }
    return 1;
}

char start_server()
{
    // socket的建立
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("Fail to create a socket.");
        return 0;
    }

    // socket的連線
    addrlen = sizeof(clientInfo);
    // bzero(&serverInfo, sizeof(serverInfo));
    memset(&serverInfo, 0, sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(server_port);
    bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    listen(sockfd, 100);
    return 1;
}

struct request *getRequest(const char *message, int forClientSockfd)
{
    if (strncmp(message, "Query ", 6) != 0)
        return NULL;

    const char *index = &message[6];
    int query_list_size = 0;
    char *buffer[256];
    while (1)
    {
        char *start_str = strchr(index, '\"');
        if (start_str == NULL)
            break;
        char *end_str = strchr(start_str + 1, '\"');
        if (end_str == NULL)
            break;

        int size = end_str - start_str - 1;
        index = end_str + 1;
        if(size > 128)
        {
            printf("A string in query is too large, so it is skipped.\n");
            continue;
        }
        buffer[query_list_size] = malloc(sizeof(char) * (size + 1));
        strncpy(buffer[query_list_size], start_str + 1, size);
        buffer[query_list_size][size] = '\0';


        query_list_size++;
        if(query_list_size == 150)
        {
            printf("Too much string in a query.\n");
            break;
        }
    }

    char **new_list = malloc(sizeof(char *) * query_list_size);
    for (int i = 0; i < query_list_size; i++)
        new_list[i] = buffer[i];

    struct request *new_request = malloc(sizeof(struct request));
    new_request->query_list = new_list;
    new_request->list_size = query_list_size;
    new_request->next = NULL;
    new_request->forClientSockfd = forClientSockfd;
    return new_request;
}

char insert_request(struct request *new_request)
{
    if (req_que_head == NULL)
    {
        req_que_head = new_request;
        req_que_tail = new_request;
    }
    else
    {
        req_que_tail->next = new_request;
        req_que_tail = new_request;
    }
    return 1;
}

char *get_search_result(char **file_table, int file_table_size,
                        char **token, int token_size)
{

    char result[RESULT_SIZE] = "";
    char str_convert[12];

    for (int j = 0; j < token_size; j++)
    {
        strcat(result, "String: \"");
        strcat(result, token[j]);
        strcat(result, "\"\n");
        int total_count = 0;

        for (int i = 0; i < file_table_size; i++)
        {
            int count_file = 0;
            if ((count_file = search_file(file_table[i], token[j])) == -1)
            {
                printf("error");
                break;
            }
            total_count += count_file;
            if (count_file > 0)
            {
                sprintf(str_convert, "%d", count_file);

                strcat(result, "File: ");
                strcat(result, file_table[i]);
                strcat(result, ", Count: ");
                strcat(result, str_convert);
                strcat(result, "\n");
            }
        }
        if (total_count == 0)
            strcat(result, "Not Found\n");
    }

    char *msg = malloc(sizeof(char) * (strlen(result)) + 1);
    strncpy(msg, result, strlen(result) + 1);
    return msg;
}

char get_file_table(const char *route, char **file_table,
                    int *file_table_size)
{
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(route)) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            char *pch;
            if ((pch = strchr(ent->d_name, '.')) == NULL)
            {
                char child_route[strlen(route) + 100];
                strcpy(child_route, route);
                strcat(child_route, "/");
                strcat(child_route, ent->d_name);
                get_file_table(child_route, file_table, file_table_size);
            }
            else
            {
                char *whole_route =
                    malloc(sizeof(char) * (strlen(route) + strlen(ent->d_name)) + 2);
                strcpy(whole_route, route);
                strcat(whole_route, "/");
                strcat(whole_route, ent->d_name);
                file_table[*file_table_size] = whole_route;
                *file_table_size += 1;
                if (*file_table_size >= FILE_TABLE_SIZE)
                {
                    printf("Too much directory, the max directory number is %d. Some "
                           "txt file will be skipped.\n",
                           FILE_TABLE_SIZE);
                    break;
                }
            }
        }
        closedir(dir);
    }
    else
    {
        printf("Error while opening file. Please check the directory.\n");
        return 0;
    }
    return 1;
}

int search_file(const char *dir, const char *token)
{
    // file open
    FILE *file = fopen(dir, "r");
    char *line_buf = NULL;
    size_t line_buf_size = 0;

    if (!file)
    {
        printf("Error open while opening the file.\n");
        return -1;
    }

    int count = 0;
    while (getline(&line_buf, &line_buf_size, file) >= 0)
    {
        char *pch = line_buf;
        while ((pch = strstr(pch, token)) != NULL)
        {
            count += 1;
            pch += 1;
        }
    }
    fclose(file);
    return count;
}
