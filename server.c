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
    ssize_t a = send(*_adminsockfd, "1", 1, 0);
    printf("  DEBUG MSG: good admin credentials. In thread created. Sent code 1 %ld\n",a);

    int _rc;
    char _line[MAXLINE];
    //char* req_type;

    //needed for request 102
    char name_of_category[MAXLINE];
    char full_path_category[MAXLINE];

    //needed for request 103
    char connected_users_string[50*conn_users_count];

    //needed for request 104
    char username_ban[50];
    BOOL swap = FALSE;
    int _err;
    int statusCode;

    while(_rc = recv(*_adminsockfd, &_line, MAXLINE, 0)){
        _line[_rc] = '\0';

        /*======REQUEST 102======*/
        if(strncmp(_line, "102", 3) == 0){ ///< create new category request
            /*
            Body: 102 name_of_category
            Action: extarct name_of_category and create a new directory in server root
            */
           sscanf (_line,"%*s %s",name_of_category);
           printf("    DEBUG MSG: req. admin 102 %s \n",name_of_category);

           struct stat st = {0};
           strcpy(full_path_category, "./");
           strcat(full_path_category, name_of_category);

            if (stat(full_path_category, &st) == -1) { ///<dir does NOT exist, can create
                printf("    ->DEBUG MSG: dir dose not exist, creating %s ...\n",full_path_category);
                if (mkdir(full_path_category, 0777) != 0){ ///<error occurred at creating dir
                    printf("    -->DEBUG MSG: error at creating...\n");
                    // send(*_adminsockfd, "-1", 2, 0);
                }
            } else{///<dir does exist, cannot create
                printf("    ->DEBUG MSG: dir  exist at path...  %s\n",full_path_category);
                // send(*_adminsockfd, "-2", 2, 0);
            }

        /*======REQUEST 103======*/
        } else if(strncmp(_line, "103", 3) == 0){ ///< view connected users request
            printf("    DEBUG MSG: req. admin 103  \n");
            strcpy(connected_users_string, "");
            for(int i = 0; i<conn_users_count; i++){
                strcat(connected_users_string, connected_users[i].username);
                strcat(connected_users_string, " ");
            }
            send(*_adminsockfd, connected_users_string, strlen(connected_users_string), 0);

        /*======REQUEST 104======*/
        } else if(strncmp(_line, "104", 3) == 0){ ///< ban client request
            printf("    DEBUG MSG: req. admin 104  \n");
            //extarct username
            sscanf (_line,"%*s %s",username_ban);
            //is this user connected?
            for(int i = 0; i<conn_users_count; i++){
                if (strncmp(username_ban, connected_users[i].username, strlen(connected_users[i].username)) == 0){
                    //connected user found - save pid
                    //pid_ban = connected_users[i].pid_child;
                    //kill child process TODO: handle this signal (close fd and send to client smth)
                    printf("    ->DEBUG MSG: user is connected; killing process...  \n");
                    kill(connected_users[i].pid_child, SIGTERM);
                    swap=TRUE;
                }
                if(swap==TRUE){
                    printf("     ->DEBUG MSG: swap data in connected_users...  \n");
                    connected_users[i].pid_child = connected_users[i + 1].pid_child;
                    strcpy(connected_users[i].username, connected_users[i + 1].username);
                }
            }
            if(swap==TRUE){
                pthread_mutex_lock(&mutex);
                conn_users_count--;
                pthread_mutex_unlock(&mutex);
            }

            //delete user from users list
            swap = FALSE;
            for(int i = 0; i<users_count; i++){
                if (strncmp(username_ban, users[i].username, strlen(users[i].username)) == 0){
                    swap=TRUE;///< start to swap from this i index
                }
                if(swap==TRUE){
                    strcpy(users[i].username, users[i + 1].username);
                    strcpy(users[i].password, users[i + 1].password);
                }
            }

            //call .py to:
                //delete from users.txt entry
                //add to users_ban.txt
                //delete objects added by that user
            int pid_py_exec = fork();
            if(pid_py_exec == -1){printf("    DEBUG MSG: ERROR at fork for python exec  \n");}

            if(pid_py_exec == 0){ //Child process (exec py script)
                _err = execlp("python3", "python3", "", username_ban, (char*) NULL);
                 if(_err==-1){printf("    DEBUG MSG: ERROR at python exec  \n");}
            }   else {//Parent process
                waitpid(pid_py_exec, &_err, 0); //_err used as __stat_loc
                 if(WIFEXITED(_err)){
                     //EXECUTION TERMINATED SUCCESSFULLY (for the waited child)
                    statusCode = WEXITSTATUS(_err);///< returned value or exit code value
                     if(statusCode==0){///< if code 0 is success
                        //SUCCESSFUL EXECUTION FROM CHILD
                        printf("    DEBUG MSG: Successfull at python exec  \n");
                    } else {
                        //EXECUTION FAILURE FROM CHILD
                         printf("    DEBUG MSG: ERROR at python exec...  \n");
                    }
                 }
            }
        
            //python3 handle_request.py -ban -user1

        /*======REQUEST 105======*/
        } else if(strncmp(_line, "105", 3) == 0){ ///< add user request
            printf("    DEBUG MSG: req. admin 105 \n");

        /*======REQUEST 106======*/
        } else if(strncmp(_line, "106", 3) == 0){ ///< delete user entry request
            printf("    DEBUG MSG: req. admin 106  \n");
        }

        

        printf("Line received %s\n",_line);


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

    printf("**DEBUG MSG: First admnin extract: username_admin:%s password:%s\n", admins[0].password, admins[0].username); 
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