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
        // strcpy(clients[client_count_local].username, strtok(client, " "));
        // strcpy(clients[client_count_local].password, strtok(NULL, "\n"));
        sscanf (client,"%s %s",clients[client_count_local].username,clients[client_count_local].password);

        clients[client_count_local].password[strcspn(clients[client_count_local].password, "\n")] = 0;       //eliminare \n adaugat de fgets
        client_count_local+=1;
    }
    *client_count = client_count_local;
    fclose(fp);

}

int send_file(int sockfd, char file_name[], char format[]){

    FILE *picture;
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];

    packet_index = 1;
    printf("numele imaginii %s\n", file_name);
    //char *img_name_png = malloc(sizeof(img_name) + 4);
    //strncpy(img_name_png, img_name, strlen(img_name) - 1); //numele fara \n
    //strcat(img_name_png, ".png");
    char file_and_format[50];
    snprintf(file_and_format, sizeof(file_and_format), "%s.%s", file_name, format);

    picture = fopen(file_and_format, "r");
    //bzero(img_name, sizeof(img_name));
    bzero(file_and_format, sizeof(file_and_format));

    printf("Getting File Size\n");   

    if(picture == NULL)
    {
        printf("Error Opening File\n\n"); 
        return 1;
    } 

    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    printf("Total File size: %i\n",size);

    //Send Picture Size
    printf("Sending File Size\n");
    write(sockfd, (void *)&size, sizeof(int));

    //Send Picture as Byte Array
    printf("Sending File as Byte Array\n");

    do
    { //Read while we get errors that are due to signals.
        stat=read(sockfd, &read_buffer , 255);
        printf("Bytes read: %i\n",stat);
    } while (stat < 0);

    printf("Received data in socket\n");
    printf("Socket data: %s\n", read_buffer);

    while(!feof(picture))
    {
    //while(packet_index = 1){
        //Read from the file into our send buffer
        read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, picture);

        //Send data through our socket 
        do{
            stat = write(sockfd, send_buffer, read_size);  
        }while (stat < 0);

        printf("Packet Number: %i\n",packet_index);
        printf("Packet Size Sent: %i\n",read_size);     
        printf(" \n");
        printf(" \n");

        packet_index++;  

        //Zero out our send buffer
        bzero(send_buffer, sizeof(send_buffer));
    }
    // recv(sockfd, &read_buffer, MAX, 0);
    // printf("%s", read_buffer);
}

