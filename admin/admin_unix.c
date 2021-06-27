#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/un.h>

#define MAX 512
#define PORT 5000
#define SA struct sockaddr

#define SERVER_PATH "/tmp/unix_sock.server"
#define CLIENT_PATH "/tmp/unix_sock.client"

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

void comunicareSocketAdminServer()
{
    int n, optiune_request;
    char request2_categorie[50], request4_user[50], request5_user[50], request5_parola[50], request6_anunt[50], request6_categorie[50], request_concatenat[120];

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
        

        printf("Introduceti optiune: ");
        scanf("%d", &optiune_request);
        getchar();   // ar trebui sa rezolve problema cu fgets => non block (citeste \n din stdin de la scanf)
        
        bzero(request_concatenat, sizeof(request_concatenat));

        if(optiune_request == 2)
        // Request type 2 format => Creeaza cateorie noua : 2 nume_categorie
        {
            printf("Introduceti numele unei categorii noi: ");
            fgets(request2_categorie, sizeof(request2_categorie), stdin);
            request2_categorie[strcspn(request2_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s", "102", request2_categorie);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 3)
        // Request type 3 format => Vizualizare utilizatori conectati : 3
        {
            printf("Utilizatorii conectati sunt:\n");
            
            snprintf(request_concatenat, 120, "%s", "103");
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 4)
        // Request type 4 format => Baneaza client : 4 nume_user
        {
            printf("Introduceti numele unui client pe care doriti sa-l banati:\n");
            fgets(request4_user, sizeof(request4_user), stdin);
            request4_user[strcspn(request4_user, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s", "104", request4_user);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 5)
        // Request type 5 format => Creaza cont utilizator nou : 5 nume_user parola_user
        {
            printf("Introduceti nume user pe care doriti sa-l creati: ");
            fgets(request5_user, sizeof(request5_user), stdin);
            request5_user[strcspn(request5_user, "\n")] = 0;       //eliminare \n adaugat de fgets

            printf("\n Introduceti parola user pe care doriti sa-l creati: ");
            fgets(request5_parola, sizeof(request5_parola), stdin);
            request5_parola[strcspn(request5_parola, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "105", request5_user, request5_parola);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 6)
        // Request type 6 format => Stergere anunt : 6 categorie nume_anunt
        {
            // alegere categorie dorita pentru a publica anunt anunturile
            printf("\nAlegeti categoria din care doriti sa stergeti un anunt: ");
            fgets(request6_categorie, sizeof(request6_categorie), stdin);
            request6_categorie[strcspn(request6_categorie, "\n")] = 0;       //eliminare \n adaugat de fgets

            printf("Introduceti numele anuntului pe care doriti sa-l stergeti: ");
            fgets(request6_anunt, sizeof(request6_anunt), stdin);
            request6_anunt[strcspn(request6_anunt, "\n")] = 0;       //eliminare \n adaugat de fgets

            // construire request final de trimis catre server
            snprintf(request_concatenat, 120, "%s %s %s", "106", request6_categorie, request6_anunt);
            write(sockfd, request_concatenat, sizeof(request_concatenat));
        }
        else if(optiune_request == 7)
        {
            printf("[LOGOUT] Sesiunea curenta se inchide\n");     

            snprintf(request_concatenat, 120, "%s", "107");
            write(sockfd, request_concatenat, sizeof(request_concatenat));

            // doar in cazul logout citit raspuns de la server in interior, altfel nu mai este posibil dupa apel kill()..
            bzero(buff, sizeof(buff));     //golire buffer
            read(sockfd, buff, sizeof(buff));
            printf("\nRaspuns server : \n%s\n", buff);    

            // signal(SIGINT, handler_sigint);
            kill(getpid(), SIGINT);
        }

        bzero(buff, sizeof(buff));     //golire buffer
        read(sockfd, buff, sizeof(buff));
        printf("\nRaspuns server : %s\n", buff);

    }
}
  
int main(int argc, char *argv[])
{
    signal(SIGINT, handler_sigint);
    //struct sockaddr_in servaddr, cli;

    int rc, len;//client_sock, 
    struct sockaddr_un server_sockaddr; 
    struct sockaddr_un client_sockaddr; 
    char buf[256];
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
  
    // socket create and varification
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("[EROARE] creare socket esuata...\n");
        exit(0);
    }
    else
        printf("[SUCCES] socket creat cu succes..\n");

  
    // // assign IP, PORT
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    // servaddr.sin_port = htons(PORT);

    client_sockaddr.sun_family = AF_UNIX;   
    strcpy(client_sockaddr.sun_path, CLIENT_PATH); 
    len = sizeof(client_sockaddr);
    
    unlink(CLIENT_PATH);
    rc = bind(sockfd, (struct sockaddr *) &client_sockaddr, len);
    if (rc == -1){
        printf("BIND ERROR: \n");
        close(sockfd);
        exit(1);
    }
    //----------------------
  // connect the client socket to server socket
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    rc = connect(sockfd, (struct sockaddr *) &server_sockaddr, len);
    if(rc == -1){
        printf("[EROARE] contexiunea la server a esuat...\n");
        close(sockfd);
        exit(1);
    } else
        printf("[SUCCES] conectat cu succes la server..\n");

 

    //   ************* Request conn *************
    recv(sockfd, &buff, 100, 0); 

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
