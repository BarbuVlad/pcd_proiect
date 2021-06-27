#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERV_TCP_PORT 5000
#define MAXLINE 512
//#define MAXHOSTNAME 100
#define MAXNOUSERS 100
#define MAXNOADMINS 10

#define BOOL int
#define TRUE 1
#define FALSE 0

#define SOCK_PATH_UNIX  "/tmp/unix_sock.server"

typedef struct connections{
	pid_t pid_child;
	char username[50];
    //int pipe_fd[2];
} connections;

typedef struct client{
    char username[50];
    char password[50];
} client;


client users[MAXNOUSERS];///< store all existing users
connections connected_users[MAXNOUSERS];///< store all connected users

client admins[MAXNOADMINS];///< store all existing admins

int users_count = 0;
int conn_users_count = 0;

int admins_count = 0;
BOOL admin_is_conn = FALSE;///< admin conn must be unique

pthread_mutex_t mutex;///<admin conn...
int sockfd;

pid_t parent;
#endif