#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Prototypes de fonctions
void displayHelp(int dS);
int endOfCommunication(char * msg);
void * receive_from_server(void * arg);
void sending(int dS, char * msg);
void *sending_th(void *dSparam);
void receiving(int dS, char * rep, ssize_t size);
void * receiving_th(void * dSparam);

int isEnd = 0; // Indicateur de fin de communication

// Affiche l'aide pour le client
void displayHelp(int dS) {
    char *msg = "help\n";
    sending(dS, msg);
}

// Vérifie si le message indique la fin de la communication
int endOfCommunication(char * msg){
    if (strcmp(msg, "fin\n")==0){
        strcpy(msg, "** a quitté la communication **\n");
        return 1;
    }
    return 0;
}

// Thread pour recevoir les messages du serveur
void * receive_from_server(void * arg) {
    int dSC = *(int *)arg;
    char message[100];
    while (1) {
        receiving(dSC, message, sizeof(message));
        printf("%s", message);
        if (strcmp(message, "** Fin de la communication. **\n") == 0) {
            printf("Le serveur a terminé la communication.\n");
            break;
        }
    }
    close(dSC); // Fermer la connexion avec le serveur
    exit(EXIT_SUCCESS); // Terminer le client
}

// Envoie un message au serveur
void sending(int dS, char * msg){
    int sendR = send(dS, msg, strlen(msg)+1, 0);
    if (sendR == -1){ /*vérification de la valeur de retour*/
        perror("erreur au send");
        exit(-1);
    }
}

// Thread pour envoyer des messages au serveur
void *sending_th(void *dSparam) {
    int dS = (long)dSparam;
    while (!isEnd) {
        char *m = (char *)malloc(sizeof(char) * 100);

        printf(">");
        fgets(m, 100, stdin);

        if (strcmp(m, "help\n") == 0) {
            displayHelp(dS);
        } else {
            isEnd = endOfCommunication(m);
            sending(dS, m);
        }

        free(m);
    }
    close(dS);
    return NULL;
}

// Réceptionne un message d'une socket et teste que tout se passe bien
void receiving(int dS, char * rep, ssize_t size){
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1){ /*vérification de la valeur de retour*/
        printf("** fin de la communication **\n");
        exit(-1);
    }
}

// Thread pour recevoir les messages du serveur
void * receiving_th(void * dSparam){
    int dS = (long)dSparam;
    while(!isEnd){

        char * r = (char *) malloc(sizeof(char)*100);
        receiving(dS, r, sizeof(char)*100);
        printf(">%s",r);

        if (endOfCommunication(r) || strcmp(r, "** Fin de la communication. **\n") == 0) {
            isEnd = 1;
        }

        free(r);
    }
    close(dS);
    return NULL;
}

// Fonction principale
int main(int argc, char *argv[]) {

    // Vérification des paramètres
    if(argc<3){
        printf("Erreur : Lancez avec ./client <votre_ip> <votre_port> ");
    }

    // Création de la socket
    long dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(aS.sin_addr));
    aS.sin_port = htons(atoi(argv[2]));

    // Demander une connexion
    socklen_t lgA = sizeof(struct sockaddr_in);
    int connectR = connect(dS, (struct sockaddr *) &aS, lgA);
    if (connectR == -1){
        perror("erreur au connect");
        exit(-1);
    }

    // Réception du nombre de client
    int nbClient;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1){ /*vérification de la valeur de retour*/
        perror("erreur au recv du numClient");
        exit(-1);
    }

    // Saisie du pseudo du client au clavier
    int availablePseudo;
    char * myPseudo = (char *) malloc(sizeof(char)*12);
    recv(dS,&availablePseudo,sizeof(int),0);

    printf("Votre pseudo (maximum 12 caractères): ");
    fgets(myPseudo, 12, stdin);
    sending(dS,myPseudo);
    recv(dS,&availablePseudo,sizeof(int),0);

    // Vérification de la disponibilité du pseudo
    while(!availablePseudo){
        printf("Pseudo déjà utilisé!\nVotre pseudo (maximum 12 caractères): ");
        fgets(myPseudo, 12, stdin);
        sending(dS,myPseudo);
        recv(dS,&availablePseudo,sizeof(int),0);
    }

    // En attente d'un autre client
    if(nbClient==1){
        printf("En attente d'un autre client\n");

        // Réception du premier message informant de l'arrivée d'un autre client
        char * msg = (char *) malloc(sizeof(char)*100);
        receiving(dS, msg, sizeof(char)*100);
        printf("%s",msg);

        free(msg);
    }

    // Création des threads pour envoyer et recevoir des messages
    pthread_t thread_sendind;
    pthread_t thread_receiving;

    int thread1 = pthread_create(&thread_sendind, NULL, sending_th, (void *)dS);
    if(thread1==-1){
        perror("error thread 1");
    }

    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)dS);
    if(thread2==-1){
        perror("error thread 2");
    }

    // Attente de la fin des threads
    pthread_join(thread_sendind, NULL);
    pthread_join(thread_receiving, NULL);

    // Fin de la communication
    return 0;
}