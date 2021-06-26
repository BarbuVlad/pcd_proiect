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

//  int pipe_fd_parent[2];// 0 - read          1 - write
// // int pipe_fd_child[2];

int users_count = 0;
int conn_users_count;

int admins_count = 0;
BOOL admin_is_conn = FALSE;///< admin conn must be unique

pthread_mutex_t mutex;///<admin conn...
int sockfd;

char locatii[256][256];
int nr_locatii = 0;

#endif