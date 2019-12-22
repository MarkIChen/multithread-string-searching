#ifndef SERVER_H
#define SERVER_H

#define MSG_MAX_SIZE 2048
#define FILE_TABLE_SIZE 500
#define RESULT_SIZE 4000
#define _GNU_SOURCE
#include <pthread.h>

struct request
{
    char **query_list;
    int list_size;
    int forClientSockfd;
    struct request *next;
};

struct request *req_que_head;
struct request *req_que_tail;

int count = 0;

char *root;
unsigned short server_port;
unsigned int t_pool_number;

int sockfd = 0;
struct sockaddr_in serverInfo, clientInfo;
pthread_mutex_t req_que_look;
pthread_mutex_t file_table_look;
unsigned int addrlen;
char input_check(int, char **);
char start_server();

void *worker_handler(void *para);
void *main_handler(void *para);

struct request *getRequest(const char *, int);
char insert_request(struct request *);

// file Searching
int file_table_size = 0;
char *file_table[FILE_TABLE_SIZE];

char get_file_table(const char *, char **, int *);
int search_file(const char *dir, const char *);
char *get_search_result(char **, int, char **, int);

#endif
