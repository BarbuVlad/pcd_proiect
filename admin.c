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
#define SA struct sockaddr
 
int sockfd;
char buff[MAX]; //buffer pentru citire din socket
 
// handler pentru primire semnal SIGINT
void handler_sigint()
{
	printf("[LOGOUT] Handler SIGINT (logout) => sesiunea curenta se inchide\n");
	send(sockfd, "107", 3, 0);
	close(sockfd);
	exit(0);
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
 
   if(picture == NULL) {
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
 
   do { //Read while we get errors that are due to signals.
      stat=read(sockfd, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);
 
   printf("Received data in socket\n");
   printf("Socket data: %s\n", read_buffer);
 
   while(!feof(picture)) {
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
 
void comunicareSocketAdminServer()
{
    int n, optiune_request;
    char request2_categorie[50], request4_user[50], request5_user[50], request5_parola[50], request6_anunt[50], request_concatenat[120];
 
 char e[50];
    while (1)
    {
        printf("\t**************************************************************************\n");
        printf("\t2. Creeaza categorie noua.                 Req format>> 2 categorie\n");
        printf("\t3. Vizualizare utilizatori conectati.      Req format>> 3\n");
        printf("\t4. Baneaza client.                         Req format>> 4 user\n");
        printf("\t5. Creaza cont utilizator nou.             Req format>> 5 user parola\n");
        printf("\t6. Sterge anunt.                           Req format>> 6 nume_anunt\n");
        printf("\t7. Logout                                  Req format>> 7\n");
        printf("\t**************************************************************************\n\n");
 
        bzero(buff, sizeof(buff));     //golire buffer
 
        printf("Introduceti optiune: ");
        scanf("%d", &optiune_request);
        fgets(e, sizeof(e), stdin);

        if(optiune_request == 2)
        {
            printf("Introduceti numele unei categorii noi: ");
            fgets(request2_categorie, sizeof(request2_categorie), stdin);
            request2_categorie[strcspn(request2_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets
 
            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s", "102", request2_categorie);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 3)
        {
            printf("Utilizatorii conectati sunt:\n");
 
            snprintf(request_concatenat, 120, "%s", "103");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 4)
        {
            printf("Introduceti numele unui client pe care doriti sa-l banati:\n");
            fgets(request4_user, sizeof(request4_user), stdin);
            request4_user[strcspn(request4_user, "\n")] = 0;       //eliminare \n adaugat de fgets
 
            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s", "104", request4_user);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 5)
        {

            printf("Introduceti nume user pe care doriti sa-l creati: ");
            fflush(stdin);
            fflush(stdout);
            fgets(request5_user, sizeof(request5_user), stdin);
            request5_user[strcspn(request5_user, "\n")] = 0;       //eliminare \n adaugat de fgets
 
            printf("\n Introduceti parola user pe care doroti sa-l creati: ");
            fgets(request5_parola, sizeof(request5_parola), stdin);
            request5_parola[strcspn(request5_parola, "\n")] = 0;       //eliminare \n adaugat de fgets
            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "105", request5_user, request5_parola);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        
        }
        else if(optiune_request == 6)
        {
            printf("Introduceti numele anuntului pe care doriti sa-l stergeti: ");
            fgets(request6_anunt, sizeof(request6_anunt), stdin);
            request6_anunt[strcspn(request6_anunt, "\n")] = 0;       //eliminare \n adaugat de fgets
 
            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s", "106", request6_anunt);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 7)
        {
            printf("[LOGOUT] Sesiunea curenta se inchide\n");     
 
            snprintf(request_concatenat, 120, "%s", "107");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
 
            signal(SIGINT, handler_sigint);
            kill(getpid(), SIGINT);
            exit(0);
 
        }
 
 
        //vechea abordare
                // n = 0;
                // while ((buff[n++] = getchar()) != '\n');
 
                // write(sockfd, buff, sizeof(buff));
 
                // char* option = strtok(buff, " ");        //de modificat dupa prima testare tot ce este char* in char[]
                // if(strcmp(option, "1")==0)
                // {
 
                //     char* img_name;
                //     img_name = strtok(NULL, " ");
                //     if (send_image(img_name) == 1){
                //         //printf("Eroare la trimitere imagine!\n");
                //         continue;
                //     }
 
                // }   
 
                // bzero(buff, sizeof(buff));
                // read(sockfd, buff, sizeof(buff));
 
                // printf("Raspuns server : %s\n\n", buff);
 
                // if ((strncmp(buff, "O zi buna.", sizeof("O zi buna."))) == 0) {
                //     signal(SIGINT, handler_sigint);
                // 	kill(getpid(), SIGINT);
                //     break;
                // }
 
 
 
        //recv(sockfd, &buff, MAX, 0);
        //printf("%s", buff);
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
    user_and_parola[0] = '\0';                         //make sure its empty
    snprintf(user_and_parola, sizeof(user_and_parola), "%s %s %s", "101", user, parola);
 
    write(sockfd, user_and_parola, sizeof(user_and_parola));
    recv(sockfd, &buff, 100, 0);  //citim raspuns server relativ la login(succes, cont/parola gresita, admin deja conectat)
 
    printf("Incercare autentificare:\n");
    if(strncmp(buff, "1", 1) ==0)
    {
        printf("\t[SUCCES] Admin conectat cu succes la server, incepe comunicarea.\n\n");
        comunicareSocketAdminServer();
    }
    else if(strncmp(buff, "2", 1) ==0)
    {
        printf("\t[LOGIN INCORECT] Datele de login sunt incorecte, verifica user si/sau parola si reincearca.\n\n");
    }
    else if(strncmp(buff, "3", 1) ==0)
    {
        printf("\t[ADMIN DEJA CONECTAT] Reincearca mai tarziu.\n\n%s\n", buff);
    }
 
    // close the socket
    close(sockfd);
}