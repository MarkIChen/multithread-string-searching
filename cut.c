#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MSG_MAX_SIZE 2048

struct request {
  char **query_list;
  int forClientSockfd;
  int list_size;
  struct request *next;
};

struct request getRequest(char *);

int main() {

  // char query = "Query "test"";

  while (1) {
    char message[MSG_MAX_SIZE];
    char receive_message[MSG_MAX_SIZE];
    if (fgets(message, MSG_MAX_SIZE, stdin) == NULL)
      continue;

    if (strlen(message) >= MSG_MAX_SIZE - 1) {
      printf("input too long\n");
      int c;
      while ((c = getchar()) != '\n' && c != EOF) {
      }
      continue;
    }
    message[strcspn(message, "\n")] = 0;

    struct request new_query = getRequest(message);

    for (int i=0;i<new_query.list_size;i++){
      printf("query: %s\n", new_query.query_list[i]);
    }

    // printf("%s", message);
  }

  return 0;
}

struct request getRequest(char *message) {
  struct request new_request;
  if (strncmp(message, "Query ", 6) != 0) {
    new_request.list_size = -1;
    return new_request;
  }
  char *index = &message[6];
  int query_list_size = 0;
  char *buffer[256];
  while (1) {
    char *start_str = strchr(index, '\"');
    if (start_str == NULL)
      break;
    char *end_str = strchr(start_str + 1, '\"');
    if (end_str == NULL)
      break;

    int size = end_str - start_str - 1;
    buffer[query_list_size] = malloc(sizeof(char) * (size + 1));
    strncpy(buffer[query_list_size], start_str + 1, size);
    buffer[query_list_size][size] = '\0';
    printf("new_string :%s\n", buffer[query_list_size]);
    index = end_str + 1;
    query_list_size++;
  }

  char **new_list = malloc(sizeof(char *) * query_list_size);
  for (int i = 0; i < query_list_size; i++)
    new_list[i] = buffer[i];

  new_request.query_list = new_list;
  new_request.next = NULL;
  new_request.list_size = query_list_size;
  return new_request;
}
