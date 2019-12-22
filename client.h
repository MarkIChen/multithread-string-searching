#ifndef CLIENT_H
#define CLIENT_H

#define MSG_MAX_SIZE 2048

unsigned long ip_addr;
unsigned short port;


void *query_handler(void *);

int sockfd;

char input_check(int, char **);
char link_to_server();

char precheck(char *);

#endif