int receive_file(int socket, char new_loc[], char categorie[]){
    int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
    printf("\t\t\t In receive image avem numele %s", new_loc);
    char imagearray[10241], verify = '1';
    FILE *image;

    //Find the size of the image
    do{
            stat = read(socket, &size, sizeof(int));
    }while(stat<0);

    printf("Packet received.\n");
    printf("Packet size: %i\n",stat);
    printf("File size: %i\n",size);
    printf(" \n");

    char buffer[] = "Got it";

    //Send our verification signal
    do{
        stat = write(socket, &buffer, sizeof(int));
    }while(stat<0);

    printf("Reply sent\n");
    printf("%s \n", new_loc);
    // char* new_loc_png = malloc(strlen(new_loc) + 5);

    char new_loc_cu_path[100];
    snprintf(new_loc_cu_path, sizeof(new_loc_cu_path), "./%s/%s", categorie, new_loc);
    // strncat(new_loc_png, new_loc, strlen(new_loc) - 1);
    // strcat(new_loc_png, ".png");

    image = fopen(new_loc_cu_path, "w");

    //free(new_loc_png);

    if( image == NULL) {
            printf("Error has occurred. File could not be opened\n");
            return -1;
    }

    //Loop while we have not received the entire file yet


    int need_exit = 0;
    struct timeval timeout = {10,0};

    fd_set fds;
    int buffer_fd, buffer_out;

    while(recv_size < size) {
    //while(packet_index < 2){

        FD_ZERO(&fds);
        FD_SET(socket,&fds);

        buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

        if (buffer_fd < 0)
            printf("error: bad file descriptor set.\n");

        if (buffer_fd == 0)
            printf("error: buffer read timeout expired.\n");

        if (buffer_fd > 0){
            do{
                read_size = read(socket,imagearray, 10241);
            }while(read_size <0);

            printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);


            //Write the currently read data into our image file
            write_size = fwrite(imagearray,1,read_size, image);
            printf("Written file size: %i\n",write_size);

            if(read_size !=write_size) {
            printf("error in read write\n");    }


            //Increment the total number of bytes read
            recv_size += read_size;
            packet_index++;
            printf("Total received file size: %i\n",recv_size);
            printf(" \n");
            printf(" \n");
        }

    }


  fclose(image);
  printf("File successfully Received!\n");
  return 1;
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
    char name_of_category[30];
    char full_path_category[50];
    char raspuns_pentru_admin_request102[100];

    //needed for request 103
    char connected_users_string[50*conn_users_count];

    //needed for request 104
    char username_ban[20];
    BOOL swap = FALSE;
    int _err;
    int statusCode;
    int path_max = 200;
    FILE *fp;
    int status;
    char path[path_max];
    char python_command[100];
    char raspuns_pentru_admin_request104[100];

    //needed for request 105
    char username_to_add[20];
    char password_to_add[20];
    char raspuns_pentru_admin_request105[100];
    char username_and_password[50];
    FILE* add_new_user;

    //needed for request 106
    char request6_categorie[50];
    char request6_numeanunt[50];
    char comanda_rm_anunt_din_categorie[120];
    char raspuns_pentru_admin_request106[120];


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
           //strcpy(full_path_category, "./");
           //strcat(full_path_category, name_of_category);
           snprintf(full_path_category, sizeof(full_path_category), "./%s", name_of_category);

            if (stat(full_path_category, &st) == -1) { ///<dir does NOT exist, can create
            
                printf("    ->DEBUG MSG: dir dose not exist, creating %s ...\n", full_path_category);    
                if (mkdir(full_path_category, 0777) != 0){ ///<error occurred at creating dir
                    printf("    -->DEBUG MSG: error at creating...\n");
                    // send(*_adminsockfd, "-1", 2, 0);
                }
                else //succes la creare director cu mkdir
                {
                    snprintf(raspuns_pentru_admin_request102, sizeof(raspuns_pentru_admin_request102), "Categoria %s a fost adaugata cu succes", name_of_category);
                    write(*_adminsockfd, raspuns_pentru_admin_request102, strlen(raspuns_pentru_admin_request102));
                }
            } else{///<dir does exist, cannot create
                printf("    ->DEBUG MSG: dir  exist at path...  %s\n", full_path_category);
                snprintf(raspuns_pentru_admin_request102, sizeof(raspuns_pentru_admin_request102), "Categoria %s exista deja", name_of_category);
                write(*_adminsockfd, raspuns_pentru_admin_request102, strlen(raspuns_pentru_admin_request102));
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
            write(*_adminsockfd, connected_users_string, strlen(connected_users_string));

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
                    printf("    DEBUG MSG: swapping users from index: %d \n", i);
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

        snprintf(python_command, sizeof(python_command), "python3 handle_request.py -f ban -u %s", username_ban);
        if (system(python_command) == -1){
            printf("    DEBUG MSG: ERROR at sytem call for py script \n");
        }

        // construire si trimitere raspuns catre admin pentru request-ul 4
        snprintf(raspuns_pentru_admin_request104, sizeof(raspuns_pentru_admin_request104), "User-ul cu numele %s a fost banat, iar conexiunea a fost inchisa!", username_ban);
        write(*_adminsockfd, raspuns_pentru_admin_request104, strlen(raspuns_pentru_admin_request104));






        /*======REQUEST 105======*/
        } else if(strncmp(_line, "105", 3) == 0){ ///< add user request
            printf("    DEBUG MSG: req. admin 105 \n");
            sscanf (_line,"%*s %s %s", username_to_add, password_to_add);
            
            //contruire format de adaugat in fisier : nume_user parola_user
            snprintf(username_and_password, sizeof(username_and_password), "\n%s %s", username_to_add, password_to_add);
            add_new_user = fopen("users.txt", "a");
            fputs(username_and_password, add_new_user);
            fclose(add_new_user);

            //construire si trimitere raspuns catre admin pentru request-ul 5
            snprintf(raspuns_pentru_admin_request105, sizeof(raspuns_pentru_admin_request105), "User-ul >%s< a fost creat cu succes!", username_to_add);
            write(*_adminsockfd, raspuns_pentru_admin_request105, strlen(raspuns_pentru_admin_request105));

        /*======REQUEST 106======*/
        // Request type 6 format => Stergere anunt : 6 categorie nume_anunt
        } else if(strncmp(_line, "106", 3) == 0){ ///< delete user entry request
            printf("    DEBUG MSG: req. admin 106  \n");
            sscanf (_line,"%*s %s %s", request6_categorie, request6_numeanunt);
            
            //construire path pentru stergere data anunt(imagine, descriere anunt)
            snprintf(comanda_rm_anunt_din_categorie, sizeof(comanda_rm_anunt_din_categorie), "rm ./%s/%s.*", request6_categorie, request6_numeanunt);

            if (system(comanda_rm_anunt_din_categorie) == -1)
            {
                printf("    DEBUG MSG: ERROR at sytem call for py script \n");
                //construire si trimitere raspuns catre admin pentru request-ul 5
                snprintf(raspuns_pentru_admin_request106, sizeof(raspuns_pentru_admin_request106), "Verificati daca numele anuntului >%s< este introdus corect!", request6_numeanunt);
            }
            else
            {
                printf("    DEBUG MSG: apelul system() pentru stergere anunt a fost executat cu succes.\n");
                //construire si trimitere raspuns catre admin pentru request-ul 5
                snprintf(raspuns_pentru_admin_request106, sizeof(raspuns_pentru_admin_request106), "Anuntul cu numele >%s< a fost sters cu succes!", request6_numeanunt);
            }

            write(*_adminsockfd, raspuns_pentru_admin_request106, strlen(raspuns_pentru_admin_request106));

        }// end of request 106

        else if(strncmp(_line, "107", 3) == 0)
        {
            write(*_adminsockfd, "[LOGOUT] Admin deconectat cu succes de la server", strlen("[LOGOUT] Admin deconectat cu succes de la server"));
            close(*_adminsockfd);
            
            //allow other admins to connect
            pthread_mutex_lock(&mutex);
            admin_is_conn = FALSE;
            pthread_mutex_unlock(&mutex);

            printf("[LOGOUT] Admin deconectat cu succes de la server.\n");
            pthread_exit(_adminsockfd);

        }

        printf("Line received:   >>>%s<<<\n",_line);

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

    // for(int i = 0; i < users_count; i++){
    //     printf("PASSWORD: %s <--->",users[i].password);
    //     printf("USERNAME: %s \n", users[i].username);

    // }
    printf("**DEBUG MSG: First admnin extract: username_admin:%s password:%s\n", admins[0].password, admins[0].username); 
    //Main local variabiles 
    //(to be inherited by children processes and shared with threads)
    struct sockaddr_in cli_addr, serv_addr;///< addresses for client and server

    int client_len;
    int accepted_sockfd;///< to catch new client conn
    int rc;///< receive count; to read data from client
    char line[MAXLINE];///< line to read through socket

    conn_users_count = 0;

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
            bzero(username, sizeof(username));
            bzero(password, sizeof(password));

            sscanf (line,"%*s %s %s",username,password);
            password[strcspn(password, "\n")] = 0;
            printf("DEBUG MSG: client crdentials extracted: username:%s...password:%s...\n", username, password);

            pid_t pid_client = fork();
            if(pid_client == -1){printf("DEBUG MSG: client fork failed \n"); exit(5);}


