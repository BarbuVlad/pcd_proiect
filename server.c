#include "server.h"

void readClients(char* location, client clients[], int* client_count){
    int client_count_local = *client_count;
    FILE *fp = fopen(location, "r");
    if(fp == NULL) {
        perror("ERROR: cannot open file to read clients.");
        exit(-1);
    }
    char client[101];
    while(fgets(client, sizeof(client), fp) != NULL) {
        strcpy(clients[client_count_local].username, strtok(client, " "));
        strcpy(users[client_count_local].password, strtok(NULL, "\n"));
        client_count_local++;
    }
    *client_count = client_count_local;
    fclose(fp);
}

void* adminThreadRoutine(void* args){
    /* Function called at creating thread to serve admin connection
        Values to be sent for connection status:
            1 - successfull connection 
        (2 - bad password username  combination; done in main)
        (3 - admin connected; done in main thread)

        Requests to be received:
            (101 - login; solved for in main)
            102  - create new category 
            103 - 


    */
    printf("[Server PID: %d] Admin thread started...\n", getpid());

    //stop other admins from connecting
    pthread_mutex_lock(&mutex);
    admin_is_conn = TRUE;
    pthread_mutex_unlock(&mutex);

    int* _adminsockfd = (int*)args;
    //send successfull connection and setup establish
    send(*_adminsockfd, "1", 1, 0);
    printf("  DEBUG MSG: good admin credentials. In thread created. Sent code 1\n");

    int _rc;
    char _line[MAXLINE];
    char* req_type;
    while(_rc = recv(*_adminsockfd, &_line, MAXLINE, 0)){
    _line[_rc] = '\0';
    }


    //Stop admin
    pthread_mutex_lock(&mutex);
    admin_is_conn = FALSE;
    pthread_mutex_unlock(&mutex);
    pthread_exit(_adminsockfd);


}

void main(int argc, char const *argv[]){
    
    readClients("users.txt", users, &users_count);///< bring in local memory the users
    readClients("admins.txt", admins, &admins_count);///< bring in local memory the users

    //Main local variabiles 
    //(to be inherited by children processes and shared with threads)
    struct sockaddr_in cli_addr, serv_addr;///< addresses for client and server

    int client_len;
    int accepted_sockfd;///< to catch new client conn
    int rc;///< receive count; to read data from client
    char line[MAXLINE];///< line to read through socket

    int admin_sockfd;///< to treat new admin conn (such that orignal fd is not lost)
    pthread_t id[2];
    char username[50], password[50];

    char* ptr;
    BOOL good_credentials_admin = FALSE;

    //init mutex (admin connection bool)
    pthread_mutex_init(&mutex, NULL);

    //create socket as internet socket, TCP; exit 1 for error
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {perror("ERROR: at create socket"); exit(1);}
    // add reuse address
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        perror("setsockopt(SO_REUSEADDR) failed");}


/*
AF_INET - clienti la distanta 

AF_UNIX STREAM -  admin masina locala 

AF_UNIX DATAGRAM - pe post de pieps (IPC)

request5_parola[ strcspn(request5_parola, "\n") ] = 0;       //eliminare \n adaugat de fgets

*/
    //intialize address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;///< TCP IP
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    //bind socket to address; exit 2 for error
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {perror("EROARE la apel bind()"); exit(2);}
    printf("[Server PID: %d] successfull bind...\n", getpid());

    //listen for connections; exit 3 for error
    if(listen(sockfd, MAXNOUSERS) < 0) {perror("ERROR: at listen on socket"); exit(3);}

    //Start infinite loop to take connection
    for(;;){
        bzero((char *)&cli_addr, sizeof(cli_addr));///< zero-out client address
        client_len = sizeof(cli_addr);

        //accept incomming conn.
        accepted_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client_len);
        if (accepted_sockfd < 0){perror("ERROR: failed accept call...");}

        //read first message (must be login request)
        rc = recv(accepted_sockfd, &line, MAXLINE, 0);
        line[rc] = '\0';///< include end of string

        printf("[Server PID: %d] Login request, payload : %s\n",  getpid(), line);

        if(strncmp(line, "101", 3) == 0){///< Admin login request
            if(admin_is_conn == TRUE){
                send(accepted_sockfd, "3", 1, 0);
                close(accepted_sockfd);
                continue;///< go to new accept
            }
            //else
            //test credentials
            sscanf (line,"%*s %s %s",username,password);
            // ptr = sscanf(line, " ");
            // strcpy(username, ptr);
            // ptr = strtok(NULL, "\0");
            // strcpy(password, ptr);
            printf("DEBUG MSG: crdentials extracted: username_admin:%s password:%s\n", username, password);

            for(int i = 0; i < admins_count; i++){
                if( (strncmp(username, admins[i].username, strlen(admins[i].username)) == 0) 
                    && 
                    (strncmp(password, admins[i].password, strlen(admins[i].password)) == 0) ){
                    good_credentials_admin=TRUE;
                     printf("    DEBUG MSG: crdentials tested: username_admin:%s password:%s\n", admins[i].username, admins[i].password);
                    break;
                }
            }
            if(good_credentials_admin==FALSE){
                send(accepted_sockfd, "2", 1, 0);
                close(accepted_sockfd);
                printf("    *DEBUG MSG: no match for admin credentials. Continue to new accept\n");
                continue;///< go to new accept
            }

            //Good credentials and no admin connected
            admin_sockfd = accepted_sockfd;///< such that original fd is not lost
            //start thread for admin conn
            if( pthread_create(&id[0], NULL, (void *)adminThreadRoutine, (void *)&admin_sockfd) != 0 ){
                printf("[Server PID: %d] ERROR at create admin thread.\n",  getpid());
                exit(4);
            }



            


        

        } else if(strncmp(line, "1", 1) == 0){///< User login request
            
        
        }


    }
        
    printf("%s.End of main...\n",users[users_count-1].username);








}