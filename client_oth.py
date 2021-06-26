import socket
import sys
import signal
import getopt
import time

IP_ADDRESS = '127.0.0.1'
PORT = 5000

argv = sys.argv[1:]
user = "no_user_inserted"
parola = "no_password_inserted"

def handler_sigint():
    print("[SIGNAL] inchid client OTH")
    exit(0)


def comunicareSocketClientSever():
    while True:
        print("**********************************************************************************")
        print("\t2. Vizualizare categorii anunturi.         Req format>> 2\n")
        print("\t3. Vizualizare anunturi din categorie.     Req format>> 3 categorie\n")
        print("\t4. Vizualizare anunt specific.             Req format>> 4 nume_anunt\n")
        print("\t5. Adauga anunt nou.                       Req format>> 5 nume_anunt_de_adaugat\n")
        print("\t6. Sterge anunt propriu.                   Req format>> 6 nume_anunt_propriu\n")
        print("\t7. Logout                                  Req format>> 7\n")
        print("**********************************************************************************")

        optiune_request = int(input("Introduceti optiune: "))

        if optiune_request == 2:
            # Request type 2 format => Vizualizare categorii de anunturi : 2
            print("Categoriile anunturilor sunt:")
            sockfd.send("2".encode())

        elif optiune_request == 3:
            # Request type 3 format => Vizualizare anunturi din categorie : 3 categorie
            print("Categoriile anunturilor sunt:")
            sockfd.send("2".encode())

            # Raspuns server
            data = sockfd.recv(1024)
            print("Raspuns server: ", data.decode("ISO-8859-1"))

            # Alegere categorie dorita pentru a vedea anunturi
            request3_categorie = input("Alegeti categoria dorita pentru a vedea anunturile din categoria aleasa: ")
            request_concatenat = "3 " + request3_categorie
            sockfd.send(request_concatenat.encode())

        elif optiune_request == 4:
            # Request type 4 format => Vizualizare anunt specific : 4 categorie nume_anunt
            print("Categoriile anunturilor sunt:")
            sockfd.send("2".encode())

            # Raspuns server
            data = sockfd.recv(1024)
            print("Raspuns server: ", data.decode("ISO-8859-1"))

            # Alegere categorie dorita pentru a vedea anunturi
            request4_categorie = input("Alegeti categoria dorita pentru a vedea anunturile din categoria aleasa: ")
            request_concatenat = "3 " + request4_categorie
            sockfd.send(request_concatenat.encode())

            # Raspuns server
            data = sockfd.recv(1024)
            print("Raspuns server: ", data.decode("ISO-8859-1"))

            # alegere anunt din categorie pentru vizualizare
            request4_nume_anunt = input("Introduceti numele anuntului din categoria >{}< pe care doriti sa-l vizualizati: ".format(request4_categorie))

            # construire raspuns final
            request_concatenat = "4 " + request4_categorie + " " + request4_nume_anunt
            sockfd.send(request_concatenat.encode())        

        elif optiune_request == 5:
            # Request type 5 format => Adauga anunt : 5 categorie nume_anunt
            print("Categoriile disponibile in care puteti adauga anunt sunt:")
            sockfd.send("2".encode())

            # Raspuns server
            data = sockfd.recv(1024)
            print("Raspuns server: ", data.decode("ISO-8859-1"))

            # alegere categorie dorita pentru a publica anunt anunturile
            request5_categorie = input("Alegeti categoria in care doriti sa adaugati un anunt:")
            request5_nume_anunt = input("Alegeti titlul anuntului pe care doriti sa-l publicati in categoria >{}<:".format(request5_categorie))

            # construiree raspuns final de trimis catre server
            request_concatenat = "5 " + request5_categorie + " " +request5_nume_anunt            
            sockfd.send(request_concatenat.encode())

        elif optiune_request == 6:
            # Request type 6 format => Sterge anunt : 6 categorie nume_anunt
            # alegere categorie si nume anunt pentru stergere
            request6_categorie = input("Alegeti categoria din care doriti sa stergeti anuntul dvs.: ")
            request6_nume_anunt = input("Introduceti numele anuntului pe care doriti sa-l stergeti: ")

            # construire raspuns final
            request_concatenat = "6 " + request6_categorie + " " + request6_nume_anunt
            sockfd.send(request_concatenat.encode())

        elif optiune_request == 7:
            # Request type 7 format => Logout: 7
            print("[LOGOUT] Sesiune curenta se inchide")
            sockfd.send("7".encode())
            sockfd.close()
            signal.signal(signal.SIGINT, handler_sigint)
            signal.SIGINT
            exit(0)



try:
    opts, args = getopt.getopt(argv, "u:p:",)
except:
    print("Error")

for opt, arg in opts:
    if opt in ['-u']:
        user = arg
    elif opt in ['-p']:
        parola = arg

user_and_parola = "1" + user + " " + parola

sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sockfd.connect((HOST, PORT))
print("[SUCCESS] conectat cu succes la server..")

sockfd.send(user_and_parola.encode())
data = sockfd.recv(1024)
print('Received', data.decode("ISO-8859-1"))


if data.decode("ISO-8859-1") == "1":
    print("\t[SUCCES] Client conectat cu succes la server, incepe comunicarea.")
    comunicareSocketClientSever()
else:
    print("\t[LOGIN INCORECT] Datele de login sunt incorecte, verifica user si/sau parola si reincearca.")
    sys.exit(0)

sockfd.close()