/*
    P: 3
    C: 3 (id al copil)

    P: 3+1 =4 ..
    C:


*/
            /*======Solve client process======*/            
            if(pid_client == 0){
                //is this user already connected?
                printf("^DEBUG MSG: child fork ID: %d \n", getpid());
                //sleep(1);
                for(int i = 0; i< conn_users_count; i++){
                    // printf("  DEBUG MSG: is user connected...");
                    // printf("\n");
                    if(strncmp(username, connected_users[i].username, strlen(connected_users[i].username))==0){
                        send(accepted_sockfd, "2", 1, 0);///< user connected
                        printf("DEBUG MSG: client fork ended; user already connected (2) \n");
                        kill(getpid(), SIGTERM);
                        break;
                    }
                }
                
                //are credentials good?
                BOOL x = FALSE;
                for(int i= 0; i<users_count; i++){
                    printf("  ->DEBUG MSG: comapre: username:%s...password:%s...\n", users[i].username, users[i].password);
                    if(strlen(username) != strlen(users[i].username)){
                        continue;
                    }
                    if(// both username and password must match
                    strncmp(username, users[i].username,strlen(users[i].username))==0 &&
                    strncmp(password, users[i].password,strlen(users[i].password))==0){///<
                        x=TRUE;
                        send(accepted_sockfd, "1", 1, 0);///< send 0 for successfull login 
                        printf("DEBUG MSG: client fork continue; user connected! \n");
                        break;
                    }


                    



                }

                //if no user found
                if(x == FALSE){
                    send(accepted_sockfd, "3", 1, 0);///< user not found
                    printf("DEBUG MSG: client fork ended; no user found (3) \n");
                    kill(getpid(), SIGTERM);
                }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                printf("   ->DEBUG MSG: aici se implementeaza req!\n");
                // declarare variabile pentru implemetare req pentru client
                int _rc_client;
                char _line_client[MAXLINE];

                //needed for request 2
                char raspuns_pentru_client_request2[200];
                char list_dir_command[100];
                FILE *fp_client_req2;
                int currentChar;
                char currentCharAsChar[10];

                //needed for request 3
                char raspuns_pentru_client_request3[200], list_anunt_command_req3[100], categorie_req3[30];
                FILE *fp_client_req3;
                int currentCharReq3, counterReq3 = 0;
                char currentCharAsCharReq3[5];

                //needed for request 4
                char raspuns_pentru_client_request4[200], categorie_req4[30], nume_anunt_req4[30], categorie_and_anunt_req4[70], buffer_local[MAXLINE]; 

                //needed for request 5
                char raspuns_pentru_client_request5[200], nume_anunt_req5[30], nume_owner_anunt_req5[30], categorie_req5[30], nume_anunt_and_owner_req5_txt[65], nume_anunt_and_owner_req5_png[65];

                //needed for request 6
                char raspuns_pentru_client_request6[200], rm_anunt_command_req6[80], categorie_req6[30], nume_anunt_req6[30];

                while(_rc_client = recv(accepted_sockfd, &_line_client, MAXLINE, 0))
                {
                    // printf("; rc is: %d\n", rc);
                    _line_client[_rc_client] = '\0';

                    /*======REQUEST 2======*/
                    if(strncmp(_line_client, "2", 1) == 0)
                    // Vizualizare categorii anunturi.         Req format>> 2
                    {
                        printf("Se ajunge in req2\n\n");
                        // TODO: returneaza toate directoarele din ./
                        bzero(raspuns_pentru_client_request2, sizeof(raspuns_pentru_client_request2));  //golrie char array pentru eliminare primire garbage
                        snprintf(list_dir_command, sizeof(list_dir_command), "ls -d */ | cut -f1 -d'/' > result.txt");
                        if (system(list_dir_command) != -1)
                        {
                            printf("    DEBUG MSG: Comanda prin apel system a fost executata cu succes\n");
                        }
                        else
                        {
                            printf("    DEBUG MSG: system() call failed\n");
                        }
                        sleep(0.1);
                        fp_client_req2 = fopen("result.txt", "r");
                        if(fp_client_req2 != NULL)
                        {
                            // read a single char in for each loop iteration
                            while ((currentChar = fgetc(fp_client_req2)) != EOF)
                            {
                                snprintf(currentCharAsChar, sizeof(currentCharAsChar), "%c", currentChar);
                                strcat(raspuns_pentru_client_request2, currentCharAsChar);
                            }
                        }
                        write(accepted_sockfd, raspuns_pentru_client_request2, strlen(raspuns_pentru_client_request2));
                    } //end of request 2

                     /*======REQUEST 3======*/
                    else if(strncmp(_line_client, "3", 1) == 0)
                    // Vizualizare anunturi din categorie.         Req format>> 3 categorie
                    {
                        printf("Se ajunge in req3, playload ---%s---\n\n", _line_client);
                        //TODO: returneaza toate anunturile din categoria aleasa > ls categorie/*txt | cut -f2 -d'/' | cut -f1 -d'.'
                        bzero(raspuns_pentru_client_request3, sizeof(raspuns_pentru_client_request3));  //golrie char array pentru eliminare primire garbage
                        // Construire comanda pentru apel system()
                        sscanf(_line_client,"%*s %s", categorie_req3);
                        snprintf(list_anunt_command_req3, sizeof(list_anunt_command_req3), "ls %s/*txt | cut -f2 -d'/' | cut -f1 -d'.' > result.txt", categorie_req3);
                        if (system(list_anunt_command_req3) != -1)
                        {
                            printf("   DEBUG MSG: Comanda system(ls %s/*txt | cut -f2 -d'/' | cut -f1 -d'.' > result.txt) a fost executata cu succes\n\n", categorie_req3);
                        }
                        sleep(0.1);
                        fp_client_req3 = fopen("result.txt", "r");
                        if(fp_client_req3 != NULL)
                        {
                            // read a single char in for each loop iteration
                            while ((currentCharReq3 = fgetc(fp_client_req3)) != EOF)
                            {
                                snprintf(currentCharAsCharReq3, sizeof(currentCharAsCharReq3), "%c", currentCharReq3);
                                strcat(raspuns_pentru_client_request3, currentCharAsCharReq3);
                            }
                        }
                        printf("ciudat=%s.", raspuns_pentru_client_request3);
                        write(accepted_sockfd, raspuns_pentru_client_request3, strlen(raspuns_pentru_client_request3));
                    } //end of request 3

                     /*======REQUEST 4======*/
                    else if(strncmp(_line_client, "4", 1) == 0)
                    // Request type 4 format => Vizualizare anunt specific : 4 categorie nume_anunt nume_owner (server sends => client receives)
                    {
                        printf("Se ajunge in req4, playload ---%s---\n\n", _line_client);
                        //char raspuns_pentru_client_request4[200], categorie_req4[30], nume_anunt_req4[30], categorie_and_anunt_req4[70]; 
                        bzero(categorie_req4, sizeof(categorie_req4));
                        bzero(nume_anunt_req4, sizeof(nume_anunt_req4));
                        bzero(categorie_and_anunt_req4, sizeof(categorie_and_anunt_req4));

                        sscanf(_line_client, "%*s %s %s", categorie_req4, nume_anunt_req4);

                        snprintf(categorie_and_anunt_req4, sizeof(categorie_and_anunt_req4), "./%s/%s", categorie_req4, nume_anunt_req4);

                        send_file(accepted_sockfd ,categorie_and_anunt_req4, "png");
                        bzero(buffer_local, sizeof(buffer_local));     //golire buffer
                        read(accepted_sockfd, buffer_local, sizeof(buffer_local));
                        printf("Raspuns client : %s\n", buffer_local);


                        send_file(accepted_sockfd ,categorie_and_anunt_req4, "txt");
                        bzero(buffer_local, sizeof(buffer_local));     //golire buffer
                        read(accepted_sockfd, buffer_local, sizeof(buffer_local));
                        printf("Raspuns client : %s\n", buffer_local);
                    } //end of request 4


                     /*======REQUEST 5======*/
                    else if(strncmp(_line_client, "5", 1) == 0)
                    // Adaugare anunt in categorie.         Req format>> 5 categorie nume_anunt username_owner
                    {
                    printf("Se ajunge in req5, playload ---%s---\n\n", _line_client);
                    
                    bzero(nume_anunt_req5, sizeof(nume_anunt_req5));
                    bzero(nume_anunt_and_owner_req5_txt, sizeof(nume_anunt_and_owner_req5_txt));
                    bzero(nume_owner_anunt_req5, sizeof(nume_owner_anunt_req5));

                    sscanf(_line_client, "%*s %s %s %s", categorie_req5, nume_anunt_req5, nume_owner_anunt_req5);

                    snprintf(nume_anunt_and_owner_req5_txt, sizeof(nume_anunt_and_owner_req5_txt), "%s_%s.txt", nume_anunt_req5, nume_owner_anunt_req5);
                    snprintf(nume_anunt_and_owner_req5_png, sizeof(nume_anunt_and_owner_req5_png), "%s_%s.png", nume_anunt_req5, nume_owner_anunt_req5);
                    // //receive_txt_file(nume_anunt_and_owner_req5_txt);

                    receive_file(accepted_sockfd, nume_anunt_and_owner_req5_png, categorie_req5);

                    snprintf(raspuns_pentru_client_request5, sizeof(raspuns_pentru_client_request5), "Imaginea din anuntul cu numele >%s< a fost salvata cu succes!", nume_anunt_req5);
                    write(accepted_sockfd, raspuns_pentru_client_request5, strlen(raspuns_pentru_client_request5));

                    bzero(raspuns_pentru_client_request5, sizeof(raspuns_pentru_client_request5));
                    receive_file(accepted_sockfd, nume_anunt_and_owner_req5_txt, categorie_req5);

                    snprintf(raspuns_pentru_client_request5, sizeof(raspuns_pentru_client_request5), "Descrierea anuntului cu numele >%s< a fost salvata cu succes!", nume_anunt_req5);
                    write(accepted_sockfd, raspuns_pentru_client_request5, strlen(raspuns_pentru_client_request5));

                    } //end of request 5


                      /*======REQUEST 6======*/
                    else if(strncmp(_line_client, "6", 1) == 0)
                    // Request type 6 format => Sterge anunt : 6 categorie nume_anunt
                    // sterge dar e incomplet (poate sterge un anunt al altui user..trebuie adaugata o verificare pentru detinator anunt==user care vrea sa stearga)
                    {
                        //char raspuns_pentru_client_request6[200], rm_anunt_command_req6[50], categorie_req6[30], nume_anunt_req6[30];
                        printf("Se ajunge in req6, playload ---%s---\n\n", _line_client);
                        bzero(raspuns_pentru_client_request6, sizeof(raspuns_pentru_client_request6));
                        // TODO: sterge anunt din categoria dorita > rm ./categorie/nume_anunt.*
                        sscanf(_line_client,"%*s %s %s", categorie_req6, nume_anunt_req6);
                        snprintf(rm_anunt_command_req6, sizeof(rm_anunt_command_req6), "rm ./%s/%s.*", categorie_req6, nume_anunt_req6);

                        if (system(rm_anunt_command_req6) != -1)
                        {
                            printf("   DEBUG MSG: Comanda system(rm ./categorie/nume_anunt.*) a fost executata cu succes\n\n");
                        }

                        snprintf(raspuns_pentru_client_request6, sizeof(raspuns_pentru_client_request6), "Anuntul >%s< din categoria >%s< a fost sters cu succes", nume_anunt_req6, categorie_req6);
                        write(accepted_sockfd, raspuns_pentru_client_request6, strlen(raspuns_pentru_client_request6));
                    } //end of request 6

                }// acolada while recv



            }
            /*================================*/
            /*=========Parent process=========*/
            else if(pid_client > 0){
                connected_users[conn_users_count].pid_child = pid_client;
                //strcpy(connected_users[conn_users_count].username, username);
                sscanf (username,"%s",connected_users[conn_users_count].username);

                // pthread_mutex_lock(&mutex);
                conn_users_count=conn_users_count+1;
                // pthread_mutex_unlock(&mutex);
printf("\n>>>DEBUG MSG: parent after child;  conn_users_count=%d usr=%s\n",conn_users_count, connected_users[conn_users_count-1].username);
            }
        }


    }









}