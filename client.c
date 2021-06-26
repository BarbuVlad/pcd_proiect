#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX 512
#define IP_ADDRESS "127.0.0.1"
#define PORT 5000
#define SIZE_TXT_FILE 1024
#define SA struct sockaddr

int sockfd;
char buff[MAX]; //buffer pentru citire din socket

// handler pentru primire semnal SIGINT
void handler_sigint()
{
	fprintf(stderr,">>>Iesire\n>>>CLIENT a solicitat logout, clientul CLIENT se inchide\n");
	send(sockfd, "7", 1, 0);
	close(sockfd);
	exit(0);
}

void send_file_txt(FILE *fp)
{
    int n;
    char data[SIZE_TXT_FILE] = {0};

    while(fgets(data, SIZE_TXT_FILE, fp) != NULL)
    {
        if (send(sockfd, data, sizeof(data), 0) == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }
    bzero(data, SIZE_TXT_FILE);
    }
}

int send_image(char *img_name){

    FILE *picture;
    int size, read_size, stat, packet_index;
    char send_buffer[10240], read_buffer[256];

    packet_index = 1;
    printf("numele imaginii %s\n", img_name);
    char *img_name_png = malloc(sizeof(img_name) + 4);
    strncpy(img_name_png, img_name, strlen(img_name) - 1); //numele fara \n
    strcat(img_name_png, ".png");

    picture = fopen(img_name_png, "r");
    img_name_png = NULL;
    img_name = NULL;
    free(img_name_png);

    printf("Getting Picture Size\n");   

    if(picture == NULL)
    {
        printf("Error Opening Image File\n\n"); 
        return 1;
    } 

    fseek(picture, 0, SEEK_END);
    size = ftell(picture);
    fseek(picture, 0, SEEK_SET);
    printf("Total Picture size: %i\n",size);

    //Send Picture Size
    printf("Sending Picture Size\n");
    write(sockfd, (void *)&size, sizeof(int));

    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");

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


void comunicareSocketClientServer()
{
    int n, optiune_request;
    char request3_categorie[50], request4_categorie[50], request4_nume_anunt[50], request5_categorie[50], request5_nume_anunt[50], request6_categorie[50], request6_nume_anunt[50], request_concatenat[120];

    while (1)
    {
        printf("\t**********************************************************************************\n");
        printf("\t2. Vizualizare categorii anunturi.         Req format>> 2\n");
        printf("\t3. Vizualizare anunturi din categorie.     Req format>> 3 categorie\n");
        printf("\t4. Vizualizare anunt specific.             Req format>> 4 nume_anunt\n");
        printf("\t5. Adauga anunt nou.                       Req format>> 5 nume_anunt_de_adaugat\n");
        printf("\t6. Sterge anunt propriu.                   Req format>> 6 nume_anunt_propriu\n");
        printf("\t7. Logout                                  Req format>> 7\n");
        printf("\t**********************************************************************************\n\n");
        

        printf("Introduceti optiune: ");
        scanf("%d", &optiune_request);
        getchar();   // ar trebui sa rezolve problema cu fgets => non block (citeste \n din stdin de la scanf)
        
        bzero(request_concatenat, sizeof(request_concatenat));

        if(optiune_request == 2)
        // Request type 2 format => Vizualizare categorii de anunturi : 2
        {
            printf("Categoriile anunturilor sunt:\n");
            
            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s", "2");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 3)
        // Request type 3 format => Vizualizare anunturi din categorie : 3 categorie
        {
            printf("Categoriile anunturilor sunt:\n");

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s", "2");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
            
            // citire raspuns server cu lista de categorii
            bzero(buff, sizeof(buff));     //golire buffer
            read(sockfd, buff, sizeof(buff));
            printf("Raspuns server : %s\n", buff);
            
            // alegere categorie dorita pentru a vedea anunturile
            printf("\nAlegeti categoria dorita pentru a vedea anunturile din categoria aleasa:\n");
            fgets(request3_categorie, sizeof(request3_categorie), stdin);
            request3_categorie[strcspn(request3_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            snprintf(request_concatenat, 120, "%s %s", "3", request3_categorie);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 4)
        // Request type 4 format => Vizualizare anunt specific : 4 categorie nume_anunt
        {
            printf("Categoriile anunturilor sunt:\n");

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s", "2");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
            
            // citire raspuns server cu lista de categorii
            bzero(buff, sizeof(buff));     //golire buffer
            read(sockfd, buff, sizeof(buff));
            printf("Raspuns server : %s\n", buff);

            // alegere categorie dorita pentru a vedea anunturile
            printf("\nAlegeti categoria dorita pentru a vedea anunturile din categoria aleasa:\n");
            fgets(request4_categorie, sizeof(request4_categorie), stdin);
            request4_categorie[strcspn(request4_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            snprintf(request_concatenat, 120, "%s %s", "3", request4_categorie);
            write(sockfd, request_concatenat, sizeof(request_concatenat));

            // citire raspuns server cu lista de categorii
            bzero(buff, sizeof(buff));     //golire buffer
            read(sockfd, buff, sizeof(buff));
            printf("Raspuns server : %s\n", buff);

            // alegere anunt din categorie pentru vizualizare
            printf("Introduceti numele anuntului din categoria >%s< pe care doriti sa-l vizualizati: ", request4_categorie);
            fgets(request4_nume_anunt, sizeof(request4_nume_anunt), stdin);
            request4_nume_anunt[strcspn(request4_nume_anunt, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "4", request4_categorie, request4_nume_anunt);
            write(sockfd, request_concatenat, sizeof(request_concatenat));

            // TODO: de apelat receive_image si receive_txt_file
            // ...
        }
        else if(optiune_request == 5)
        // Request type 5 format => Adauga anunt : 5 categorie nume_anunt
        {
            printf("Categoriile disponibile in care puteti adauga anunt sunt:\n");

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s", "2");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
            
            // citire raspuns server cu lista de categorii disponibile in care se poate adauga anunt
            bzero(buff, sizeof(buff));     //golire buffer
            read(sockfd, buff, sizeof(buff));
            printf("Raspuns server : %s\n", buff);

            // alegere categorie dorita pentru a publica anunt anunturile
            printf("\nAlegeti categoria in care doriti sa adaugati un anunt: ");
            fgets(request5_categorie, sizeof(request5_categorie), stdin);
            request5_categorie[strcspn(request5_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            //alegere nume anunt pentru a fi publicat
            printf("\nAlegeti titlul anuntului pe care doriti sa-l publicati in categoria >%s<: ", request5_categorie);
            fgets(request5_nume_anunt, sizeof(request5_nume_anunt), stdin);
            request5_nume_anunt[strcspn(request5_nume_anunt, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "5", request5_categorie, request5_nume_anunt);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 6)
        // Request type 6 format => Sterge anunt : 6 categorie nume_anunt
        {
            // alegere categorie dorita pentru a publica anunt anunturile
            printf("\nAlegeti categoria din care doriti sa stergeti anuntul dvs.: ");
            fgets(request6_categorie, sizeof(request6_categorie), stdin);
            request6_categorie[strcspn(request6_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            // alegere nume anunt dorit pentru stergere
            printf("\nIntroduceti numele anuntului pe care doriti sa-l stergeti: ");
            fgets(request6_nume_anunt, sizeof(request6_nume_anunt), stdin);
            request6_nume_anunt[strcspn(request6_nume_anunt, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "6", request6_categorie, request6_nume_anunt);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 7)
        // Request type 6 format => Logout: 7
        {
            printf("[LOGOUT] Sesiunea curenta se inchide\n");     

            snprintf(request_concatenat, 120, "%s", "7");
            write(sockfd, request_concatenat, sizeof(request_concatenat));

            signal(SIGINT, handler_sigint);
            kill(getpid(), SIGINT);
            exit(0);
        }

        bzero(buff, sizeof(buff));             // golire buffer
        read(sockfd, buff, sizeof(buff));      // citire raspuns server final server pentru un request dat anterior
        printf("Raspuns server : %s\n", buff);

    }
}


int main(int argc, char *argv[])
{
    int opt;   // pentru getopt
    char user[50], parola[50], user_and_parola[110];    //folosite pentru login
    struct sockaddr_in servaddr, cli;
  
    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("[EROARE] creare socket esuata...\n");
        exit(0);
    }
    else
        printf("[SUCCES] socket creat cu succes..\n");

    bzero(&servaddr, sizeof(servaddr));
  
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    servaddr.sin_port = htons(PORT);
  
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("[EROARE] contexiunea la server a esuat...\n");
        exit(0);
    }
    else
        printf("[SUCCES] conectat cu succes la server..\n");

    // daca conectarea la server a avut succes atunci citim user si paroola in vederea autentificarii
    snprintf(user, sizeof(user), "%s", "no_user_inserted");
    snprintf(parola, sizeof(parola), "%s", "no_password_inserted");

    while ((opt = getopt(argc, argv, "u:p:")) != -1)
    {
        switch (opt)
        {
        case 'u':
            user[0] = '\0';
            strcpy(user, optarg);
            break;
        case 'p':
            parola[0] = '\0';
            strcpy(parola, optarg);
            break;
        }
    }
 

    //   ************* Request login de forma    => 1 user parola *************
    // trimite credentiale pentru login
    bzero(user_and_parola, sizeof(user_and_parola));                         //make sure its empty
    snprintf(user_and_parola, sizeof(user_and_parola), "%s %s %s", "1", user, parola);

    write(sockfd, user_and_parola, sizeof(user_and_parola));
    recv(sockfd, &buff, 100, 0);  //citim raspuns server relativ la login(succes, cont/parola gresita, admin deja conectat)

    printf("Incercare autentificare:\n");
    if(strncmp(buff, "1", 1) ==0)
    {
        printf("\t[SUCCES] Client conectat cu succes la server, incepe comunicarea.\n\n");
        comunicareSocketClientServer();
    }
    else if(strncmp(buff, "2", 1) ==0)
    {
        printf("\t[LOGIN INCORECT] Datele de login sunt incorecte, verifica user si/sau parola si reincearca.\n\n");
    }
    else if(strncmp(buff, "3", 1) ==0)
    {
        printf("\t[LOGIN INCORECT] Datele de login sunt incorecte, verifica user si/sau parola si reincearca.\n\n");
    }

    // close the socket
    close(sockfd);
}