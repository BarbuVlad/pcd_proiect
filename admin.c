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

            printf("\n Introduceti parola user pe care doroti sa-l creati: ");
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

            signal(SIGINT, handler_sigint);
            kill(getpid(), SIGINT);
            exit(0);

        }

        bzero(buff, sizeof(buff));     //golire buffer
        read(sockfd, buff, sizeof(buff));
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
    bzero(user_and_parola, sizeof(user_and_parola));                    //make sure its empty
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
